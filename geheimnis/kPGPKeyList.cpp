
#include "kPGPKeyList.h"

#include "kPGPKeyAdd.h"
#include "kPGPKeyExport.h"
#include "kPGPSelectKey.h"
#include "myProfile.h"
#include "kPGPKeyProps.h"
#include "myKeyring.h"
#include "misc.h"
#include "gdebug.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kiconloader.h>
#if QT_VERSION < 300
#include <kapp.h>
#else
#include <kapplication.h>
#endif

#include <qsize.h>
#include <qpopupmenu.h>
#include <qprogressdialog.h>
#include <qstringlist.h>
#include <qstring.h>
#include <qpoint.h>
#include <qevent.h>
#include <qlayout.h>
#include <qframe.h>
#include <qdragobject.h>
#if QT_VERSION < 300
#include <qlist.h>
#define QPtrList QList
#else
#include <qptrlist.h>
#define QList QPtrList
#endif

#include "kPGPKeyList.moc"

kPGPKeyList::kPGPKeyList(QWidget *parent, int colOptions) : KListView(parent) {
	QString sFile;
	KConfig *cfg = NULL;

	currentProfile = NULL;
	flatView = FALSE;
	showOnlySecretKeys = FALSE;
	showNamesOnly = FALSE;
	showKeysOnly = FALSE;
	allowMultipleKeys = FALSE;
	showOnlyGoodKeys = FALSE;

	mnuKeys = new QPopupMenu();
	mnuPKey = new QPopupMenu();
	mnuSKey = new QPopupMenu();
	mnuSigs = new QPopupMenu();

	/* ************************************************************************************
	 * Load Menubar
	 */

	mnuKeys->insertItem(i18n("&Generate Key Pair..."), ID_GENKEY);

	mnuKeys->insertItem(i18n("&Import Key..."), ID_IMPORTKEY);
	mnuSKey->insertItem(i18n("&Update Key..."), ID_IMPORTKEY);
	mnuPKey->insertItem(i18n("&Update Key(s)..."), ID_IMPORTKEY);
	mnuSigs->insertItem(i18n("&Import Key..."), ID_IMPORTKEY);

	mnuKeys->insertItem(i18n("&Export Public Key..."), ID_EXPORTKEY);
	mnuSKey->insertItem(i18n("&Export Public Key..."), ID_EXPORTKEY);
	mnuPKey->insertItem(i18n("&Export Public Key..."), ID_EXPORTKEY);

	mnuKeys->insertItem(i18n("Key I&nfo..."), ID_KEYINFO);
	mnuSKey->insertItem(i18n("Key I&nfo..."), ID_KEYINFO);
	mnuPKey->insertItem(i18n("Key I&nfo..."), ID_KEYINFO);
	mnuSigs->insertItem(i18n("Signing Key I&nfo..."), ID_KEYINFO);

	mnuKeys->insertItem(i18n("&Edit Key..."), ID_EDITKEY);
	mnuSKey->insertItem(i18n("&Edit Key..."), ID_EDITKEY);
	mnuPKey->insertItem(i18n("&Edit Key..."), ID_EDITKEY);
	mnuSigs->insertItem(i18n("&Edit Signing Key..."), ID_EDITKEY);

	mnuKeys->insertItem(i18n("&Disable/Enable Key..."), ID_DISENABLEKEY);
	mnuSKey->insertItem(i18n("&Disable/Enable Key..."), ID_DISENABLEKEY);
	mnuPKey->insertItem(i18n("&Disable/Enable Key..."), ID_DISENABLEKEY);

	mnuKeys->insertItem(i18n("Re&move Key..."), ID_REMOVEKEY);
	mnuSKey->insertItem(i18n("Re&move Key..."), ID_REMOVEKEY);
	mnuPKey->insertItem(i18n("Re&move Key..."), ID_REMOVEKEY);

	mnuKeys->insertItem(i18n("Re&voke Key..."), ID_REVOKEKEY);
	mnuSKey->insertItem(i18n("Re&voke Key..."), ID_REVOKEKEY);

	mnuKeys->insertSeparator();
	mnuSKey->insertSeparator();
	mnuPKey->insertSeparator();

	mnuKeys->insertItem(i18n("&Add Name..."), ID_ADDNAME);
	mnuSKey->insertItem(i18n("&Add Name..."), ID_ADDNAME);

	mnuKeys->insertItem(i18n("&Sign Key..."), ID_SIGNKEY);
	mnuSKey->insertItem(i18n("&Sign Key..."), ID_SIGNKEY);
	mnuPKey->insertItem(i18n("&Sign Key..."), ID_SIGNKEY);

	mnuKeys->insertItem(i18n("Revo&ke Signature..."), ID_REVOKESIG);
	mnuSigs->insertItem(i18n("Revo&ke Signature..."), ID_REVOKESIG);

	mnuKeys->insertSeparator();

	mnuKeys->insertItem(i18n("&Reload Keyring"), ID_RELOADKEYRING);

	connect(mnuKeys,SIGNAL(activated(int)),SLOT(mnuKeys_selected(int)));
	connect(mnuSKey,SIGNAL(activated(int)),SLOT(mnuKeys_selected(int)));
	connect(mnuPKey,SIGNAL(activated(int)),SLOT(mnuKeys_selected(int)));
	connect(mnuSigs,SIGNAL(activated(int)),SLOT(mnuKeys_selected(int)));
	
	cfg = kapp->config();
	if (!cfg) return;

	/* ***************************************************************8
	 * Load pixmaps
	 */
	KIconLoader *ldr = kapp->iconLoader();
	pixKeySec = ldr->loadIcon("icon_key_sec.xpm",KIcon::User);
	pixKeyPub = ldr->loadIcon("icon_key_pub.xpm",KIcon::User);
	pixKeyPub2 = ldr->loadIcon("icon_key_pub2.xpm",KIcon::User);
	pixKeyExp = ldr->loadIcon("icon_key_expired.xpm",KIcon::User);
	pixKeyRev = ldr->loadIcon("icon_key_revoked.xpm",KIcon::User);
	pixKeyDis = ldr->loadIcon("icon_key_disabled.xpm",KIcon::User);

	pixNameSec = ldr->loadIcon("icon_name_sec.xpm",KIcon::User);
	pixNamePub = ldr->loadIcon("icon_name_pub.xpm",KIcon::User);
	pixNamePub2 = ldr->loadIcon("icon_name_pub2.xpm",KIcon::User);
	pixNameRev = ldr->loadIcon("icon_name_bad.xpm",KIcon::User);

	pixSigSec = ldr->loadIcon("icon_sig_sec.xpm",KIcon::User);
	pixSigPub = ldr->loadIcon("icon_sig_pub.xpm",KIcon::User);
	pixSigPub2 = ldr->loadIcon("icon_sig_pub2.xpm",KIcon::User);

	pixTrust0 = ldr->loadIcon("trust_0.xpm",KIcon::User);
	pixTrust1 = ldr->loadIcon("trust_1.xpm",KIcon::User);
	pixTrust2 = ldr->loadIcon("trust_2.xpm",KIcon::User);
	pixTrust3 = ldr->loadIcon("trust_3.xpm",KIcon::User);
	pixTrust4 = ldr->loadIcon("trust_4.xpm",KIcon::User);

	pixValid0 = ldr->loadIcon("valid_0.xpm",KIcon::User);
	pixValid1 = ldr->loadIcon("valid_1.xpm",KIcon::User);
	pixValid2 = ldr->loadIcon("valid_2.xpm",KIcon::User);
	pixValid3 = ldr->loadIcon("valid_3.xpm",KIcon::User);
	pixValid4 = ldr->loadIcon("valid_4.xpm",KIcon::User);

	/* ************************************************************************
	 * Load Profiles / Prefs
	 */
	
	cfg->setGroup("__Key Management");
	showName = cfg->readBoolEntry("Name",TRUE);
	showEmail = cfg->readBoolEntry("Email",TRUE);
	showSize = cfg->readBoolEntry("Size",TRUE);
	showKeyID = cfg->readBoolEntry("KeyID",TRUE);
	showType = cfg->readBoolEntry("Type",TRUE);
	showTrust = cfg->readBoolEntry("Trust",TRUE);
	showValidity = cfg->readBoolEntry("Validity",TRUE);
	
	// Now, override from constructor...
	showName = (!(colOptions & KPGPKEYS_HIDENAME));
	showEmail = (!(colOptions & KPGPKEYS_HIDEEMAIL));
	showSize = (!(colOptions & KPGPKEYS_HIDESIZE));
	showKeyID = (!(colOptions & KPGPKEYS_HIDEKEYID));
	showType = (!(colOptions & KPGPKEYS_HIDETYPE));
	showTrust = (!(colOptions & KPGPKEYS_HIDETRUST));
	showValidity = (!(colOptions & KPGPKEYS_HIDEVALIDITY));

	if (showName) addColumn(i18n("Name"));
	if (showEmail) addColumn(i18n("Email"));
	if (showSize) addColumn(i18n("Size"));
	if (showKeyID) addColumn(i18n("KeyID"));
	if (showType) addColumn(i18n("Type"));
	if (showTrust)  addColumn(i18n("Trust"));
	if (showValidity) addColumn(i18n("Validity"));
	setMultiSelection(allowMultipleKeys);
	setAllColumnsShowFocus(true);
	connect(this,SIGNAL(rightButtonClicked(QListViewItem *,const QPoint &,int)),
		this,SLOT(slotRightClick(QListViewItem *,const QPoint &,int)));
	setAcceptDrops(true);
	cfg->setGroup("__Global");
	flatView = (!cfg->readBoolEntry("KeyTreeView",TRUE));

	setShowSortIndicator( true );
}

QPopupMenu *kPGPKeyList::getMenu() {
	return mnuKeys;
}

void kPGPKeyList::setFlatView(bool value) {
	flatView = value;
}

void kPGPKeyList::setShowOnlyGoodKeys(bool value) {
	showOnlyGoodKeys = value;
}

void kPGPKeyList::setShowOnlySecretKeys(bool value) {
	showOnlySecretKeys = value;
}

void kPGPKeyList::setShowNamesOnly(bool value) {
	showNamesOnly = value;
}

void kPGPKeyList::setShowKeysOnly(bool value) {
	showKeysOnly = value;
}

void kPGPKeyList::setAllowMultipleKeys(bool value) {
	allowMultipleKeys = value;
}

void kPGPKeyList::loadKeyring()
{
	// This profile requires that a good profile is loaded!
	QListViewItem *lviKey = NULL, *lviName = NULL, *lviSig = NULL;
	QList<pkiName> namering;
	QList<pkiSubKey> subkeys;
	QList<pkiSig> sigring;

	pkiKey *thisKey = NULL;       // Do NOT "delete" these objects, it would break the keyring!
	pkiName *thisName = NULL;     //  |
	pkiSubKey *thisSubKey = NULL; //  |
	pkiSig *thisSig = NULL;       //  v
	pkiKey *signKey = NULL;       // thru here...

	int i=0;
	unsigned int j=0, k=0;
	QString keyIconName;
	QString strUnknown(i18n("Unknown"));
	QString str4Q(i18n("????"));
	QListViewItem *lviValid, *lviDunno, *lviSecretKeys, *lviRevokedExpiredDisabled;
	QListViewItem *lviOne;

	if (currentProfile == NULL) return;
	if (currentProfile->getPGPVersion() == 0) return;

	clear();
	setRootIsDecorated(!flatView);
	setMultiSelection(allowMultipleKeys);
	bool wasEnabled = isEnabled();
	setEnabled(false);

	if (flatView) {
		lviSecretKeys = lviRevokedExpiredDisabled = lviValid = lviDunno = NULL;
	} else {
		lviValid = new QListViewItem(this,i18n("Known Valid Keys"));
		lviValid->setPixmap(0,pixKeyPub2);
		lviDunno = new QListViewItem(this,i18n("Possibly Valid Keys"));
		lviDunno->setPixmap(0,pixKeyPub);
		lviSecretKeys = new QListViewItem(this,i18n("Secret Key Pairs"));
		lviSecretKeys->setPixmap(0,pixKeySec);
		lviRevokedExpiredDisabled = new QListViewItem(this,i18n("Revoked / Expired / Disabled Keys"));
		lviRevokedExpiredDisabled->setPixmap(0,pixKeyRev);
	}

	for (i=0;i<currentProfile->keyring->count();i++) {
		// Per KEY

		thisKey = currentProfile->keyring->at(i);
		if (thisKey->haveSecret)
			keyIconName = QString("%1 Key Pair").arg(thisKey->getKeyType());
		else
			keyIconName = QString("%1 Public Key").arg(thisKey->getKeyType());
		namering = thisKey->names;
		subkeys = thisKey->subkeys;

		lviOne = lviDunno; // default...
		if (thisKey->getValidity() >= 2) lviOne = lviValid;
		if (thisKey->haveSecret) lviOne = lviSecretKeys;
		if (thisKey->isRevoked || thisKey->isExpired || thisKey->isDisabled) lviOne = lviRevokedExpiredDisabled;

		// Okay, here, we start to process keys...
		// We disqualify until it passes all tests...
		if ((thisKey->isRevoked || thisKey->isExpired || thisKey->isDisabled) && showOnlyGoodKeys) {
			// do nothing...
		} else if (showOnlySecretKeys && !thisKey->haveSecret) {
			// do nothing
		} else {
			if (!showNamesOnly) {
				// if we're not just showing names (used in encryption dialog), then show keys, names, and sigs...
				if (namering.count() > 0)
					lviKey = addLine(lviOne,namering.at(0)->getDisplayName(),namering.at(0)->getEmailAddress(),thisKey->getKeySize(),thisKey->getKeyID(),keyIconName,thisKey->getTrust(),thisKey->getValidity());
				else
					lviKey = addLine(lviOne,strUnknown,str4Q,thisKey->getKeySize(),thisKey->getKeyID(),keyIconName,thisKey->getTrust(),thisKey->getValidity());
		
				if (thisKey->isRevoked)
					lviKey->setPixmap(0,pixKeyRev);
				else if (thisKey->isExpired)
					lviKey->setPixmap(0,pixKeyExp);
				else if (thisKey->isDisabled)
					lviKey->setPixmap(0,pixKeyDis);
				else if (thisKey->haveSecret)
					lviKey->setPixmap(0,pixKeySec);
				else if (thisKey->getValidity() >= 2)
					lviKey->setPixmap(0,pixKeyPub2);
				else
					lviKey->setPixmap(0,pixKeyPub);
				
			}
	
			if (!showKeysOnly) {
				for (j=0;j<namering.count();j++) {
					// Per NAME
		
					thisName = namering.at(j);
					sigring = thisName->sigs;
		
					if (showNamesOnly) 
						lviName = addLine(NULL,thisName->getDisplayName(),thisName->getEmailAddress(),thisKey->getKeySize(),thisKey->getKeyID(),i18n("Display Name"),-1,thisKey->getValidity());
					else
						lviName = addLine(lviKey,thisName->getDisplayName(),thisName->getEmailAddress(),thisKey->getKeySize(),thisKey->getKeyID(),i18n("Display Name"),-1,thisKey->getValidity());

					if (thisName->isRevoked) {
						lviName->setPixmap(6,pixValid0);  // Validity = 0
						lviName->setPixmap(0,pixNameRev);  // Symbol for revoked name
					} else {
						if (thisKey->haveSecret) {
							lviName->setPixmap(0,pixNameSec);
						} else {
							if (thisKey->getValidity() > 2) lviName->setPixmap(0,pixNamePub2);
							else lviName->setPixmap(0,pixNamePub);
						}
					}
		
					if (!showNamesOnly) {
						for (k=0;k<sigring.count();k++) {
							// Per SIGNATURE
			
							thisSig = sigring.at(k);
							signKey = currentProfile->getKey(thisSig->getSigningKeyID());
							if (signKey != NULL) {
								if (signKey->haveSecret) {
									keyIconName = i18n("Personal Signature");
								} else {
									if (signKey->getValidity() > 2 && signKey->getTrust() > 2)
									keyIconName = i18n("Trusted Signature");
								else
									keyIconName = i18n("Untrusted Signature");
							}
		
							if (signKey->names.count() > 0)
								lviSig = addLine(lviName,signKey->names.at(0)->getDisplayName(),signKey->names.at(0)->getEmailAddress(),signKey->getKeySize(),signKey->getKeyID(),keyIconName,signKey->getTrust(),signKey->getValidity());
							else
								lviSig = addLine(lviName,strUnknown,str4Q,signKey->getKeySize(),signKey->getKeyID(),keyIconName,signKey->getTrust(),signKey->getValidity());
		
							if (signKey->haveSecret) {
								lviSig->setPixmap(0,pixSigSec);
							} else {
								if (signKey->getValidity() > 2 && signKey->getTrust() > 2)
									lviSig->setPixmap(0,pixSigPub2);
								else
									lviSig->setPixmap(0,pixSigPub);
								}
							} else {
								lviSig = addLine(lviName,strUnknown,str4Q,str4Q,thisSig->getSigningKeyID(),i18n("Unknown Signature"),0,0);
								lviSig->setPixmap(0,pixSigPub);
							}	
						}
					}					
				}
	
				if (!showNamesOnly) {
					for (j=0;j<subkeys.count();j++) {
						// Per SUBKEY
			
						thisSubKey = subkeys.at(j);
						keyIconName = QString(i18n("%1 Subkey")).arg(thisSubKey->getKeyType());
						sigring = thisSubKey->sigs;
						if (namering.count() > 0)
							lviName = addLine(lviKey,namering.at(0)->getDisplayName(),namering.at(0)->getEmailAddress(),thisSubKey->getKeySize(),thisSubKey->getKeyID(),keyIconName,-1,thisKey->getValidity());
						else
							lviName = addLine(lviKey,strUnknown,str4Q,thisSubKey->getKeySize(),thisSubKey->getKeyID(),keyIconName,-1,thisKey->getValidity());
		
						if (thisSubKey->isRevoked) {
							lviName->setPixmap(6,pixValid0);  // Validity = 0
							lviName->setPixmap(0,pixKeyRev);  // Symbol for revoked keys
						} else {
							if (thisKey->haveSecret) {
								lviName->setPixmap(0,pixKeySec);
							} else {
								if (thisKey->getValidity() > 2) lviName->setPixmap(0,pixKeyPub2);
								else lviName->setPixmap(0,pixKeyPub);
							}
						}
			
						for (k=0;k<sigring.count();k++) {
							// Per SIGNATURE
			
							thisSig = sigring.at(k);
							signKey = currentProfile->getKey(thisSig->getSigningKeyID());
							if (signKey != NULL) {
								if (signKey->haveSecret) {
									keyIconName = i18n("Personal Signature");
								} else {
									if (signKey->getValidity() > 2 && signKey->getTrust() > 2)
										keyIconName = i18n("Trusted Signature");
									else
										keyIconName = i18n("Untrusted Signature");
								}
			
								if (signKey->names.count() > 0)
									lviSig = addLine(lviName,signKey->names.at(0)->getDisplayName(),signKey->names.at(0)->getEmailAddress(),signKey->getKeySize(),signKey->getKeyID(),keyIconName,signKey->getTrust(),signKey->getValidity());
								else
									lviSig = addLine(lviName,strUnknown,str4Q,signKey->getKeySize(),signKey->getKeyID(),keyIconName,signKey->getTrust(),signKey->getValidity());
			
								if (signKey->haveSecret) {
									lviSig->setPixmap(0,pixSigSec);
								} else {
									if (signKey->getValidity() > 2 && signKey->getTrust() > 2)
										lviSig->setPixmap(0,pixSigPub2);
									else
										lviSig->setPixmap(0,pixSigPub);
								}
							} else {
								lviSig = addLine(lviName,i18n("Unknown Signature"),"",i18n("????"), thisSig->getSigningKeyID(),i18n("Unknown Signature"),0,0);
								lviSig->setPixmap(0,pixSigPub);
							}
						}					
					}
				}
			}
		}
	}
	// Now, if any of the four top-level children are empty, remove them for a nice-looking window
	if (!flatView) {
		if (lviSecretKeys->childCount() == 0) delete lviSecretKeys;
		if (lviRevokedExpiredDisabled->childCount() == 0) delete lviRevokedExpiredDisabled;
		if (lviValid->childCount() == 0) delete lviValid;
		if (lviDunno->childCount() == 0) delete lviDunno;
	}
	if ( wasEnabled )
	  setEnabled(true);
}

QListViewItem *kPGPKeyList::addLine(QListViewItem *lvParent, const QString & displayName, const QString & emailAddress, const QString & keySize, const QString & keyID, const QString & typeText, const int trust, const int validity)
{
	int z = 0;
	QListViewItem *lviKey = NULL;
	if (lvParent) // subkey, name, or signature
		lviKey = new QListViewItem(lvParent);
	else
		lviKey = new QListViewItem(this);

	if (showName) { lviKey->setText(z,displayName); z++; }
	if (showEmail) { lviKey->setText(z,emailAddress); z++; }
	if (showSize) { lviKey->setText(z,keySize); z++; }
	if (showKeyID) { lviKey->setText(z,keyID); z++; }
	if (showType) { lviKey->setText(z,typeText); z++; }
	if (showTrust) {
		switch(trust) {
			case -1: // Trust not applicable
			        lviKey->setText(z,"");           // Dont try to wrap this string into i18n  -- stefan
				break;
			case 1:
				lviKey->setPixmap(z,pixTrust1);
				break;
			case 2:
				lviKey->setPixmap(z,pixTrust2);
				break;
			case 3:
				lviKey->setPixmap(z,pixTrust3);
				break;
			case 4:
				lviKey->setPixmap(z,pixTrust4);
				break;
			case 0:
			default:
				lviKey->setPixmap(z,pixTrust0);
				break;
		}
		z++;
	}
	if (showValidity) {
		switch(validity) {
			case 1:
				lviKey->setPixmap(z,pixValid1);
				break;
			case 2:
				lviKey->setPixmap(z,pixValid2);
				break;
			case 3:
				lviKey->setPixmap(z,pixValid3);
				break;
			case 4:
				lviKey->setPixmap(z,pixValid4);
				break;
			case 0:
			default:
				lviKey->setPixmap(z,pixValid0);
				break;
		}
		z++;
	}
	return lviKey;
}

QString kPGPKeyList::getSelectedKeyID()
{
	// This function returns the KeyID of the selected item...
	// we can only have one selection - do it easy!
	QListViewItem *tmpItem = NULL;

	tmpItem = currentItem();
	if (!tmpItem) return QString::null;

	QString a = tmpItem->text(getKeyIDColumn());
	if (a.length() < 3) return QString::null;

	return a;
}

QStringList kPGPKeyList::getSelectedList()
{
  // This function returns a string list of keyIDs selected
  // Only have to use it with multi-select mode, and if you can support
  // that many selections. Of course, you can use:
  // getSelectedList().item(0)
  QStringList ret;
  int keyIDCol = getKeyIDColumn();

  if ( isMultiSelection() ) {
    for ( QListViewItemIterator it( this ) ; it.current() ; ++it )
      if ( it.current()->isSelected() )
	ret << it.current()->text( keyIDCol );
  } else {
    QListViewItem * item = currentItem();
    if ( item )
      ret << item->text( keyIDCol );
  }
  return ret;
}

void kPGPKeyList::mnuKeys_selected(int menuID)
{
	/* Note: for now, the menu will only act on the one key you
	 * select. Eventually, it may support multiple keys.
	 * This was done because we have some issues with Qt's TreeView
	 * widget, and the fact that selected items can be hidden
	 * by closing the parent.... -- cdw 
	 */
	switch (menuID) {
		case ID_CLOSE:
			close();
			break;
#if 0
		case ID_IMPORTKEY:
			importKey(getSelectedKeyID());
			break;
		case ID_GENKEY:
			genKeyPair();
			break;
		case ID_RELOADKEYRING:
			if (!currentProfile) break;
			setEnabled(false);
			currentProfile->loadKeyring(); // to load into profile's memory
			setEnabled(true);
			loadKeyring(); // to show on screen...
			break;
		case ID_EDITKEY:
			editKey(getSelectedKeyID());
			break;
		case ID_REMOVEKEY:
			removeKey(getSelectedKeyID());
			break;
		case ID_REVOKEKEY:
			removeRevokeKey(getSelectedKeyID());
			break;
		case ID_ADDNAME:
			addName(getSelectedKeyID());
			break;
		case ID_SIGNKEY:
			signKey(getSelectedKeyID());
			break;
		case ID_EXPORTKEY:
			exportKey(getSelectedKeyID());
			break;
		case ID_REVOKESIG:
			revokeSignature(getSelectedKeyID());
			break;
		case ID_KEYINFO:
			keyInfo(getSelectedKeyID());
			break;
		case ID_DISENABLEKEY:
			disEnableKey(getSelectedKeyID());
			break;
#endif
		default:
			KMessageBox::error(this,i18n("Sorry, but that function is not currently available. Please go to http://geheimnis.sourceforge.net and submit a bug report."));
			break;
	}
}

void kPGPKeyList::slotRightClick(QListViewItem *lvi, const QPoint &pos, int column) {
	// first, change the selection so that we selected the thing we just
	// clicked on...
	setSelected(lvi,true);

	if (column); // we don't use this param, don't have a choice about it
	// so I don't want any warnings...

	// now, are we on a secret key pair, or a public key, or what?
	if (lvi) {
		QString tmp;
		tmp = lvi->text(4);
		if (tmp.contains(i18n("Subkey")) > 0) lvi = lvi->parent();
		if (tmp.contains(i18n("Display Name")) > 0) lvi = lvi->parent();

		tmp = lvi->text(4);
		if (tmp.contains(i18n("Key Pair")) > 0) {
			mnuSKey->exec(pos);
		} else if (tmp.contains(i18n("Public Key")) > 0) {
			mnuPKey->exec(pos);
		} else if (tmp.contains(i18n("Signature")) > 0) {
			mnuSigs->exec(pos);
		} else mnuKeys->exec(pos);
	} else {
		mnuKeys->exec(pos);
	}
}

int kPGPKeyList::getKeyIDColumn()
{
	int z = 0;
	if (!showKeyID) return -1; // FAIL!!!! EVIL BAD NOT GOOD!!

	if (showName) z++;
	if (showEmail) z++;
	if (showSize) z++;
// We don't need to show the rest, I just coded it for completeness .. cdw
//	if (showKeyID) z++;
//	if (showType) z++;
//	if (showTrust) z++;
//	if (showValidity) z++;
// This is designed to mirror the addItem thing above -
// if you change the order, change it here too...	
	return z;
}

void kPGPKeyList::setProfile(Profile *newP)
{
	currentProfile = newP;
}

void kPGPKeyList::dropEvent(QDropEvent *e) {
	// I **HATE** this code. --cdw 
	QStrList strings;
	if ( QUriDrag::decode( e, strings ) ) {
	        QStringList files;
 		if ( QUriDrag::decodeLocalFiles( e, files ) ) {
		 	for ( QStringList::Iterator it = files.begin();
			      it != files.end(); ++it ) {
				KeyImportDialog * p = new KeyImportDialog( currentProfile, 0, QString::null, this );
				p->setFile( *it );
			        p->show();
 	 	 	}
 	 	}
 	}
}

