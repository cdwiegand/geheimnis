
#include "myProfile.h"

#include "be_gpg.h"
#include "be_pgp2.h"
#include "be_pgp5.h"
#include "be_pgp6.h"
#include "myClipboard.h"
#include "myKeyring.h"
#include "myServerInterface.h"
#include "misc.h"
#include "bdebug.h"

#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>
#if QT_VERSION < 300
#include <kapp.h>
#else
#include <kapplication.h>
#endif

#include <qfile.h>
#include <qstrlist.h>
#include <qprogressdialog.h>
#if QT_VERSION < 300
#include <qlist.h>
#else
#include <qptrlist.h>
#ifndef QList
#define QList QPtrList
#endif
#endif

#include <unistd.h>

Profile::Profile(const int newPGPVersion, const QString & profileName)
{
	KConfig *cfg = NULL;

	cfg = kapp->config();
	if (cfg == NULL) return; // failed...

	if (!profileName.isEmpty()) {
		cfg->setGroup(profileName);
		cfg->writeEntry("PGPVersion",newPGPVersion);
		cfg->sync();
	}

	loadProfile(profileName);
}

Profile::Profile(const QString & profileName)
{
	loadProfile(profileName);
}

void Profile::loadProfile(const QString & newProfileName)
{
	KConfig *cfg = NULL;

	keyring = new pkiKeyring();
	backend = NULL;

	if (!newProfileName.isEmpty()) {
		profileName = newProfileName;
		bDebug() << "newProfileName = " << newProfileName << endl;
		cfg = kapp->config();
		if (cfg == NULL) return; // failed...
	
		cfg->setGroup(profileName);
		pgpVersion = cfg->readNumEntry("PGPVersion",0);
		bDebug() << "pgpVersion == " << pgpVersion << endl;
	
		switch (pgpVersion) {
			case 6:
				backend = new be_pgp6(profileName);
				break;
			case 5:
				backend = new be_pgp5(profileName);
				break;
			case 2:
				backend = new be_pgp2(profileName);
				break;
			case 1:
				backend = new be_gpg(profileName);
				break;
		}
		backend->setProfileName(profileName);
	} else {
		// called with no valid profile name! must be bad code - no pgpVersion for you!
		bDebug() << "loadProfile() called with NULL." << endl;
		pgpVersion = 0; // just to make sure!
	}
	
	bDebug() << "Profile returning" << endl;
	return;
}

Profile::~Profile() 
{
  if (keyring) delete keyring;
}

QString Profile::keyringsToUse(bool bIncludeSecret, bool bIncludePublic)
{
	if ( backend )
		return backend->keyringsToUse(bIncludeSecret, bIncludePublic);
	else return QString::null;
}

int Profile::getPGPVersion() 
{
	return pgpVersion;
}

bool Profile::keyServerCompatible()
{
	KConfig *cfg = kapp->config();
	if (cfg) {
		cfg->setGroup("__Global");
		if (cfg->readBoolEntry("UseCurl",false)) return true;
	}

	if ( backend )
		return backend->isKeyServerCompatible();
	else return false;
}

// -----------------------------------------------------------------

void Profile::loadKeyring(bool justRefreshTrust, const QString & keyID)
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

	QProgressDialog *winProgress = new QProgressDialog(i18n("Loading keyring..."),0,200);

	if (!justRefreshTrust && keyID.isEmpty()) keyring->clear();

	winProgress->setAutoClose(false);
	winProgress->setProgress(0);
	winProgress->show();
	qApp->processEvents();

	// Programmer's note: When scanning, **ALWAYS** check before adding a key, even if you only load the ring once.
	// This way, I will always be sure duplicates don't exist and everything links correctly.
	// Programmer's note 2: ONLY LOAD NAMES ONCE AND ONLY ONCE!!!!! -- chris and stefan

	// Modes are not globally defined - you can have as many modes as you want, please define at top of function

	qApp->processEvents();
	if ( backend )
		backend->loadKeyring(keyring,winProgress,justRefreshTrust,keyID);

	bDebug() << "Profile::loadKeyring about to return!" << endl;

	winProgress->hide();
	delete winProgress;
	qApp->processEvents();
	return;
}

pkiKey *Profile::getKey(const QString & keyID)
{
	return keyring->getKey(keyID);
}

pkiSubKey *Profile::getSubKey(const QString & subkeyID, QList<pkiSubKey> *subkeys)
{
	unsigned int l=0;
	if (!subkeys) return 0;
	for (l=0;l<subkeys->count();l++) {
		if (subkeys->at(l)->getKeyID() == subkeyID) return subkeys->at(l);
	}
	return 0;
}

pkiSig *Profile::getSig(const QString & keyID, QList<pkiSig> *sigs)
{
	unsigned int l=0;
	if (!sigs) return 0;
	for (l=0;l<sigs->count();l++) {
		if (sigs->at(l)->getSigningKeyID() == keyID) return sigs->at(l);
	}
	return 0;
}

int Profile::importKey_File(const QString & fileName)
{
	if (fileName.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a file!"));
		return -1;
	}

	if ( backend ) {
		int a = backend->importKey_File(fileName);
		loadKeyring(); // have to refresh the list...
		return a;
	} else return -1;
}

int Profile::importKey_Clipboard()
{
	QString tmpS;
	int retVal = 0;

	tmpS = getSafeTmpNameQ();

	myClipboard::saveFromClipboard(tmpS);
	retVal = importKey_File(tmpS);
	unlink(QFile::encodeName(tmpS));

	return retVal;
}

#if 0 // later...
int Profile::importKeyServer(QStrList *keyList, bool forceFullUpdate) {
  return importKeyServer( QStringList::fromStrList( *keyList ), forceFullUpdate );
}
#else
int Profile::importKey_Server(QStrList *keyList, bool forceFullUpdate) {
	unsigned int i = 0;
	int j = 0, k = 0;

	for (i=0;i<keyList->count();i++) {
		k = importKey_Server(QString(keyList->at(i)),FALSE);
		if (k != 0) j = -1;
	}
	if (forceFullUpdate) loadKeyring();
	return k;
}
#endif

int Profile::importKey_Server(const QStringList & keyList, bool forceFullUpdate) {
	int retVal = -1;

	for ( QStringList::ConstIterator it = keyList.begin() ;
	      it != keyList.end() ; ++it ) {
		int k = importKey_Server( (*it), false );
		if ( !k ) retVal = -1;
	}
	if (forceFullUpdate) loadKeyring();
	return retVal;
}

int Profile::importKey_Server(const QString & keyID, bool forceFullUpdate)
{
	int retVal = -1;
	QString tmpKey;
	bool bUseCurl = false;
	KConfig *cfg = NULL;
	QString keyServerString("");

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	cfg = kapp->config();
	if (cfg) {
		cfg->setGroup("__Global");
		keyServerString = cfg->readEntry("Keyserver","pgpkeys.mit.edu");
		bUseCurl = cfg->readBoolEntry("UseCurl",false);
	}

	if (bUseCurl) {
		myServerInterface *keyServer = new myServerInterface();
		tmpKey = getSafeTmpNameQ();
		keyServer->downloadKey(keyID,tmpKey);
		importKey_File(tmpKey);
		unlink(QFile::encodeName(tmpKey));
	} else {
		if ( backend )
			retVal = backend->importKey_Server(keyID,keyServerString);
		else return -1;
	}

	if (forceFullUpdate) loadKeyring();
	return retVal;
}

int Profile::editKey(const QString & keyID)
{
	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	if ( backend ) {
		int retVal = backend->editKey(keyID);
		loadKeyring();
		return retVal;
	} return -1;
}

int Profile::disEnableKey(const QString & keyID)
{
	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	if ( backend ) {
		int retVal = backend->disEnableKey(keyID);
		loadKeyring();
		return retVal;
	} else return -1;
}

int Profile::removeKey(const QString & keyID)
{
	// PRIVATE FUNCTION!!
	if (keyID.isEmpty()) {
		showBadKeyIDDialog();
		return -1;
	}

	if ( backend ) {
		int retVal = backend->removeKey(keyID);
		loadKeyring();
		return retVal;
	} else return -1;
}

int Profile::revokeKey(const QString & keyID)
{
	// PRIVATE FUNCTION!!
	int tmp = 0;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	// Note: warningContinueCancel doesn't have defaults like the others in QT 2.2.2 - look at the header file. ;( cdw
	tmp = KMessageBox::warningContinueCancel(NULL,i18n("Are you REALLY REALLY sure you want to revoke this key!! This may not be undoable!!"),i18n("Warning"),i18n("Continue"));
	if (tmp != KMessageBox::Continue) return -1;

	if ( backend ) {
		int retVal = backend->revokeKey(keyID);
		loadKeyring();
		return retVal;
	} else return -1;
}

int Profile::revokeSignature(const QString & keyID)
{
	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	if ( backend ) {
		int retVal = backend->revokeSignature(keyID);
		loadKeyring();
		return retVal;
	} else return -1;
}

int Profile::addName(const QString & keyID)
{
	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	if ( backend ) {
		int retVal = backend->addName(keyID);
		loadKeyring();
		return retVal;
	} else return -1;
}

int Profile::genKeyPair()
{
	if ( backend ) {
		int retVal = backend->genKeyPair();
		loadKeyring();
		return retVal;
	} else return -1;
}

int Profile::exportKey_Server(const QString & keyID)
{
	QString keyServer;
	KConfig *cfg;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	cfg = kapp->config();
	if (cfg) {
		cfg->setGroup("__Global");
		keyServer = cfg->readEntry("Keyserver","pgpkeys.mit.edu");
	}

	if ( backend )
		return backend->exportKey_Server(keyID,keyServer);
	else return -1;
}

int Profile::exportKey_Clipboard(const QString & keyID)
{
	QString tmpS;
	int retVal = 0;

	tmpS = getSafeTmpNameQ();
	retVal = exportKey_File(keyID,tmpS);
	myClipboard::saveIntoClipboard(tmpS);
	unlink(QFile::encodeName(tmpS));
	return retVal;
}

int Profile::exportKey_File(const QString & keyID,const QString & fileName)
{
	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	if ( backend )
		return backend->exportKey_File(keyID,fileName);
	else return -1;
}

int Profile::signKey(const QString & keyIDToSign, const QString & signingKeyID)
{
	if (keyIDToSign.isEmpty() || signingKeyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}

	if ( backend ) {
		int retVal = backend->signKey(keyIDToSign,signingKeyID);
		loadKeyring();
		return retVal;
	} else return -1;
}

int Profile::removeRevokeKey(const QString & keyID)
{
	// This function is meant to remove if the key selected isn't secret
	int retVal=-1;
	pkiKey *tmpKey = NULL;

	if (keyID.isEmpty()) {
		KMessageBox::sorry(NULL,i18n("You must enter or select a key!"));
		return -1;
	}
	tmpKey = getKey(keyID);

	if (tmpKey) {
		if (tmpKey->haveSecret) retVal = revokeKey(keyID);
		else retVal = removeKey(keyID);
	} else retVal = removeKey(keyID);

	// DO NOT "delete" KEY! It's part of a keyring!!!
	loadKeyring();
	return retVal;
}

int Profile::decryptFile(const QString & file1, const QString & file2, bool detachedSig, bool justVerify)
{
	if ( backend )
		return backend->decryptFile(file1,file2,detachedSig,justVerify);
	else return -1;
}

int Profile::encryptFile( const QString & inFile, const QString & outFile,
			    const QString & signingID,
			    const QString & recipList,
			    bool detachSig, bool signFile, bool encryptFile,
			    bool asciiArmor, bool uniText, bool clearSign )
{
	if ( backend )
	  return backend->encryptFile(inFile,outFile,signingID,recipList,detachSig,signFile,
				      encryptFile,asciiArmor,uniText,clearSign);
	else return -1;
}

int Profile::encryptFile( const QString & inFile, const QString & outFile,
			    const QString & signingID,
			    const QStringList & recipList,
			    bool detachSig, bool signFile, bool encryptFile,
			    bool asciiArmor, bool uniText, bool clearSign )
{
	if ( backend )
	  return backend->encryptFile(inFile,outFile,signingID,
				      recipList.join(" "),detachSig,signFile,
				      encryptFile,asciiArmor,uniText,clearSign);
	else return -1;
}

QString Profile::formatRecipIDForEncryption(const QString & recipKeyID)
{
	QString ret("");
	if (backend) {
		ret.sprintf(backend->recipFormat.latin1(),recipKeyID.latin1());
	}

	return ret;
}



