
#ifndef PGPPROFILE5_H
#define PGPPROFILE5_H

#include "be_base.h"

#include <qstring.h>
#include <qstringlist.h>

class pkiKeyring;
class QProgressDialog;

// -----------------------------------------------------------------

class be_pgp5 : public be_base {
	public:
		be_pgp5(const QString & profileName);
		virtual ~be_pgp5() {}

		QString keyringsToUse(bool bIncludeSecret=TRUE, bool bIncludePublic=TRUE);
		void loadKeyring(pkiKeyring *keyring, QProgressDialog *winProgress, bool justRefreshTrust=FALSE, const QString & keyID=QString::null);

		int editKey(const QString & keyID);
		int signKey(const QString & keyIDToSign, const QString & signingKeyID);
		int revokeSignature(const QString & sigID);
		int disEnableKey(const QString & keyID);
		int removeKey(const QString & keyID);
		int revokeKey(const QString & keyID);
		int addName(const QString & keyID);
		int genKeyPair();

		int importKey_Server(const QString & keyID, const QString & keyServer); // requires a KeyID
		int importKey_File(const QString & fileName);    // requires a file path
		int exportKey_Server(const QString & keyID, const QString & keyServer);
		int exportKey_File(const QString & keyID, const QString & fileName);

		int decryptFile(const QString & inFile, const QString & outFile, bool detachSig, bool justVerify);
		int encryptFile(const QString & inFile, const QString & outFile, const QString & signingID, const QStringList & recipList,
			bool detachSig, bool signFile, bool encryptFile, bool asciiArmor, bool uniText, bool clearSign);
	
		bool isKeyServerCompatible() { return true; }

	private:
		QString execPGPk;
		QString execPGPs;
		QString execPGPe;
		QString execPGPv;
		void verifyDecryptFile(int mode, const QString & file1, const QString & file2);
		void loadPGP5Keyring(pkiKeyring *keyring,const QString & appString, QProgressDialog *winProgress);
 		void loadPGP65Validity(pkiKeyring *keyring,const QString & appString, QProgressDialog *winProgress);
};

#endif






