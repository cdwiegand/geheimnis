
#include "kPGPKeyAdd.h"

#include "misc.h"
#include "myProfile.h"
#include "kPGPKeyList.h"

#include <klocale.h>
#include <kurlrequester.h>
#include <kmessagebox.h>

#include <qbuttongroup.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlabel.h>

#include <assert.h>

#include "kPGPKeyAdd.moc"

KeyImportDialog::KeyImportDialog( Profile * profile,
				  kPGPKeyList * passedKPGPKeyList,
				  const QString & keyID,
				  QWidget * parent, const char * name )
  : KDialogBase( Plain, i18n("Key Import"), Help|Ok|Cancel, Ok, parent, name ),
    mProfile( profile ), mKeyWindow( passedKPGPKeyList )
{
  assert( profile );
  mainFun();
  if ( !keyID.isEmpty() )
    setKeyID( keyID );
}

void KeyImportDialog::setKeyID( const QString & query )
{
  if ( !mProfile->keyServerCompatible() ) return;
  mKeyserverQuery->setText( query );
  mButtonGroup->setButton( Keyserver );
}

void KeyImportDialog::setFile( const QString & fileName )
{
  mFileRequester->setURL( fileName );
  mButtonGroup->setButton( File );
}

void KeyImportDialog::setClipboard()
{
  mButtonGroup->setButton( Clipboard );
}

void KeyImportDialog::mainFun()
{
  // tmp. vars:
  QRadioButton * radio;
  QHBoxLayout  * hlay;
  QLabel       * label;

  mButtonGroup = new QButtonGroup( plainPage() );
  mButtonGroup->hide();

  QVBoxLayout * vlay = new QVBoxLayout( plainPage(), 0, spacingHint() );

  // row 0: label:
  vlay->addWidget( new QLabel( i18n("Select source for key import:"), plainPage() ) );
    
  // row 1: "file" radio and requester:
  hlay = new QHBoxLayout( vlay ); // inherits spacing
  radio = new QRadioButton( i18n("&File"), plainPage() );
  mButtonGroup->insert( radio, File );
  hlay->addWidget( radio );

  mFileRequester = new KURLRequester( plainPage() );
  mFileRequester->setEnabled( false );
  hlay->addWidget( mFileRequester, 1 );

  // disable file requester if radio not checked:
  connect( radio, SIGNAL(toggled(bool)),
	   mFileRequester, SLOT(setEnabled(bool)) );

  // row 2: "keyserver" radio, server list combobox and query lineedit:
  hlay = new QHBoxLayout( vlay ); // inherits spacing
  radio = new QRadioButton( i18n("From &keyserver" ), plainPage() );
  mButtonGroup->insert( radio, Keyserver );
  hlay->addWidget( radio );

  mKeyserverCombo = new QComboBox( true, plainPage() );
  mKeyserverCombo->insertStringList( getKeyserverList() );
  mKeyserverCombo->setEnabled( false );
  hlay->addWidget( mKeyserverCombo );

  mKeyserverQuery = new QLineEdit( plainPage() );
  mKeyserverQuery->setEnabled( false );
  label = new QLabel( mKeyserverQuery,
		      i18n("retrieve key with name or &ID"), plainPage() );
  hlay->addWidget( label );
  hlay->addWidget( mKeyserverQuery, 1 );

  if ( !mProfile->keyServerCompatible() ) {
    radio->setEnabled( false );
    label->setEnabled( false );
    mKeyserverCombo->setEnabled( false );
    mKeyserverQuery->setText( i18n("<current backend does not support keyservers>") );
    mKeyserverQuery->setEnabled( false );
  }

  // disable combobox and line edit if radio not checked:
  connect( radio, SIGNAL(toggled(bool)),
	   mKeyserverCombo, SLOT(setEnabled(bool)) );
  connect( radio, SIGNAL(toggled(bool)),
	   mKeyserverQuery, SLOT(setEnabled(bool)) );
  
  // row 3: "clipboard" radio
  radio = new QRadioButton( i18n("Clip&board"), plainPage() );
  mButtonGroup->insert( radio, Clipboard );
  vlay->addWidget( radio );
  vlay->addStretch( 1 );

  mButtonGroup->setButton( File );
}

void KeyImportDialog::slotOk()
{
  assert( mProfile->getPGPVersion() != 0 );

  bool okWeDidIt = false;

  switch ( mButtonGroup->id( mButtonGroup->selected() ) ) {
  default:
  case File:
    if ( mFileRequester->url().length() < 3) {
      KMessageBox::sorry( this, i18n("Please choose a file to import.") );
      return;
    }
    hide();
    okWeDidIt = (mProfile->importKey_File( mFileRequester->url() ) == 0);
    break;
  case Keyserver:
    if (mKeyserverQuery->text().length() < 3) {
      KMessageBox::sorry( this, i18n("Please enter a key ID or email "
				     "address to search for.") );
      return;
    }
    hide();
    okWeDidIt = (mProfile->importKey_Server( mKeyserverQuery->text() ) == 0);
    break;
  case Clipboard:
    hide();
    okWeDidIt = (mProfile->importKey_Clipboard() == 0);
    break;
  }

  if ( okWeDidIt ) {
    if ( mKeyWindow ) mKeyWindow->loadKeyring();
    KDialogBase::slotOk();
  } else {
    KMessageBox::error( this, i18n("Error importing key!") );
    show();
  }
}

