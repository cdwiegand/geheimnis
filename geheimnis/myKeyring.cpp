
#include "myKeyring.h"
#include <klocale.h>

/*
 * --------------------------------- pkiSig ------------------------------
 */

void pkiKeyring::clear()
{
	theRing.clear();
}

void pkiKeyring::addKey(pkiKey *theKey)
{
	theRing.append(theKey);
}

void pkiKeyring::append(pkiKey *theKey)
{
	theRing.append(theKey);
}

pkiKey *pkiKeyring::at(int iIdx)
{
	return theRing.at(iIdx);
}

int pkiKeyring::count() const
{
	return theRing.count();
}
                
pkiKey *pkiKeyring::getKey(const QString & keyID)
{
        unsigned int l=0;
	QString tmpString;
        pkiKey *pk = NULL;
        for (l=0;l<theRing.count();l++) {
                pk = theRing.at(l); 
		tmpString = pk->getKeyID();
                if (tmpString == keyID) return pk;
        }
        return NULL;
}

pkiSig::pkiSig(pkiName *theName, const QString & signKeyID)
{
	signingKeyID = signKeyID;
	parentName = theName;
	parentSubKey = NULL;
	theName->sigs.append(this);
	return;
}

pkiSig::pkiSig(pkiSubKey *theSK, const QString & signKeyID)
{
	signingKeyID = signKeyID;
	parentName = NULL;
	parentSubKey = theSK;
	theSK->sigs.append(this);
	return;
}

pkiName *pkiSig::getParentName() const
{
	return parentName;
}

pkiSubKey *pkiSig::getParentSubKey() const
{
	return parentSubKey;
}

QString pkiSig::getSigningKeyID() const
{
	return signingKeyID;
}

pkiKey *pkiSig::getSigningKey() const
{
	pkiKeyring *a;
	pkiKey *b;
	// Have to find the profile, so we go up the chain...
	if (parentName) {
		b = parentName->getParentKey();
	} else {
		b = parentSubKey->getParentKey();
	}
	a = b->parentKeyring;
	return a->getKey(signingKeyID);
}

/*
 * --------------------------------- pkiName ------------------------------
 */

pkiName::pkiName(pkiKey *theKey, const QString & dName, const QString & eMail) 
{
	displayName = dName;
	emailAddress = eMail;
	parentKey = theKey;
	if (dName.length() > 0 && !theKey->names.isEmpty()) { // FIXME does this even work?
		// Okay, we'll see if perhaps we shouldn't place this at the end...
		if (theKey->names.at(0)->getDisplayName().left(1) == QString("*")) {
			theKey->names.insert(0,this);     // garbage in first name, so replace
		} else if (theKey->names.at(0)->getDisplayName().left(1) == QString("[")) {
			theKey->names.insert(0,this);     // garbage in first name, so replace
		} else
			theKey->names.append(this);
	} else
		theKey->names.append(this);
	isRevoked = 0;
	return;
}

pkiKey *pkiName::getParentKey() const
{
	return parentKey;
}

QString pkiName::getDisplayName() const
{
	return displayName;
}

QString pkiName::getEmailAddress() const
{
	return emailAddress;
}

/*
 * --------------------------------- pkiSubKey ----------------------------
 */

pkiSubKey::pkiSubKey(pkiKey *theKey, const QString & newSKeyID)
{
	keyID = newSKeyID;
	keySize = QString("0");
	keyType = i18n("Unknown");
	expireDate = QString("--------");
	createDate = QString("--------");
	isRevoked = 0;
	parentKey = theKey;
	theKey->subkeys.append(this);
	return;
}

pkiKey *pkiSubKey::getParentKey() const
{
	return parentKey;
}

QString pkiSubKey::getKeyID() const
{
	return keyID;
}

void pkiSubKey::setKeySize(const QString & a)
{
	keySize = a;
}

QString pkiSubKey::getKeySize() const
{
	return keySize;
}

void pkiSubKey::setKeyType(const QString & a)
{
	keyType = a;
}

QString pkiSubKey::getKeyType() const
{
	return keyType;
}

void pkiSubKey::setExpireDate(const QString & a)
{
	expireDate = a;
}

QString pkiSubKey::getExpireDate() const
{
	return expireDate;
}

void pkiSubKey::setCreateDate(const QString & a)
{
	createDate = a; 
}

QString pkiSubKey::getCreateDate() const
{
	return createDate;
}

/*
 * --------------------------------- pkiKey ------------------------------
 */

pkiKey::pkiKey(pkiKeyring *theKeyring, const QString & newKeyID)
{
	// defaults
	keyID = newKeyID;
	keySize = QString("0");
	keyType = i18n("Unknown");
	expireDate = QString("--------");
	createDate = QString("--------");
	haveSecret = FALSE;
	isExpired = 0;
	isRevoked = 0;
	isDisabled = 0;
	validity = 0;
	trust = 0;
	parentKeyring = theKeyring;
	theKeyring->addKey(this); // now we're double-linked...
	return;
}

QString pkiKey::getKeyID() const
{
	return keyID;
}

QString pkiKey::getKeySize() const
{
	return keySize;
}

void pkiKey::setKeySize(const QString & a)
{
	keySize = a;
}

QString pkiKey::getKeyType() const
{
	return keyType;
}

void pkiKey::setKeyType(const QString & a)
{
	keyType = a;
}

QString pkiKey::getFingerprint() const
{
	return fingerprint;
}

void pkiKey::setFingerprint(const QString & a)
{
	fingerprint = a;
}

QString pkiKey::getExpireDate() const
{
	return expireDate;
}

void pkiKey::setExpireDate(const QString & a)
{
	expireDate = a;
}

QString pkiKey::getCreateDate() const
{
	return createDate;
}

void pkiKey::setCreateDate(const QString & a)
{
	createDate = a;
}

int pkiKey::getValidity() const
{
	if (isExpired > 0 || isRevoked > 0 || isDisabled > 0)
		return 0;
	else
		return validity;
}

void pkiKey::setValidity(int i)
{
	validity = i;
}

int pkiKey::getTrust() const
{
	return trust;
}

void pkiKey::setTrust(int i)
{
	trust = i;
}



