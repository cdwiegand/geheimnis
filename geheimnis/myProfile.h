// -*- c++ -*-

#ifndef PGPPROFILE_H
#define PGPPROFILE_H

#include <qstring.h>

class myClipboard;
class myKeyring;
class myServerInterface;

#if QT_VERSION < 300
template <typename T> class QList;
# ifndef QPtrList
#  define QPtrList QList
# endif
#else
template <typename T> class QPtrList;
# ifndef QList
#  define QList QPtrList
# endif
#endif

class pkiSubKey;
class pkiKey;
class pkiKeyring;
class pkiSig;
class QStrList;
class QStringList;
class be_base;
class ProfileManager;


class Profile {
  friend class ProfileManager;
		Profile(const QString & profileName); // for existing profiles
		Profile(const int pgpVersion, const QString & profileName); // for new profiles
		~Profile();
	public:

		QString name() const { return profileName; }
		// keyring stuff (non-be dependant)
		pkiKey *getKey(const QString & keyID);
		pkiSubKey *getSubKey(const QString & subKeyID, QList<pkiSubKey> *subkeys);
		pkiSig *getSig(const QString & keyID, QList<pkiSig> *sigs);

		// be stuff just passed on to the backend-> area...
		// except the clipboard, and importKeyServer(QStrList), which are done by us..
		QString keyringsToUse(bool bIncludeSecret=TRUE, bool bIncludePublic=TRUE);
		void loadKeyring(bool justRefreshTrust=FALSE, const QString & keyID=QString::null);
		int editKey(const QString & keyID);
		int signKey(const QString & keyIDToSign, const QString & signingKeyID);
		int revokeSignature(const QString & sigID);
		int revokeKey(const QString & keyID);
		int removeKey(const QString & keyID);
		int removeRevokeKey(const QString & keyID);
		int disEnableKey(const QString & keyID);
		int addName(const QString & keyID);
		int genKeyPair();
		int importKey_Server(const QString & keyID, bool forceFullUpdate=TRUE); // requires a KeyID
		int importKey_Server(QStrList *keyList, bool forceFullUpdate=TRUE); // supports multiple keyIDs
		int importKey_Server(const QStringList & keyList, bool forceFullUpdate=TRUE); // supports multiple keyIDs
		int importKey_File(const QString & fileName);    // requires a file path
		int importKey_Clipboard();
		int exportKey_Server(const QString & keyID);
		int exportKey_File(const QString & keyID, const QString & fileName);
		int exportKey_Clipboard(const QString & keyID);
		int decryptFile(const QString & inFile, const QString & outFile, bool detachSig, bool justVerify);
		int encryptFile(const QString & inFile, const QString & outFile, const QString & signingID, const QString & recipList,
			bool detachSig, bool signFile, bool encryptFile, bool asciiArmor, bool uniText, bool clearSign);
		int encryptFile(const QString & inFile, const QString & outFile, const QString & signingID, const QStringList & recipList,
			bool detachSig, bool signFile, bool encryptFile, bool asciiArmor, bool uniText, bool clearSign);

		// other stuff....
		QString formatRecipIDForEncryption(const QString & recipKeyID);
		pkiKeyring *keyring;
		int getPGPVersion();
		bool keyServerCompatible();

	private:
		QString profileName;
		int pgpVersion;
			// PGP 7.0 = 7
			// PGP 6.5 = 6
			// PGP 5   = 5
			// PGP 2.6 = 2
			// GnuPG   = 1
		void loadProfile(const QString & profileName);
		be_base *backend;
};

#endif




