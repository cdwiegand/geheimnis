// -*- c++ -*-

#ifndef KPGPKEYLIST_H
#define KPGPKEYLIST_H

// FIXME: move to defines.h
#define ID_CLOSE 500
#define ID_GENKEY 501
#define ID_EDITKEY 502
#define ID_REMOVEKEY 503
#define ID_REVOKEKEY 504
#define ID_ADDNAME 505
#define ID_SIGNKEY 506
#define ID_REVOKESIG 507
#define ID_RELOADKEYRING 508
#define ID_IMPORTKEY 509
#define ID_EXPORTKEY 510
#define ID_KEYINFO 511
#define ID_DISENABLEKEY 512
#define ID_GETKEYSIGS 514

const int KPGPKEYS_HIDENONE = 0;
const int KPGPKEYS_HIDENAME = 1;
const int KPGPKEYS_HIDEEMAIL = 2;
const int KPGPKEYS_HIDESIZE = 4;
const int KPGPKEYS_HIDEKEYID = 8;
const int KPGPKEYS_HIDETYPE = 16;
const int KPGPKEYS_HIDETRUST = 32;
const int KPGPKEYS_HIDEVALIDITY = 64;


#include <klistview.h>
#include <qpixmap.h>

class kPGPKeyProps;
class QPopupMenu;
class Profile;
class QString;
class QStringList;
class QPoint;
class QDropEvent;
class QVBoxLayout;
class QFrame;

class kPGPKeyList : public KListView {
		Q_OBJECT
		
	public:
		kPGPKeyList(QWidget *parent=0, int colOptions=0);
		void loadKeyring();
		QPopupMenu *getMenu();
		void setProfile(Profile *newProfile);

		void setFlatView(bool value=FALSE);
		void setShowOnlySecretKeys(bool value=FALSE);
		void setShowNamesOnly(bool value=FALSE);
		void setShowKeysOnly(bool value=FALSE);
		void setAllowMultipleKeys(bool value=FALSE);
		void setShowOnlyGoodKeys(bool value=FALSE);

		QString getSelectedKeyID();
		QStringList getSelectedList();
		
	public slots:
		void mnuKeys_selected(int);
		void slotRightClick(QListViewItem *lvi, const QPoint &pos, int column);
		void dropEvent(QDropEvent *e);

	public:
		// Child widgets
		QPopupMenu *mnuSKey;
		QPopupMenu *mnuSigs;
		QPopupMenu *mnuPKey;
		QPopupMenu *mnuKeys;
		QFrame *lblTop;
		Profile *currentProfile;

		bool flatView;						// By default, all of these are OFF / FALSE.
		bool showOnlySecretKeys;
		bool showNamesOnly;
		bool showKeysOnly;
		bool allowMultipleKeys;		// Call loadKeyring() if you change them! -- cdw
		bool showName;						// By default, all of these are ON / TRUE
		bool showEmail;
		bool showSize;
		bool showKeyID;
		bool showType;
		bool showTrust;
		bool showValidity;
		bool showOnlyGoodKeys;
		
		// Private functions...

private:
		int getKeyIDColumn();
		QListViewItem *addLine(QListViewItem *lvParent, const QString & displayName, const QString & emailAddress, const QString & keySize, const QString & keyID, const QString & typeText, const int trust, const int validity);

		// FIXME KDNDDropZone *myDNDZone;

		// Private pixmaps
		QPixmap pixKeySec;   // trusted key pair
		QPixmap pixKeyPub;   // untrusted key
		QPixmap pixKeyPub2;  // trusted key
		QPixmap pixKeyExp;   // expired key
		QPixmap pixKeyRev;   // revoked key
		QPixmap pixKeyDis;   // disabled key
		QPixmap pixNameSec;  // name on key pair
		QPixmap pixNamePub;  // untrusted name
		QPixmap pixNamePub2; // trusted name
		QPixmap pixNameRev;  // revoked name
		QPixmap pixSigSec;   // signature from trusted key pair
		QPixmap pixSigPub;   // untrusted signature
		QPixmap pixSigPub2;  // trusted signature
		
		QPixmap pixTrust0;
		QPixmap pixTrust1;
		QPixmap pixTrust2;
		QPixmap pixTrust3;
		QPixmap pixTrust4;
		
		QPixmap pixValid0;
		QPixmap pixValid1;
		QPixmap pixValid2;
		QPixmap pixValid3;
		QPixmap pixValid4;
};

#endif  // KPGPKEYLIST_H



