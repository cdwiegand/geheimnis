// -*- c++ -*-
#ifndef PGPPROFILEBASE_H
#define PGPPROFILEBASE_H

#include <qstring.h>
#include <qstringlist.h>

class pkiKeyring;
class QProgressDialog;

/** @short interface for backend classes.
    @author Chris Wiegand <cwiegand@users.sf.net>
*/

class be_base {
	public:
  be_base() {}
  virtual ~be_base() {}
		// common to all backends - code in be_base:
		void setProfileName(const QString & aProfileName) { profileName = aProfileName; }

		// common to all backends - code in real backend:
		virtual QString keyringsToUse(bool bIncludeSecret=TRUE, bool bIncludePublic=TRUE) = 0;
		virtual void loadKeyring(pkiKeyring *keyring, QProgressDialog *winProg, bool justRefreshTrust=FALSE, const QString & keyID=QString()) = 0;

		virtual int editKey(const QString & keyID) = 0;
		virtual int signKey(const QString & keyIDToSign, const QString & signingKeyID) = 0;
		virtual int revokeSignature(const QString & sigID) = 0;
		virtual int disEnableKey(const QString & keyID) = 0;
		virtual int removeKey(const QString & keyID) = 0;
		virtual int revokeKey(const QString & keyID) = 0;
		virtual int addName(const QString & keyID) = 0;
		virtual int genKeyPair() = 0;

		virtual int importKey_Server(const QString & keyID, const QString & keyServer) = 0; // requires a KeyID
		virtual int importKey_File(const QString & fileName) = 0; // requires a file path
		virtual int exportKey_Server(const QString & keyID, const QString & keyServer) = 0;
		virtual int exportKey_File(const QString & keyID, const QString & fileName) = 0;

		virtual int decryptFile(const QString & inFile, const QString & outFile, bool detachSig, bool justVerify) = 0;
		virtual int encryptFile(const QString & inFile, const QString & outFile, const QString & signingID, const QStringList & recipList,
			bool detachSig, bool signFile, bool encryptFile, bool asciiArmor, bool uniText, bool clearSign) = 0;
	
		virtual bool isKeyServerCompatible() { return FALSE; }
		QString recipFormat;

	// protected variable, all backends have these things:
protected:
		QStringList pgpExecs;
		QString profileName;
		QStringList secretKeyringFiles;
		QStringList publicKeyringFiles;
};

#endif








