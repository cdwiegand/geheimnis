
#include "kPGPDNDZone.h"

#include "misc.h"
#include "kPGPEncrypt.h"
#include "kPGPDecrypt.h"
#include "kPGPPrefs.h"
#include "kPGPKeys.h"

#include <klocale.h>
#include <kaction.h>
#include <kstdaction.h>
#if QT_VERSION < 300
#include <kapp.h>
#else
#include <kapplication.h>
#endif

#include <qevent.h>
#include <qdragobject.h>
#include <qlabel.h>

#include "kPGPDNDZone.moc"

kPGPDNDZone::kPGPDNDZone( QWidget * parent, const char * name )
  : KMainWindow(parent, name)
{
  // Setting up the main widget:
  QLabel * lblMain = new QLabel(i18n(" < < Drag-n-drop files here > > "), this);
  setCentralWidget(lblMain);
  lblMain->setAlignment(AlignCenter);

  setAcceptDrops(true); // 2.0
  resize(450,125);

  // Setting up the actions:
  KAction * action;
  QString msg;

  // "encrypt/sign" action:
  action = new KAction( i18n("Encrypt/Sign"), "encrypted", CTRL+Key_E,
			this, SLOT(encrypt_clicked()),
			actionCollection(), "edit_encrypt_sign" );
  msg = i18n( "Encrypt and/or sign files" );
  action->setToolTip( msg );

  // "decrypt/verify" action:
  action = new KAction( i18n("Decrypt/Verify"), "decrypted", CTRL+Key_D,
			this, SLOT(decrypt_clicked()),
			actionCollection(), "edit_decrypt_verify" );
  msg = i18n( "Decrypt and/or verify files" );
  action->setToolTip( msg );

  // "key management" action:
  action = new KAction( i18n("Key Management"), "view_tree", CTRL+Key_K,
			this, SLOT(keys_clicked()),
			actionCollection(), "edit_keymanagement" );
  msg = i18n( "Manage your Web of Trust" );
  action->setToolTip( msg );

  // these are std actions:
  (void)KStdAction::preferences( this, SLOT(prefs_clicked()), actionCollection() );
  (void)KStdAction::close( this, SLOT(close()), actionCollection() );


  createGUI("geheimnisdndzoneui.rc");

  applyMainWindowSettings( kapp->config(), "Main Window - Drop Zone" );
}

bool kPGPDNDZone::queryClose() {
  saveMainWindowSettings( kapp->config(), "Main Window - Drop Zone" );
  return true;
}

void kPGPDNDZone::dragEnterEvent(QDragEnterEvent *qevent) {
	qevent->accept(QUriDrag::canDecode(qevent));
}

void kPGPDNDZone::dropEvent(QDropEvent *e) {
	QWidget *c;

	// I **HATE** this code. --cdw 
	QStrList strings;
	if ( QUriDrag::decode( e, strings ) ) {
	        QStringList files;
 		if ( QUriDrag::decodeLocalFiles( e, files ) ) {
		 	for ( QStringList::Iterator it = files.begin(); it != files.end() ; ++it ) {
				c = droppedFile( 0, *it );
				if ( c ) c->show();
 	 	 	}
 	 	}
 	}
}

void kPGPDNDZone::encrypt_clicked() {
  (new kPGPEncrypt())->show();
}

void kPGPDNDZone::decrypt_clicked() {
  (new kPGPDecrypt())->show();
}

void kPGPDNDZone::keys_clicked() {
  (new KeyEditorWindow( this ))->show();
}

void kPGPDNDZone::prefs_clicked() {
  (new kPGPPrefs( this ))->show();
}




