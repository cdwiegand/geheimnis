
#include "be_gpg.h"
#include "misc.h"
#include "myKeyring.h"
#include "bdebug.h"
#include "defines.h"

#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#if QT_VERSION < 300
#include <kapp.h>
#else
#include <kapplication.h>
#endif

#include <qregexp.h>
#include <qprogressdialog.h>

#include <stdlib.h>
#include <assert.h>

be_gpg::be_gpg(const QString & newProfileName)
  : be_base()
{
	KConfig *cfg = NULL;
	QString sTmp;
	QString homeEnv;

	bDebug() << "be_gpg1" << endl;
	homeEnv = getenv("HOME");

	cfg = kapp->config();
	if (cfg == NULL) return; // failed...

	cfg->setGroup(newProfileName);

	bDebug() << "be_gpg2" << endl;
	if (cfg->readEntry("binGPG","").length() < 2) cfg->writeEntry("binGPG","gpg");
	execGPG = cfg->readEntry("binGPG","gpg");
	if (cfg->readEntry("SecretKeyring","").length() < 2) cfg->writeEntry("SecretKeyring","~/.gnupg/secring.gpg");
	if (cfg->readEntry("PublicKeyring","").length() < 2) cfg->writeEntry("PublicKeyring","~/.gnupg/pubring.gpg");
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

QString be_gpg::keyringsToUse(bool bIncludeSecret, bool bIncludePublic)
{
	QString pgpAppString("");
	unsigned int i; // general counter...

	pgpAppString = " --no-default-keyring ";
	if (bIncludeSecret) {
		for (i=0;i<secretKeyringFiles.count();i++) {
			pgpAppString.append(" --secret-keyring ");
			pgpAppString.append(*secretKeyringFiles.at(i));
		}
	}
	if (bIncludePublic) {
		for (i=0;i<publicKeyringFiles.count();i++) {
			pgpAppString.append(" --keyring ");
			pgpAppString.append(*publicKeyringFiles.at(i));
		}
	}

	return pgpAppString;
}

// -----------------------------------------------------------------

void be_gpg::loadKeyring(pkiKeyring *keyring, QProgressDialog *winProgress, bool justRefreshTrust, const QString & keyID)
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
	QString appString;

	// Load all keyrings - gpg supports multiples :)
	if (!keyID.isEmpty()) {
		appString = QString("%1 --list-sigs %2 --with-colons --no-greeting %3").arg(execGPG).arg(keyID).arg(keyringsToUse(TRUE,TRUE));
		loadGPGKeyring(keyring,appString,1, winProgress);
	} else {
		if (justRefreshTrust) {
			appString = QString("%1 --list-keys --with-colons --no-greeting %2").arg(execGPG).arg(keyringsToUse(TRUE,TRUE));
			loadGPGKeyring(keyring,appString,1, winProgress);
		} else {
			appString = QString("%1 --list-sigs --fingerprint --with-colons --no-greeting %2").arg(execGPG).arg(keyringsToUse(TRUE,TRUE));
			loadGPGKeyring(keyring,appString,1, winProgress);
			appString = QString("%1 --list-secret-keys --with-colons --no-greeting %2").arg(execGPG).arg(keyringsToUse(TRUE,FALSE));
			loadGPGKeyring(keyring,appString,2, winProgress);
		}
	}

	return;
}

QString be_gpg::unquoteGpgUid( const QCString & str ) {
  if ( str.isEmpty() ) return QString::null;
  // gnupg-1.0.6/doc/DETAILS is a bit vague on this. It says that the
  // UID field in the --with-colons listing is "encoded like a C
  // string", but doesn't explain which escape mechanisms are used.
  // We support the \x escape only and additionally assume that the
  // alphanumeric hex digits are lowercase, since it is used as an
  // example in the docs and there's no reason to assume otherwise...

  // the regexp:
  static QRegExp rx("\\\\x[0-9a-f][0-9a-f]", true /* case-sens. */ );
  // array for conversion from hexchars:
  static const QCString fromHex = "0123456789abcdef";

  int pos = 0;
  char buf[2]; buf[1] = 0;
  QCString result = str;
  while ( ( pos = result.find( rx, pos ) ) >= 0 ) {
    int msb = fromHex.find( result[pos+2] );
    int lsb = fromHex.find( result[pos+3] );

    assert( msb >= 0 ); assert( msb <= 15 ); // just in case...
    assert( lsb >= 0 ); assert( lsb <= 15 );

    buf[0] = uchar(16 * msb + lsb);
    result.replace( pos, 4, buf );
    pos++;
  }
  return QString::fromUtf8( result.data() );
}

void be_gpg::loadGPGKeyring(pkiKeyring *keyring, const QString & appString, int mode, QProgressDialog *winProgress)
{
	// This functions loads each record (line) and sends it to parseGPGKey for merging...
	pkiKey *newKey = NULL;
	pkiSubKey *newSubKey = NULL;
	pkiName *newName = NULL;
	pkiSig *newSig = NULL;
	FILE *inFile, *outFile; // DO NOT FREE ME (I don't think so, that is. )
	char buf[65530], keyType[KEY_TYPE_LENGTH]; // FIXME: Un-hard code these if possible.
	QString tmpString;
	int lineType=0, inSubKey = 0;

	bDebug() << appString << endl;
	pipe_pgp(appString.latin1(),&inFile,&outFile);
	qApp->processEvents();
	if (outFile == NULL) return;

	while (!feof(outFile)) {
		incrQPD(winProgress);
		qApp->processEvents();
		cpString(outFile,buf);

		bDebug() << "GPG In: " << buf << endl;

		// using fgets() causes problems...
		if (strchr(buf,':') != NULL) {
			strncpy(keyType,buf,3);
			lineType = 0;
			if (strncmp(keyType,"sec",3) == 0) lineType = 1; // secret line...
			if (strncmp(keyType,"pub",3) == 0) lineType = 2; // public key
			if (strncmp(keyType,"uid",3) == 0) lineType = 4; // user id
			if (strncmp(keyType,"sig",3) == 0) lineType = 5; // signature...
			if (strncmp(keyType,"fpr",3) == 0) lineType = 7; // fingerprint...
			if (strncmp(keyType,"sub",3) == 0) lineType = 6; // subkey...
			if (strncmp(keyType,"ssb",3) == 0) lineType = 6; // subkey secret...
			if (strncmp(keyType,"rev",3) == 0) lineType = 8; // revokation...

			switch (lineType) {
				case 1:
				case 2:
					// What we do here: scan for the keyID. If we find it, use that key. If not, new key...
					tmpString.sprintf("0%s",getItem(buf,':',5));
					//
					//	Any GPG keyid starting w/ a letter
				        //	must be prefixed with '0' to work.
				        //
				        //	As the additional '0' does no harm
				        //	in case of the keyid starting w/ a
				        //	digit, add it anywhere.
				        //
				        //	Keyid len is 64bits, which makes for
				        //	16 digits, size of array is big enough.
				        //
					newKey = keyring->getKey(tmpString);
					if (newKey == NULL) {
						bDebug() << "\tNew Key!" << endl;

						newKey = new pkiKey(keyring,tmpString);
					}
					if (lineType == 1) newKey->haveSecret = TRUE;


					tmpString = getItem(buf,':',2);
					switch (tmpString[0].latin1()) {
						case 'o':
						case 'q':
							newKey->setValidity(0);
							break;
						case 'e':
							newKey->setValidity(0);
							newKey->isExpired = TRUE;
							break;
						case 'd':
							newKey->setValidity(0);
							newKey->isDisabled = TRUE;
							break;
						case 'r':
							newKey->setValidity(0);
							newKey->isRevoked = TRUE;
							break;
						case 'n':
							newKey->setValidity(1);
							break;
						case 'm':
							newKey->setValidity(2);
							break;
						case 'f':
							newKey->setValidity(3);
							break;
						case 'u':
							newKey->setValidity(4);
							break;
					}

					// expiration date...
					tmpString = getItem(buf,':',6);
					newKey->setCreateDate(tmpString);
					tmpString = getItem(buf,':',7);
					if (!tmpString.isEmpty()) newKey->setExpireDate(tmpString);

					// next, key size...
					tmpString = getItem(buf,':',3);
					newKey->setKeySize(tmpString);

					// next, GPG key type...
					tmpString = getItem(buf,':',4);
					if (tmpString == "1") newKey->setKeyType("RSA");
					if (tmpString == "16") newKey->setKeyType("ElGamal");
					if (tmpString == "17") newKey->setKeyType("DSA");
					if (tmpString == "20") newKey->setKeyType("ElGamal");
			
					if (lineType == 2 && mode == 1) {
						// finally, user name
						tmpString = unquoteGpgUid(getItem(buf,':',10));
						// FIXME yeah, acutally we do, but in the constructor for pkiName make 0-len names convert to i18n("Unknown") and prepare for sorting...
						// we don´t load empty names in pub line -- stefan
						if (!tmpString.isEmpty()) newName = new pkiName(newKey,extractDisplayName(tmpString),extractEmailAddress(tmpString));
						inSubKey = 0;
					}

 					tmpString = getItem(buf,':',9);
 					switch (tmpString[0].latin1()) {
 						case '-':
 						case 'q':
 						case 'e':
 							newKey->setTrust(0);
 							break;
 						case 'n':
 							newKey->setTrust(1);
 							break;
 						case 'm':
 							newKey->setTrust(2);
 							break;
 						case 'f':
 							newKey->setTrust(3);
 							break;
 						case 'u':
 							newKey->setTrust(4);
 							break;
 					}
 					if (lineType == 1) newKey->setTrust(4);
					break;
				
				case 4:
					if (mode == 1) {
						bDebug() << "\tNew Name!" << endl;

						tmpString = unquoteGpgUid(getItem(buf,':',10));
						newName = new pkiName(newKey,extractDisplayName(tmpString),extractEmailAddress(tmpString));

						// revoked?
						tmpString = getItem(buf,':',2);
						if (tmpString[0].latin1() == 'r') newName->isRevoked = TRUE;

						inSubKey = 0;
						winProgress->setLabelText(newName->getDisplayName()); // I want to show the name...
					} // we only load names under the public keyring!
					break;
				case 5:
					if (mode == 1) {
						bDebug() << "\tNew Signature!" << endl;

						tmpString.sprintf("0%s",getItem(buf,':',5));
						if (tmpString.length() > 1) {
							if (inSubKey == 0)
								newSig = new pkiSig(newName,tmpString);
							else
								newSig = new pkiSig(newSubKey,tmpString);
						} // else it's an invalid key...
					} // we only load names/subkeys, therefore sigs, on public keyring!
					break;
				case 7:
					// fingerprint...
					bDebug() << "\tNew Fingerprint!" << endl;

					tmpString = getItem(buf,':',10);
					newKey->setFingerprint(tmpString);
					break;
				case 6:
					// subkey...
					if (mode == 1) {
						bDebug() << "\tNew Subkey!" << endl;
						// keyID
						tmpString.sprintf("0%s",getItem(buf,':',5));
						newSubKey = new pkiSubKey(newKey,tmpString);
	
						// next, key size...
						tmpString = getItem(buf,':',3);
						newSubKey->setKeySize(tmpString);
	
						// next, GPG key type...
						tmpString = getItem(buf,':',4);
						if (tmpString == "1") newSubKey->setKeyType("RSA");
						if (tmpString == "16") newSubKey->setKeyType("ElGamal");
						if (tmpString == "17") newSubKey->setKeyType("DSA");
						if (tmpString == "20") newSubKey->setKeyType("ElGamal");
						
						// revoked?
						tmpString = getItem(buf,':',2);
						if (tmpString[0].latin1() == 'r') newSubKey->isRevoked = TRUE;
					
						inSubKey = 1;
					}
					break;
				case 8:
				default:
					break;
			}
		}
	}
	fclose(outFile);
}

int be_gpg::importKey_File(const QString & fileName)
{
	int retVal = -1;
	QString appString;

	if (fileName.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a file!"));
		return -1;
	}

	appString = QString("%1 %2 --import \\\"%3\\\"").arg(execGPG).arg(keyringsToUse()).arg(fileName);
	retVal = runProcessFG(appString.latin1());

	return retVal;
}

int be_gpg::importKey_Server(const QString & keyID, const QString & keyServer)
{
	int retVal = -1;
	QString appString;
	KConfig *cfg = NULL;
	QString tmpOptions;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	cfg = kapp->config();
	if (cfg) {
		cfg->setGroup(profileName);
		if (cfg->readBoolEntry("HonorHTTPProxy",false))
		  tmpOptions += " --honor-http-proxy ";
	}
	appString = QString("%1 %2 %3 --keyserver %4 --recv-keys %5").arg(execGPG).arg(keyringsToUse()).arg(tmpOptions).arg(keyServer).arg(keyID);
	retVal = runProcessFG(appString.latin1());

	return retVal;
}

int be_gpg::editKey(const QString & keyID)
{
	int retVal = -1;
	QString appString;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	appString = QString("%1 %2 --edit-key %3").arg(execGPG).arg(keyringsToUse()).arg(keyID);
	retVal = runProcessFG(appString.latin1());

	return retVal;
}

int be_gpg::disEnableKey(const QString & keyID)
{
	int retVal = -1;
	QString appString;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	appString = QString("%1 %2 --edit-key %3").arg(execGPG).arg(keyringsToUse()).arg(keyID);
	retVal = runProcessFG(appString.latin1());

	return retVal;
}

int be_gpg::removeKey(const QString & keyID)
{
	QString appString;
	int retVal = -1;

	if (keyID.isEmpty()) {
		showBadKeyIDDialog();
		return -1;
	}

// FIXME deleting secret keys does'nt work until we get around to
// passing the pkiKey, then we can check this:
/*
	tmpKey = getKey(keyID);
	if (tmpKey) if (tmpKey->haveSecret) {
		appString = QString("%1 %2 --delete-secret-key %3").arg(pgpExecs.at(0)).arg(keyringsToUse()).arg(keyID);	
		retVal = runProcessFG(appString.data());
	}
*/
	appString = QString("%1 %2 --delete-key %3").arg(execGPG).arg(keyringsToUse()).arg(keyID);
	retVal = runProcessFG(appString.latin1());

	return retVal;
}

int be_gpg::revokeKey(const QString & keyID)
{
	QString appString, txtRevokCert;
	int retVal = -1;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	KMessageBox::information(NULL,i18n("When GnuPG revokes a key, it saves it as a Revocation Certificate.\nTo actually revoke the key, you must import this certificate,\nthen send it to the key servers and give it everyone who uses your key.\nI will launch GnuPG to revoke the key.\nAfter, YOU MUST IMPORT THAT REVOCATION CERTIFICATE TO ACTUALLY REVOKE YOUR KEY."),NULL,"NotifyGnuPG10RevokCert");
	txtRevokCert = KFileDialog::getSaveFileName(0,0,NULL,0);
	if (txtRevokCert.isNull()) return -1;
	appString = QString("%1 %2 -o %3 --gen-revoke %4").arg(execGPG).arg(keyringsToUse()).arg(txtRevokCert).arg(keyID);
	retVal = runProcessFG(appString.latin1());

	return retVal;
}

int be_gpg::revokeSignature(const QString & keyID)
{
	QString appString;
	int retVal = -1;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	appString = QString("%1 %2 --edit-key %3").arg(execGPG).arg(keyringsToUse()).arg(keyID);
	retVal = runProcessFG(appString.latin1());

	return retVal;
}

int be_gpg::addName(const QString & keyID)
{
	QString appString;
	int retVal = -1;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	appString = QString("%1 %2 --edit-key %3").arg(execGPG).arg(keyringsToUse()).arg(keyID);
	retVal = runProcessFG(appString.latin1());

	return retVal;
}

int be_gpg::genKeyPair()
{
	QString appString;
	int retVal = -1;

	appString = QString("%1 %2 --gen-key").arg(execGPG).arg(keyringsToUse());
	retVal = runProcessFG(appString.latin1());

	return retVal;
}

int be_gpg::exportKey_Server(const QString & keyID, const QString & keyServer)
{
	QString appString;
	int retVal = -1;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	appString = QString("%1 %2 --keyserver %3 --send-keys %4").arg(execGPG).arg(keyringsToUse()).arg(keyServer).arg(keyID);
	retVal = runProcessFG(appString.latin1());

	return retVal;
}

int be_gpg::exportKey_File(const QString & keyID,const QString & fileName)
{
	int retVal=-1;
	QString appString;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	appString = QString("%1 %2 -a -o \\\"%3\\\" --export %4").arg(execGPG).arg(keyringsToUse()).arg(fileName).arg(keyID);
	retVal = runProcessFG(appString.latin1());

	return retVal;
}

int be_gpg::signKey(const QString & keyIDToSign, const QString & signingKeyID)
{
	int retVal = -1;
	QString appString;

	if (keyIDToSign.isEmpty() || signingKeyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	if (KMessageBox::questionYesNo(0,i18n(
		"Do you want this signature to be exportable?\n"
		"(If not, this will be a local-only sign, others\n"
		"will not be able to use your signature.")
		,i18n("Exportable Signature")) == KMessageBox::Yes) {
			// exportable signature
			appString = QString("%1 %2 -u %3 --sign-key %4").arg(execGPG).arg(keyringsToUse()).arg(signingKeyID).arg(keyIDToSign);
	} else {
		// local only signature
			appString = QString("%1 %2 -u %3 --lsign %4").arg(execGPG).arg(keyringsToUse()).arg(signingKeyID).arg(keyIDToSign);
	}

	retVal = runProcessFG(appString.latin1());
	
	return retVal;
}

int be_gpg::decryptFile(const QString & file1, const QString & file2, bool detachedSig, bool justVerify)
{
	// Private function
	QString pgpAppString;
	int retVal = -1;

	if (!justVerify) {
		// we're decrypting...
		pgpAppString = QString("%1 -v %2 --output \\\"%3\\\" --decrypt \\\"%4\\\"").arg(execGPG).arg(keyringsToUse()).arg(file2).arg(file1);
	} else {
		// just verifying,
		if (detachedSig) {
			pgpAppString = QString("%1 -v %2 --verify \\\"%3\\\" \\\"%4\\\"").arg(execGPG).arg(keyringsToUse()).arg(file2).arg(file1);
		} else {
			pgpAppString = QString("%1 -v %2 --verify \\\"%3\\\"").arg(execGPG).arg(keyringsToUse()).arg(file1);
		}
	}

	retVal = runProcessFG(pgpAppString.latin1(),1);  // we want the shell to stay open
	return retVal;
}

int be_gpg::encryptFile(const QString & inFile, const QString & outFile, const QString & signingID, const QStringList & recipList,
	bool detachSig, bool signFile, bool encryptFile, bool asciiArmor, bool uniText, bool clearSign)
{
	QString pgpAppString, comment, tmpString;
	int retVal = -1;
	KConfig *cfg = NULL;

	cfg = kapp->config();
	cfg->setGroup("__Global");
	comment = cfg->readEntry("Comment", i18n("Made with Geheimnis"));

	pgpAppString = QString("%1 -v --no-batch ").arg(execGPG);
	tmpString = QString("--comment \\\"%1\\\" ").arg(comment);
	pgpAppString.append(tmpString);
	if (detachSig) pgpAppString.append(" -b ");
	if (encryptFile) pgpAppString.append(" -e ");
	if (signFile) {
		pgpAppString += QString(" -s -u %1 ").arg(signingID);
	}
	if (asciiArmor) pgpAppString.append(" -a ");
	if (clearSign) pgpAppString.append(" --clearsign ");
	if (uniText) pgpAppString.append(" -t ");
	// if we're not encrypting, recipList is just a " ", so we're safe...
	pgpAppString.append(keyringsToUse());
	pgpAppString.append(" ");
	pgpAppString.append(recipList.join(QString::fromLatin1(" ")));
	pgpAppString.append(" -o ");
//	pgpAppString.append(outFile);
//	pgpAppString.append(" ");
//	pgpAppString.append(inFile);
	pgpAppString += QString("\\\"%1\\\" "). arg(outFile);
	pgpAppString += QString("\\\"%1\\\""). arg(inFile);
	retVal = runProcessFG(pgpAppString.latin1());

	return retVal;
}







