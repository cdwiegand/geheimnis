
#include "be_pgp2.h"
#include "misc.h"
#include "myKeyring.h"
#include "bdebug.h"
#include "defines.h"

#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>
#if QT_VERSION < 300
#include <kapp.h>
#else
#include <kapplication.h>
#endif

#include <qprogressdialog.h>

be_pgp2::be_pgp2(const QString & newProfileName)
  : be_base()
{
	KConfig *cfg = NULL;

	cfg = kapp->config();
	if (cfg == NULL) return; // failed...

	cfg->setGroup(newProfileName);

	if (cfg->readEntry("binPGP2","").length() < 2) cfg->writeEntry("binPGP2","pgp");
	execPGP = cfg->readEntry("binPGP2","pgp");
	recipFormat = " %s ";

	cfg->sync(); // we MIGHT have changed profile-specific configs
	return;
}

QString be_pgp2::keyringsToUse(bool, bool)
{
	// PGP 2 doesn't support keyring specified in command line.
	return QString::null;
}

// -----------------------------------------------------------------

void be_pgp2::loadKeyring(pkiKeyring *keyring, QProgressDialog *winProgress, bool justRefreshTrust, const QString & keyID)
{
	/* DO NOT DELETE newKey, tmpKey, newName, or newSig under punishment of death!
	 * That would corrupt the keyring, which is what we're returning!
	 *
	 * This function is VERY GENERIC and just calls sub-functions to do all
	 * of the actual work...
	 *
	 * justRefreshTrust just refreshes the validity/trust vales, does not load new keys fully...
	 * if you specify a keyID, justRefreshTrust is ignored...
	 * keyID , if not null, will only load that key, but fully, no matter what justRefreshTrust says
	 * if it's null, it loads the whole keyring...
	 */

	// Programmer's note: When scanning, **ALWAYS** check before adding a key, even if you only load the ring once.
	// This way, I will always be sure duplicates don't exist and everything links correctly.
	// Programmer's note 2: ONLY LOAD NAMES ONCE AND ONLY ONCE!!!!! -- chris and stefan

	// Modes are not globally defined - you can have as many modes as you want, please define at top of function

	QString appString;

	qApp->processEvents();
	if (!keyID.isEmpty()) {
		appString = QString("%1 +LANGUAGE=en -kvv %2").arg(execPGP).arg(keyID);
		loadPGP2Keyring(keyring,appString,2, winProgress);
		appString = QString("%1 +LANGUAGE=en -kc %2").arg(execPGP).arg(keyID);
		loadPGP2Validity(keyring,appString, winProgress);			
	} else {
		if (justRefreshTrust) {
			appString = QString("%1 +LANGUAGE=en -kc").arg(execPGP);
			loadPGP2Validity(keyring,appString, winProgress);			
		} else {
			// PGP 2, requires four passes, once for secret keys, then for public keys, then for fingerprints...
			// and finally for the validity/trust.
			// first thing, pipe the secret, then the public
			appString = QString("%1 +LANGUAGE=en -kv ~/.pgp/secring.pgp").arg(execPGP);
			loadPGP2Keyring(keyring,appString,1, winProgress);
			appString = QString("%1 +LANGUAGE=en -kvv").arg(execPGP);
			loadPGP2Keyring(keyring,appString,2, winProgress);
			appString = QString("%1 +LANGUAGE=en -kvc").arg(execPGP);
			loadPGP2Keyring(keyring,appString,3, winProgress);
			appString = QString("%1 +LANGUAGE=en +batchmode -kc").arg(execPGP);
			loadPGP2Validity(keyring,appString, winProgress);			
		}
	}

	return;
}

void be_pgp2::loadPGP2Keyring(pkiKeyring *keyring, const QString & appString, int mode, QProgressDialog *winProgress)
{
	pkiKey *newKey = NULL;
	pkiName *newName = NULL;
	pkiSig *newSig = NULL;
	FILE *inFile, *outFile; // DO NOT FREE ME (I don't think so, that is. )
	char buf[65530], keyType[KEY_TYPE_LENGTH]; // FIXME: Un-hard code these if possible.
	QString tmpString, tmpString2;
	int lineType=0;

	pipe_pgp(appString.latin1(),&inFile,&outFile);
	qApp->processEvents();

	if (outFile == NULL) return;
	while (!feof(outFile)) {
		incrQPD(winProgress);
		qApp->processEvents();
		fgets(buf,65530,outFile);

		bDebug() << "PGP2 In: " << buf << endl;

		strncpy(keyType,buf,3);
		qApp->processEvents();
		lineType = 0; // unknown yet...
		if (strncmp(keyType,"sec",3) == 0) lineType = 1; // secret key
		if (strncmp(keyType,"pub",3) == 0) lineType = 2; // public key
		if (strncmp(keyType,"sig",3) == 0) lineType = 5; // signature
		if (strncmp(keyType,"   ",3) == 0) lineType = 4; // name OR fingerprint...

                // from this point on, only check against lineType, not keyType -- Stefan
		
		switch (lineType) {
			case 1:
			case 2:
				bDebug() << "\tNew Key!" << endl;

				// eat leading "sec" or "pub"
				strtok(buf, " ");
					
				// next, key size...
				tmpString2 = strtok('\0',"/");
	
				// next, keyID
				tmpString = QString("0x%1").arg(strtok('\0'," "));

				// Here, we search for this key. We may already have it, if so, use existing record...
				newKey = keyring->getKey(tmpString);
				if (newKey == NULL) {
					newKey = new pkiKey(keyring,tmpString);
					tmpString2 = tmpString2.stripWhiteSpace();
					newKey->setKeySize(tmpString2);
					newKey->setKeyType("RSA");
				}

				// creation date...
				tmpString = strtok('\0'," ");
				newKey->setCreateDate(tmpString);
				// expire date (PGP-2.6.3i doesnt support expiring of keys, so...   -- stefan)
					
				if (lineType == 1) newKey->haveSecret = TRUE;

				// finally, user name. Only load the names in the PUBLIC keyring.
				// We only load the secret one to pick up if we have the secret as well as public keys...
				// only load names ONE TIME -- stefan
				if (mode != 2) break;

				bDebug() << "\tNew Name!" << endl;

				tmpString = strtok('\0',"\n");
				tmpString2 = tmpString.stripWhiteSpace();
  				if (tmpString2.find("*** KEY REVOKED ***") >= 0) newKey->isRevoked = TRUE;
				// We've got a name!!!
				newName = new pkiName(newKey,extractDisplayName(tmpString2),extractEmailAddress(tmpString2));

				// We have to use sscanf with a space in the first position
				// of the format string to ignore the leading spaces before the name
				// sscanf(buf," %[^\n]",tmpString); // Old version I want to keep until I trust stripWhiteSpace()
				winProgress->setLabelText(newName->getDisplayName()); // I want to show the name...
				break;
			case 4:
				// Only load the names in the PUBLIC keyring to avoid duplicates.
				// Only load fingerprints in the PUBLIC keyring.
				if (mode == 2) {
					bDebug() << "\tNew Name!" << endl;

					// We've got a name!!!
					tmpString = buf;
					tmpString2 = tmpString.stripWhiteSpace();
					newName = new pkiName(newKey,extractDisplayName(tmpString2),extractEmailAddress(tmpString2));

					// We have to use sscanf with a space in the first position
					// of the format string to ignore the leading spaces before the name
					// sscanf(buf," %[^\n]",tmpString); // Old version I want to keep until I trust stripWhiteSpace()
					winProgress->setLabelText(newName->getDisplayName()); // I want to show the name...
				} else if (mode == 3) {
					bDebug() << "\tNew Fingerprint!" << endl;

					// We've got a fingerprint!!!
					tmpString = buf;
					if (tmpString.find("Key fingerprint =")<0) break;
					// This is no fingerprint, but a name
					tmpString2 = tmpString.mid(tmpString.find('=') + 1,tmpString.length() - tmpString.find('=') - 1);
					tmpString = tmpString2.stripWhiteSpace();
					newKey->setFingerprint(tmpString);
				}
				break;
			case 5:
				bDebug() << "\tNew Signature!" << endl;

				// eat leading "sig"
				strtok(buf, " ");
					
				tmpString = QString("0x%1").arg(strtok('\0'," "));
				newSig = new pkiSig(newName,tmpString);
				break;
		}
	}
	fclose(outFile);
}

void be_pgp2::loadPGP2Validity(pkiKeyring *keyring, const QString & appString, QProgressDialog *winProgress)
{
	pkiKey *newKey = NULL;
	FILE *inFile, *outFile; // DO NOT FREE ME (I don't think so, that is. )
	char buf[65530];
	QString tmpString, KeyId, Validity, Trust;

	pipe_pgp(appString.latin1(),&inFile,&outFile);
	qApp->processEvents();

	if (outFile == NULL) return;

	winProgress->setLabelText(i18n("Loading Validity..."));
		
	while (!feof(outFile)) {
		incrQPD(winProgress);
		qApp->processEvents();
		fgets(buf,65530,outFile);
		tmpString = buf;
		if (tmpString.find("KeyID    Trust     Validity  User ID") > -1)	break;
	}
	
	while (!feof(outFile)) {
		incrQPD(winProgress);
		qApp->processEvents();
		fgets(buf,65530,outFile);
  		tmpString = buf;
  		if (tmpString.mid(2,1) != " ") {       // Startpoint of Key-ID, empty when name or signature
  			KeyId = tmpString.mid(2,8);
  			KeyId = "0x" + KeyId;
  			Validity = tmpString.mid(21,9);
  			Trust = tmpString.mid(11,9);

			bDebug() << "KeyID " << KeyId << endl
				 << "Validity " << Validity << endl;

			newKey = keyring->getKey(KeyId);
			if (newKey == NULL) continue;              
			// Houston, we have a problem ....
				
			switch (Validity[0].latin1()) {
				case 'u':
					newKey->setValidity(0);
					break;
				case 'n':
					newKey->setValidity(1);
					break;
				case 'm':
					newKey->setValidity(2);
					break;
				case 'c':
					newKey->setValidity(3);
					break;
			}
			
			if (Trust.contains("untrusted"))
				newKey->setTrust(1);
			if (Trust.contains("undefined"))
				newKey->setTrust(0);
			if (Trust.contains("marginal"))
				newKey->setTrust(2);
			if (Trust.contains("complete"))
				newKey->setTrust(3);
			
			if (newKey->haveSecret) {
				newKey->setValidity(4);
				newKey->setTrust(4);
			}
  		}

	}
	fclose(outFile);
}

int be_pgp2::importKey_File(const QString & fileName)
{
	int retVal = -1;
	QString appString;

	if (fileName.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a file!"));
		return -1;
	}

	appString = QString("%1 -ka \\\"%2\\\"").arg(execPGP).arg(fileName);
	retVal = runProcessFG(appString);

	return retVal;
}

int be_pgp2::importKey_Server(const QString & keyID, const QString &)
{
	int retVal = -1;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	showUnsupportedDialog();
	// KMessageBox::sorry(NULL,i18n("Sorry, but your encryption program does not support that feature."));

	return retVal;
}

int be_pgp2::editKey(const QString & keyID)
{
	int retVal = -1;
	QString appString;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	appString = QString("%1 -ke %2").arg(execPGP).arg(keyID);
	retVal = runProcessFG(appString);

	return retVal;
}

int be_pgp2::disEnableKey(const QString & keyID)
{
	int retVal = -1;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	showUnsupportedDialog();

	return retVal;
}

int be_pgp2::removeKey(const QString & keyID)
{
	QString appString;
	int retVal = -1;

	if (keyID.isEmpty()) {
		showBadKeyIDDialog();
		return -1;
	}

	appString = QString("%1 -kr %2").arg(execPGP).arg(keyID);
	retVal = runProcessFG(appString);

	return retVal;
}

int be_pgp2::revokeKey(const QString & keyID)
{
	int retVal = -1;
	QString appString;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	appString = QString("%1 -kd %2").arg(execPGP).arg(keyID);
	retVal = runProcessFG(appString);

	return retVal;
}

int be_pgp2::revokeSignature(const QString & keyID)
{
	QString appString;
	int retVal = -1;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	appString = QString("%1 -krs %2").arg(execPGP).arg(keyID);
	retVal = runProcessFG(appString);

	return retVal;
}

int be_pgp2::addName(const QString &keyID)
{
	QString appString;
	int retVal = -1;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	appString = QString("%1 -ke %2").arg(execPGP).arg(keyID);
	retVal = runProcessFG(appString);

	return retVal;
}

int be_pgp2::genKeyPair()
{
	QString appString;
	int retVal = -1;

	appString = QString("%1 -kg").arg(execPGP);
	retVal = runProcessFG(appString);

	return retVal;
}

int be_pgp2::exportKey_Server(const QString &keyID, const QString &)
{
	int retVal = -1;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	showUnsupportedDialog();

	return retVal;
}

int be_pgp2::exportKey_File(const QString &keyID, const QString &fileName)
{
	int retVal=-1;
	QString appString;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	// Is there *any* way short of piping that we can work around this? -- chris
	KMessageBox::sorry(NULL,i18n("PGP 2 does not allow you to specify an outgoing file to save the exported key to.\nPGP 2 will save the file to the filename you give it,\nwith an \".asc\" appended if the name you give it does not end in .asc already."));
	appString = QString("%1 -kxa %2 \\\"%3\\\"").arg(execPGP).arg(keyID).arg(fileName);
	retVal = runProcessFG(appString);

	return retVal;
}

int be_pgp2::signKey(const QString &keyIDToSign, const QString &signingKeyID)
{
	int retVal = -1;
	QString appString;

	if (keyIDToSign.isEmpty() || signingKeyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	appString = QString("%1 -ks %2 -u %3").arg(execPGP).arg(keyIDToSign).arg(signingKeyID);
	retVal = runProcessFG(appString);
	
	return retVal;
}

int be_pgp2::decryptFile(const QString &file1, const QString &file2, bool detachedSig, bool justVerify)
{
	// Private function
	QString pgpAppString;
	int retVal = -1;

	if (!justVerify) {
		// we're decrypting...
		pgpAppString = QString("%1 \\\"%2\\\" -o \\\"%3\\\"").arg(execPGP).arg(file1).arg(file2);
	} else {
		// just verifying,
		if (detachedSig) {
			pgpAppString = QString("%1 \\\"%2\\\" \\\"%3\\\" -o /dev/null").arg(execPGP).arg(file2).arg(file1);
		} else {
			pgpAppString = QString("%1 \\\"%2\\\" -o /dev/null").arg(execPGP).arg(file1);
		}
	}

	retVal = runProcessFG(pgpAppString,1);  // we want the shell to stay open
	return retVal;
}

int be_pgp2::encryptFile(const QString & inFile, const QString & outFile, const QString & signingID, const QStringList & recipList,
	bool detachSig, bool signFile, bool encryptFile, bool asciiArmor, bool uniText, bool clearSign)
{
	QString pgpAppString, comment, tmpString;
	int retVal = -1;
	KConfig *cfg = NULL;

	cfg = kapp->config();
	cfg->setGroup("__Global");
	comment = cfg->readEntry("Comment", i18n("Made with Geheimnis"));

	pgpAppString = QString("%1 +COMMENT=\\\"%2\\\" ").arg(execPGP).arg(comment);
	pgpAppString.append(" -"); // we either have to add an "e" and/or "s"...
	if (encryptFile) pgpAppString.append("e");
	if (signFile) {
		pgpAppString.append("s");
		if (detachSig) pgpAppString.append("b");
	}
	if (asciiArmor) pgpAppString.append("a");
	if (uniText) pgpAppString.append("t");
	if (clearSign) pgpAppString.append(" +CLEARSIG=ON ");
	if (signFile) {
		tmpString = QString(" -u %1 ").arg(signingID);
		pgpAppString.append(tmpString);
	}
	// WARNING: possible sec problem if filoenames contain special chars!
	tmpString = QString(" -o \\\"%1\\\" \\\"%2\\\" %3").arg(outFile).arg(inFile).arg(recipList.join(QString(" ")));
	pgpAppString.append(tmpString);
	retVal = runProcessFG(pgpAppString);

	return retVal;
}








