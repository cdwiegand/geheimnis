
#include "be_pgp5.h"
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
#include <qregexp.h>

#include <stdlib.h>
#include <time.h>

be_pgp5::be_pgp5(const QString & newProfileName)
  : be_base()
{
	KConfig *cfg = NULL;
	QString sTmp;
	QString homeEnv;

	homeEnv = getenv("HOME");

	cfg = kapp->config();
	if (cfg == NULL) return; // failed...

	cfg->setGroup(newProfileName);

	if (cfg->readEntry("binPGP5K","").length() < 2) cfg->writeEntry("binPGP5K","pgpk");
	execPGPk = cfg->readEntry("binPGP5K","pgpk");
	if (cfg->readEntry("binPGP5E","").length() < 2) cfg->writeEntry("binPGP5E","pgpe");
	execPGPe = cfg->readEntry("binPGP5E","pgpe");
	if (cfg->readEntry("binPGP5S","").length() < 2) cfg->writeEntry("binPGP5S","pgps");
	execPGPs = cfg->readEntry("binPGP5S","pgps");
	if (cfg->readEntry("binPGP5V","").length() < 2) cfg->writeEntry("binPGP5V","pgpv");
	execPGPv = cfg->readEntry("binPGP5V","pgpv");

	if (cfg->readEntry("SecretKeyring","").length() < 2) cfg->writeEntry("SecretKeyring","~/.pgp/secring.skr");
	if (cfg->readEntry("PublicKeyring","").length() < 2) cfg->writeEntry("PublicKeyring","~/.pgp/pubring.pkr");
	recipFormat = " -r %s ";

        secretKeyringFiles = cfg->readListEntry("SecretKeyring",';');
        publicKeyringFiles = cfg->readListEntry("PublicKeyring",';');

        // Now, scan lists for ~, replace with getenv("HOME");
	for ( QStringList::Iterator it = secretKeyringFiles.begin() ;
	      it != secretKeyringFiles.end() ; ++it )
	  (*it).replace(QRegExp("^~"), homeEnv);
	for ( QStringList::Iterator it = publicKeyringFiles.begin() ;
	      it != publicKeyringFiles.end() ; ++it )
	  (*it).replace(QRegExp("^~"), homeEnv);

	cfg->sync(); // we MIGHT have changed profile-specific configs
	return;
}

QString be_pgp5::keyringsToUse(bool bIncludeSecret, bool bIncludePublic)
{
	QString pgpAppString("");

	if (bIncludeSecret) {
		if (bIncludePublic) {
			pgpAppString = QString(" --secring=%1 --pubring=%2").arg(*secretKeyringFiles.at(0)).arg(*publicKeyringFiles.at(0));
		} else {
			pgpAppString = QString(" --secring=%1").arg(*secretKeyringFiles.at(0));
		}
	} else {
		if (bIncludePublic) {
			pgpAppString = QString(" --pubring=%1").arg(*publicKeyringFiles.at(0));
		}
	}

	return pgpAppString;
}

// -----------------------------------------------------------------

void be_pgp5::loadKeyring(pkiKeyring *keyring, QProgressDialog *winProgress, bool justRefreshTrust, const QString & keyID)
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

	//qApp->processEvents();
	if (!keyID.isEmpty()) {
		appString = QString("%1 +language=us -ll %2 %3").arg(execPGPk).arg(keyID).arg(keyringsToUse());
		loadPGP5Keyring(keyring, appString,winProgress);
		appString = QString("%1 +language=us -c %2 %3").arg(execPGPk).arg(keyID).arg(keyringsToUse());
		loadPGP65Validity(keyring,appString, winProgress);
	} else {
		if (justRefreshTrust) {
			appString = QString("%1 +language=us -c %2").arg(execPGPk).arg(keyringsToUse());
			loadPGP65Validity(keyring,appString, winProgress);
		} else {
			appString = QString("%1 +language=us -ll %2").arg(execPGPk).arg(keyringsToUse());
			loadPGP5Keyring(keyring,appString,winProgress);
			// sorry, need a second pass to read validity and trust -- stefan
			appString = QString("%1 +language=us -c %2").arg(execPGPk).arg(keyringsToUse());
			loadPGP65Validity(keyring,appString, winProgress);
		}
	}

	return;
}

void be_pgp5::loadPGP5Keyring(pkiKeyring *keyring, const QString & appString, QProgressDialog *winProgress)
{
	pkiKey *newKey = NULL;
	pkiName *newName = NULL;
	pkiSig *newSig = NULL;
	FILE *inFile, *outFile; // DO NOT FREE ME (I don't think so, that is. )
	char buf[65530], keyType[KEY_TYPE_LENGTH]; // FIXME: Un hard-code these if possible.
	QString tmpString, tmpString2;
	int lineType=0;
	time_t localdate;
	tm *gmtdate;
	char date[12];

	winProgress->setLabelText(i18n("Loading keyring..."));
	pipe_pgp(appString.latin1(),&inFile,&outFile);
	if (outFile == NULL) return;
	while (!feof(outFile)) {
		incrQPD(winProgress);
		qApp->processEvents();
		fgets(buf,65530,outFile);

		bDebug() << "PGP5 In: " << buf << endl;

		if (strlen(buf) >= 5) {
			strncpy(keyType,strtok(buf," "),KEY_TYPE_LENGTH);
			lineType=0; // unknown default...
			if (strcmp(keyType,"sec") == 0)  lineType = 1; // secret line...
			if (strcmp(keyType,"sec+") == 0) lineType = 1; // secret line...
			if (strcmp(keyType,"pub") == 0)  lineType = 2; // public key
			if (strcmp(keyType,"pub@") == 0) lineType = 2; // disabled public key
			if (strcmp(keyType,"sub") == 0)  lineType = 3; // subkey...
			if (strcmp(keyType,"uid") == 0)  lineType = 4; // userid..
			if (strcmp(keyType,"f16") == 0)  lineType = 7; // fingerprint...
			if (strcmp(keyType,"f20") == 0)  lineType = 7; // fingerprint...
			if (strcmp(keyType,"sig") == 0)  lineType = 5; // signature...
			if (strcmp(keyType,"SIG") == 0)  lineType = 5; // signature...
			if (strcmp(keyType,"sig?") == 0) lineType = 5; // signature...
	
			qApp->processEvents();
			switch (lineType) {
				case 1:
				case 2:
					bDebug() << "\tNew Key!" << endl;

					// next, key size...
					tmpString = strtok('\0'," ");
					tmpString2 = strtok('\0'," ");
					newKey = new pkiKey(keyring,tmpString2);

					newKey->setKeySize(tmpString);
					newKey->haveSecret = (lineType == 1);
					if (strcmp(keyType,"pub@") == 0) newKey->isDisabled = TRUE;
					
					// next, 2 dates
					tmpString = strtok('\0'," ");
					newKey->setCreateDate(tmpString);
					tmpString = strtok('\0'," ");
					newKey->setExpireDate(tmpString);
					
					if ("*revoked*" == newKey->getExpireDate().lower()) {
						newKey->isRevoked = TRUE;
						// I have to insert a dummy name to get same layout as in the other PGP programs  -- stefan
						newName = new pkiName(newKey,"*REVOKED*","*REVOKED*");
					}
					
					time (&localdate);
					gmtdate = gmtime (&localdate);
					strftime (date,sizeof date,"%Y-%m-%d",gmtdate);
					if ("----------" != newKey->getExpireDate() && "*REVOKED*" != newKey->getExpireDate() && strncmp(date,newKey->getExpireDate().latin1(),12) > 0)
						newKey->isExpired = TRUE;
					
					// next, key type...
					newKey->setKeyType(strtok('\0'," "));
					if ( newKey->getKeyType().lower() == "dss" ) newKey->setKeyType("DH/DSS");
					break;
				case 4:
					bDebug() << "\tNew Name!" << endl;

					tmpString = strtok('\0',"\n");
					tmpString2 = tmpString.stripWhiteSpace();
					newName = new pkiName(newKey,extractDisplayName(tmpString2),extractEmailAddress(tmpString2));

					winProgress->setLabelText(newName->getDisplayName()); // I want to show the name...
					break;
				case 5:
					bDebug() << "\tNew Signature!" << endl;

					// Signature for PGP 5
					// sig	0x???????? date name ... name <email address>
					newSig = new pkiSig(newName,strtok('\0'," "));
					break;
			case 7:
					bDebug() << "\tNew Fingerprint!" << endl;

					// Fingerprint
					tmpString = strtok('\0',"\n");
					tmpString2 = tmpString.mid(tmpString.find('=') + 1,tmpString.length() - tmpString.find('=') - 1);
					tmpString = tmpString2.stripWhiteSpace();
					newKey->setFingerprint(tmpString);
					break;
			}
		}
	}
	fclose(outFile);
	return;
}

void be_pgp5::loadPGP65Validity(pkiKeyring *keyring, const QString & appString, QProgressDialog *winProgress)
{
 	pkiKey *newKey = NULL;
 	FILE *inFile, *outFile; // DO NOT FREE ME (I don't think so, that is. )
 	char buf[65530];
 	QString tmpString, KeyId, Validity, Trust;

 	pipe_pgp(appString.latin1(),&inFile,&outFile);
 	qApp->processEvents();

 	if (outFile == NULL) return;

 	winProgress->setLabelText(i18n("Loading Validity..."));
	winProgress->show();
	qApp->processEvents();
 		
 	while (!feof(outFile)) {
		incrQPD(winProgress);
	 	winProgress->setLabelText(i18n("Loading Validity..."));
		winProgress->show();
 		qApp->processEvents();
 		fgets(buf,65530,outFile);
 		tmpString = buf;
 		if (tmpString.find("KeyID      Trust     Validity  User ID") > -1)	break;
 	}
 	
 	while (!feof(outFile)) {
		incrQPD(winProgress);
		qApp->processEvents();
 		fgets(buf,65530,outFile);
   		tmpString = buf;
   		if (tmpString.mid(2,1) == "0") {       // Startpoint of Key-ID, empty when name or signature
   			KeyId = tmpString.mid(2,10);
   			Validity = tmpString.mid(23,9);
   			Trust = tmpString.mid(13,9);
			winProgress->setLabelText(KeyId);
   		
 			newKey = keyring->getKey(KeyId);
 			if (newKey == NULL) continue;
 			// Houston, we have a problem ....

			bDebug() << "KeyID " << KeyId << endl
				 << "Validity " << Validity << endl;
 				
 			switch (Validity[0].latin1()) {
 				case 'u':
 					newKey->setValidity(0);
 					break;
 				case 'i':
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
 			
 			if (newKey->haveSecret == TRUE && newKey->getValidity() > 1) {
 				newKey->setValidity(4);
 				newKey->setTrust(4);
 			}
   		}

 	}
 	fclose(outFile);
}

int be_pgp5::importKey_File(const QString & fileName)
{
	int retVal = -1;
	QString appString;

	if (fileName.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a file!"));
		return -1;
	}

	appString = QString("%1 -a \\\"%2\\\" %3").arg(execPGPk).arg(fileName).arg(keyringsToUse());
	retVal = runProcessFG(appString);

	return retVal;
}

int be_pgp5::importKey_Server(const QString & keyID, const QString &)
{
	int retVal = -1;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	QString appString = QString("%1 -a %2 %3").arg(execPGPk).arg(keyID).arg(keyringsToUse());
	retVal = runProcessFG(appString);

	return retVal;
}

int be_pgp5::editKey(const QString & keyID)
{
	int retVal = -1;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	QString appString = QString("%1 -e %2 %3").arg(execPGPk).arg(keyID).arg(keyringsToUse());
	retVal = runProcessFG(appString);

	return retVal;
}

int be_pgp5::disEnableKey(const QString & keyID)
{
	int retVal = -1;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	QString appString = QString("%1 -d %2 %3").arg(execPGPk).arg(keyID).arg(keyringsToUse());
	retVal = runProcessFG(appString);

	return retVal;
}

int be_pgp5::removeKey(const QString & keyID)
{
	int retVal = -1;

	if (keyID.isEmpty()) {
		showBadKeyIDDialog();
		return -1;
	}

	QString appString = QString("%1 -r %2 %3").arg(execPGPk).arg(keyID).arg(keyringsToUse());
	retVal = runProcessFG(appString);

	return retVal;
}

int be_pgp5::revokeKey(const QString & keyID)
{
	QString appString, txtRevokCert;
	int retVal = -1;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	appString = QString("%1 --revoke %2 %3").arg(execPGPk).arg(keyID).arg(keyringsToUse());
	retVal = runProcessFG(appString);

	return retVal;
}

int be_pgp5::revokeSignature(const QString & keyID)
{
	QString appString;
	int retVal = -1;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	appString = QString("%1 --revokes %2 %3").arg(execPGPk).arg(keyID).arg(keyringsToUse());
	retVal = runProcessFG(appString);

	return retVal;
}

int be_pgp5::addName(const QString & keyID)
{
	QString appString;
	int retVal = -1;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	appString = QString("%1 -e %2 %3").arg(execPGPk).arg(keyID).arg(keyringsToUse());
	retVal = runProcessFG(appString);

	return retVal;
}

int be_pgp5::genKeyPair()
{
	QString appString;
	int retVal = -1;

	appString = QString("%1 -g %2").arg(execPGPk).arg(keyringsToUse());
	retVal = runProcessFG(appString);

	return retVal;
}

int be_pgp5::exportKey_Server(const QString & keyID, const QString &)
{
	QString appString;
	int retVal = -1;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	showUnsupportedDialog();

	return retVal;
}

int be_pgp5::exportKey_File(const QString & keyID,const QString & fileName)
{
	int retVal=-1;
	QString appString;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	appString = QString("%1 -xa %2 -o \\\"%3\\\" %4").arg(execPGPk).arg(keyID).arg(fileName).arg(keyringsToUse());
	retVal = runProcessFG(appString);

	return retVal;
}

int be_pgp5::signKey(const QString & keyIDToSign, const QString & signingKeyID)
{
	int retVal = -1;
	QString appString;

	if (keyIDToSign.isEmpty() || signingKeyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	appString = QString("%1 -s %2 -u %3 %4").arg(execPGPk).arg(keyIDToSign).arg(signingKeyID).arg(keyringsToUse());
	retVal = runProcessFG(appString);
	
	return retVal;
}

int be_pgp5::decryptFile(const QString & file1, const QString & file2, bool detachedSig, bool justVerify)
{
	// Private function
	QString pgpAppString;
	int retVal = -1;

	if (!justVerify) {
		// we're decrypting...
		pgpAppString = QString("%1 -o \\\"%2\\\" %3 \\\"%4\\\"").arg(execPGPv).arg(file2).arg(keyringsToUse()).arg(file1);
	} else {
		// just verifying,
		if (detachedSig) {
			// Sadly, PGP 5 is stupid and can't handle both the sig and real file at once... :(
			pgpAppString = QString("%1 \\\"%2\\\" %3").arg(execPGPv).arg(file2).arg(keyringsToUse());
		} else {
			pgpAppString = QString("%1 \\\"%2\\\" %3 -o /dev/null").arg(execPGPv).arg(file1).arg(keyringsToUse());
		}
	}

	retVal = runProcessFG(pgpAppString,1);  // we want the shell to stay open
	return retVal;
}

int be_pgp5::encryptFile(const QString & inFile, const QString & outFile, const QString & signingID, const QStringList & recipList,
	bool detachSig, bool signFile, bool encryptFile, bool asciiArmor, bool uniText, bool clearSign)
{
	// PGP-5 doesn't have a command for clearSign, it's turned on by default when using asciiArmor
	if (clearSign); // to ignore warning...
	QString pgpAppString, comment, tmpString;
	int retVal = -1;
	KConfig *cfg = NULL;

	cfg = kapp->config();
	cfg->setGroup("__Global");
	comment = cfg->readEntry("Comment", i18n("Made with Geheimnis"));

	if (encryptFile)
 		pgpAppString = QString("%1 --comment=\\\"%2\\\"").arg(execPGPe).arg(comment);
	else
		pgpAppString = QString("%1 --comment=\\\"%2\\\"").arg(execPGPs).arg(comment);

	if (detachSig) pgpAppString.append(" -b ");
	if (encryptFile && signFile) pgpAppString.append(" -s ");
	if (signFile) {
		tmpString = QString(" -u %1 ").arg(signingID);
		pgpAppString.append(tmpString);
	}
	if (asciiArmor) pgpAppString.append(" -a ");
	if (uniText) pgpAppString.append(" -t ");
	tmpString = QString(" %1 %2 -o \\\"%3\\\" \\\"%4\\\"").arg(keyringsToUse()).arg(recipList.join(QString(" "))).arg(outFile).arg(inFile);
	pgpAppString.append(tmpString);
	retVal = runProcessFG(pgpAppString);

	return retVal;
}





