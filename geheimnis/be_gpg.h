
#ifndef PGPPROFILE1_H
#define PGPPROFILE1_H

#include "be_base.h"

#include <qstring.h>
#include <qstringlist.h>

class pkiKeyring;
class QProgressDialog;
class QCString;

// -----------------------------------------------------------------

class be_gpg : public be_base {
	public:
		be_gpg(const QString & profileName);
		virtual ~be_gpg() {}

		QString keyringsToUse(bool bIncludeSecret=TRUE, bool bIncludePublic=TRUE);
		void loadKeyring(pkiKeyring *keyring, QProgressDialog *winProg, bool justRefreshTrust, const QString & keyID);
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

	protected:
		QString unquoteGpgUid( const QCString & str);

	private:
		QString execGPG;
		void verifyDecryptFile(int mode, const QString & file1, const QString & file2);
		void loadGPGKeyring(pkiKeyring *keyring,const QString & appString, int mode, QProgressDialog *winProgress);
};

#endif





