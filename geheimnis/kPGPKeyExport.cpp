
#include "kPGPKeyExport.h"

#include "myProfile.h"
#include "misc.h"
#include "gdebug.h"

#include <kurlrequester.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <qbuttongroup.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qcombobox.h>

#include <assert.h>

#include "kPGPKeyExport.moc"

kPGPKeyExport::kPGPKeyExport( Profile * profile,
			      const QString & keyID,
			      QWidget * parent, const char * name )
  : KDialogBase( Plain, i18n("Key Export"), Help|Ok|Cancel, Ok,
		 parent, name ),
  mProfile( profile ), mKeyID( keyID )
{
  assert( profile );
  gDebug() << "kPGPKeyExport::kPGPKeyExport: mKeyID = \"" << mKeyID << "\""
	   << endl;
  //assert( keyID.length() == 16 );

  setButtonOKText( i18n("&Export"), QString::null /*no tooltip*/,
		   i18n("Click here to export the key with\nID %1 "
			"to the destination specified above.").arg(keyID) );

  QVBoxLayout * vlay = new QVBoxLayout( plainPage(), 0, spacingHint() );

  vlay->addWidget( new QLabel( i18n("Exporting the key with ID %1."),
			       plainPage() ) );

  mButtonGroup = new QButtonGroup( 3, Qt::Vertical, i18n("Destination"),
				   plainPage() );
  mButtonGroup->layout()->setSpacing( spacingHint() );

  // group column 0: radio buttons
  mButtonGroup->insert( new QRadioButton( i18n("&File"),
					  mButtonGroup ), File );
  mButtonGroup->insert( new QRadioButton( i18n("&Keyserver"),
					  mButtonGroup ), Keyserver );
  mButtonGroup->insert( new QRadioButton( i18n("Clip&board"),
					  mButtonGroup ), Clipboard );

  // group column 1: associated controls (file requester, combobox):
  // for File: URLRequester:
  mFileRequester = new KURLRequester( mButtonGroup );
  mFileRequester->setEnabled( false ); // since File radio isn't checked

  // disable url requester when radio is nor checked:
  connect( mButtonGroup->find(File), SIGNAL(toggled(bool)),
	   mFileRequester, SLOT(setEnabled(bool)) );

  // for Keyserver: ComboBox:
  mKeyserverCombo = new QComboBox( true, mButtonGroup );
  mKeyserverCombo->insertStringList( getKeyserverList() );

  // disable combobox when radio is not checked:
  connect( mButtonGroup->find(Keyserver), SIGNAL(toggled(bool)),
	   mKeyserverCombo, SLOT(setEnabled(bool)) );

  vlay->addWidget( mButtonGroup );
  vlay->addStretch( 1 );

  mButtonGroup->setButton( File ); // export to file is default...
  if ( !profile->keyServerCompatible() ) {
    mButtonGroup->find( Keyserver )->setEnabled( false );
    mKeyserverCombo->setEditText( i18n("Current backend has no keyserver support") );
    mKeyserverCombo->resize( mKeyserverCombo->sizeHint() );
  }
}

void kPGPKeyExport::slotOk()
{
  assert( mProfile->getPGPVersion() != 0 );

  switch ( mButtonGroup->id( mButtonGroup->selected() ) ) {
  case Keyserver:
    mProfile->exportKey_Server( mKeyID );
    break;
  case Clipboard:
    mProfile->exportKey_Clipboard( mKeyID );
    break;
  default:
  case File:
    if ( mFileRequester->url().isEmpty() ) {
      KMessageBox::sorry(this,i18n("Please choose a file to export to."));
      return;
    }
    // Okay, let's do it!
    mProfile->exportKey_File( mKeyID, mFileRequester->url() );
  }
  KDialogBase::slotOk();
}

