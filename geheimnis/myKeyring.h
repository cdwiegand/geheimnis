
#ifndef GEHEIMNIS_MYKEYRING_H
#define GEHEIMNIS_MYKEYRING_H

#include <qstring.h>
#if QT_VERSION < 300
#include <qlist.h>
#else
#include <qptrlist.h>
#ifndef QList
#define QList QPtrList
#endif
#endif

class pkiSig;
class pkiName;
class pkiKey;
class pkiKeyring;
class pkiSubKey;

class pkiSig {
	public:
		pkiSig(pkiName *theName, const QString & signKeyID);
		pkiSig(pkiSubKey *theSK, const QString & signKeyID);

		pkiName *getParentName() const;
		pkiSubKey *getParentSubKey() const;
		QString getSigningKeyID() const;
		pkiKey *getSigningKey() const;

	private:		
		pkiName *parentName; // One of these WILL be null...
		pkiSubKey *parentSubKey;
		QString signingKeyID;
};

class pkiName {
	public:
		pkiName(pkiKey *theKey, const QString & dName, const QString & eMail=QString::null);

		pkiKey *getParentKey() const;
		QString getDisplayName() const;
		QString getEmailAddress() const;
		bool isRevoked;
		QList<pkiSig> sigs;
		pkiKey *parentKey;
		
	private:
		QString emailAddress;
		QString displayName;
};

class pkiSubKey {
	public:
		pkiSubKey(pkiKey *theKey, const QString & newSKeyID);

		pkiKey *getParentKey() const;
		QString getKeyID() const; // must be set in constructor
		QString getKeySize() const;
		void setKeySize(const QString & a);
		QString getKeyType() const;
		void setKeyType(const QString & a);
		bool isRevoked;
		QString getExpireDate() const;
		void setExpireDate(const QString & a);
		QString getCreateDate() const; // FIXME: QDate??
		void setCreateDate(const QString & a);
		QList<pkiSig> sigs;
		pkiKey *parentKey;

	private:		
		QString expireDate;
		QString createDate;
		QString keyID;
		QString keySize;
		QString keyType;
};

class pkiKey {
	public:
		pkiKey(pkiKeyring *theKeyring, const QString & newKeyID); // set defaults

		QString getKeyID() const;
		QString getKeySize() const;
		void setKeySize(const QString & a);
		QString getKeyType() const;
		void setKeyType(const QString & a);
		QString getFingerprint() const;
		void setFingerprint(const QString & a);
		QList<pkiName> names;
		QList<pkiSubKey> subkeys;
		bool haveSecret;
		bool isRevoked;
		bool isExpired;
		bool isDisabled;
		QString getExpireDate() const;
		void setExpireDate(const QString & a);
		QString getCreateDate() const;
		void setCreateDate(const QString & a);
		int getValidity() const;
		void setValidity(int i);
		int getTrust() const;
		void setTrust(int i);
		pkiKeyring *parentKeyring;

	private:
		int trust;      // How much do we trust keys he's signed?
		int validity;   // Is this key signed by people we trust?
		QString expireDate; // DEPRECATE FIXME
		QString createDate; // DEPRECATE FIXME
		QString keyType;
		QString keyID;
		QString keySize;
		QString fingerprint;

};

class pkiKeyring {
	public:
		void addKey(pkiKey *theKey);
		void append(pkiKey *theKey); // compat
		void clear();
		pkiKey *at(int iIdx);
		int count() const;
		pkiKey *getKey(const QString & keyID);

	private:
		QList<pkiKey> theRing;
};

// -----------------------------------------------------------------

/* Trust/Validity Values:
 * 0 = Unknown/Not Assigned
 * 1 = Not Trusted/Valid
 * 2 = Marginally Trusted/Valid
 * 3 = Fully Trusted/Valid
 * 4 = Implicit/Ultimate Trust/Validity
 */

// -----------------------------------------------------------------
 
#include "myProfile.h"

#endif


