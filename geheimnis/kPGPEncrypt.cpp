
#include "kPGPEncrypt.h"

#include "misc.h"
#include "myProfile.h"
#include "profilemanager.h"
#include "myClipboard.h"
#include "kPGPKeyList.h"
#include "myKeyring.h"
#include "mySession.h"
#include "gdebug.h"

#include <kmessagebox.h>
#include <klocale.h>
#include <kurlrequester.h>
#if QT_VERSION < 300
#  include <kpropsdlg.h>
#else
#  include <kpropertiesdialog.h>
#endif

#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qbuttongroup.h>

#include <assert.h>

#include "kPGPEncrypt.moc"

kPGPEncrypt::kPGPEncrypt( const QString & infile, const QString & outfile,
			  QWidget * parent, const char * name )
  : CryptDialogBase( i18n("Encrypt / Sign"), parent, name ),
    mCurrentProfile(0)
{
  QButtonGroup * group;

  QGridLayout * glay = new QGridLayout( plainPage(), 8, 2, // rows x cols
					0, spacingHint() );
  glay->setColStretch( 1, 1 ); // non-labels
  glay->setRowStretch( 4, 1 ); // listview

  // row 0: "Profile" combobox and label:
  mProfileCombo = new QComboBox( false, plainPage() );
  mProfileCombo->insertStringList( profileManager.profileNameList() );
  glay->addWidget( new QLabel( mProfileCombo, i18n("&Profile:"), plainPage() ),
		   0, 0 );
  glay->addWidget( mProfileCombo, 0, 1 );

  // recognize when the current profile has changed:
  connect ( mProfileCombo, SIGNAL(activated(int)),
	    this, SLOT(slotProfileSelected(int)) );

  // row 1: "Input" group:
  group = createClipOrFileGroup( i18n("Input"),
				 i18n("&Use content of clipboard"),
				 i18n("&File"), mInClipboardRadio,
				 mInFileRequester, infile, plainPage() );
  glay->addMultiCellWidget( group, 1, 1, 0, 1 );

  // row 2: "Output" group:
  QString realOutFile = ( outfile.isEmpty() ) ? infile + ".pgp" : outfile ;
  group = createClipOrFileGroup( i18n("Output"),
				 i18n("&Write to clipboard"),
				 i18n("F&ile"), mOutClipboardRadio,
				 mOutFileRequester, realOutFile, plainPage() );
  glay->addMultiCellWidget( group, 2, 2, 0, 1 );

  // row 3: "Sign as" checkbox and secKey combobox:
  mSignCheck = new QCheckBox( i18n("&Sign as:"), plainPage() );
  mSignCheck->setChecked( true );
  mSignCombo = new QComboBox( false, plainPage() );
  glay->addWidget( mSignCheck, 3, 0 );
  glay->addWidget( mSignCombo, 3, 1 );

  // disable combo when not checked:
  connect( mSignCheck, SIGNAL(toggled(bool)),
	   mSignCombo, SLOT(setEnabled(bool)) );

  // row 4: "Encrypt to" checkbox and pubKey listview
  mEncryptCheck = new QCheckBox( i18n("&Encrypt to:"), plainPage() );
  glay->addWidget( mEncryptCheck, 4, 0, Qt::AlignTop|Qt::AlignLeft );

  mEncryptToList = new kPGPKeyList( plainPage(),
				    KPGPKEYS_HIDETYPE + KPGPKEYS_HIDETRUST );
  mEncryptToList->setShowNamesOnly( true );
  mEncryptToList->setAllowMultipleKeys( true );
  mEncryptToList->setFlatView( true );
  mEncryptToList->setShowOnlyGoodKeys( true );
  mEncryptToList->setEnabled( false );

  glay->addWidget( mEncryptToList, 4, 1 );

  // disable list when not checked:
  connect( mEncryptCheck, SIGNAL(toggled(bool)),
	   mEncryptToList, SLOT(setEnabled(bool)) );
  
  // row 5: "Signature options" group:
  group = new QButtonGroup( 3, Qt::Vertical, i18n("Options"),
			    plainPage() );
  group->layout()->setSpacing( spacingHint() );
  mSignDetachedRadio = new QRadioButton( i18n("&Detached signature"), group );
  mSignClearRadio = new QRadioButton( i18n("C&learsign"), group );
  mSignTextmodeCheck = new QCheckBox( i18n("Universal &text"), group );
  mEncryptToSelfCheck = new QCheckBox( i18n("Encrypt to self"), group );
  mAsciiArmorCheck = new QCheckBox( i18n("Use ASCII armor"), group );
  mSignTextmodeCheck->setChecked( true );
  mSignClearRadio->setChecked( true );
  mAsciiArmorCheck->setChecked( true );
  mEncryptToSelfCheck->setChecked( true );
  mEncryptToSelfCheck->setEnabled( false );

  glay->addMultiCellWidget( group, 5, 5, 0, 1 );

  // plug-fest:
  // 1. disable textmode when not signing:
  connect( mSignCheck, SIGNAL(toggled(bool)),
	   mSignTextmodeCheck, SLOT(setEnabled(bool)) );
  // 2. disable encrypt-to-self when not encrypting:
  connect( mEncryptCheck, SIGNAL(toggled(bool)),
	   mEncryptToSelfCheck, SLOT(setEnabled(bool)) );
  // 3. more complex logic for clearsigning vs. detached sig:
  connect( mSignCheck, SIGNAL(toggled(bool)),
	   SLOT(slotCheckDetachedSigPossible()) );
  connect( mEncryptCheck, SIGNAL(toggled(bool)),
	   SLOT(slotCheckDetachedSigPossible()) );
  // 4. enable Ok only when either signing or encrypting:
  connect( mSignCheck, SIGNAL(toggled(bool)),
	   SLOT(slotEnableOkButton()) );
  connect( mEncryptCheck, SIGNAL(toggled(bool)),
	   SLOT(slotEnableOkButton()) );

  switch ( mProfileCombo->count() ) {
  case 0:
    // No profiles! Prevent this from happening _before_ creating this
    // dialog!
    gFatal() << "kPGPEncrypt instantated without profiles available!" << endl;
    break; // make compiler happy
  case 1:
    slotProfileSelected(0);
    break;
  default:
    profileManager.setToDefaultProfile( mProfileCombo );
    if ( mProfileCombo->currentItem() > 0 )
      slotProfileSelected( mProfileCombo->currentItem() );
    break;
  }
}

kPGPEncrypt::~kPGPEncrypt() {
}

void kPGPEncrypt::slotEnableOkButton() {
  enableButtonOK( mSignCheck->isChecked() || mEncryptCheck->isChecked() );
}

void kPGPEncrypt::slotCheckDetachedSigPossible() {
  bool signing = mSignCheck->isChecked();
  bool encrypting = mEncryptCheck->isChecked();

  bool enable = ( signing && !encrypting );

  mSignDetachedRadio->setEnabled( enable );
  mSignClearRadio->setEnabled( enable );

  if ( encrypting ) // have to use clearsigning when encrypting:
    mSignClearRadio->setChecked( true );
}

bool kPGPEncrypt::sanityCheckOutFile( bool & clipboard, QRadioButton *,
				      KURLRequester * )
{
  // clipboard is always OK:
  if ( clipboard ) return true;

  if ( mOutFileRequester->url().isEmpty() ) {
    QString suggestedFileName = mInFileRequester->url();
    if ( mSignCheck->isChecked() && !mEncryptCheck->isChecked() )
      if ( mSignDetachedRadio->isChecked() )
	if ( mAsciiArmorCheck->isChecked() )
	  suggestedFileName += ".sig.asc";  // armored detached sig
	else
	  suggestedFileName += ".sig";      // non-armor detached sig
      else
	suggestedFileName += ".pgp";        // clearsign
    else
      if ( mAsciiArmorCheck->isChecked() )
	suggestedFileName += ".asc";        // armored encrypted
      else
	suggestedFileName += ".pgp";        // non-armor encrypted

    QString msg = i18n("You chose to write output to a file,\n"
		       "but you didn't give a filename.\n"
		       "\n"
		       "Use \"%1\" as output filename,\n"
		       "use clipboard or neither?")
      .arg( suggestedFileName );
#if QT_VERSION < 300
    switch ( KMessageBox::warningYesNoCancel( this, msg, QString::null,
#else
    switch ( KMessageBox::questionYesNoCancel( this, msg, QString::null,
#endif
					       i18n("Use &suggested name"),
					       i18n("&Use clipboard") ) ) {
    default:
    case KMessageBox::Yes:
      mOutFileRequester->setURL( suggestedFileName );
      return true;
    case KMessageBox::No:
      clipboard = true;
      mOutClipboardRadio->setChecked( true );
      return true;
    case KMessageBox::Cancel:
      mOutFileRequester->setFocus();
      return false;
    }
  }

  return CryptDialogBase::sanityCheckOutFile( clipboard, mOutClipboardRadio,
					      mOutFileRequester );
}

void kPGPEncrypt::slotOk()
{
  bool usingIncomingClipboard = mInClipboardRadio->isChecked();
  bool usingOutgoingClipboard = mOutClipboardRadio->isChecked();;
  bool signing = mSignCheck->isChecked();
  bool encrypting = mEncryptCheck->isChecked();

  // the GUI should prevent this from being triggered:
  assert( signing || encrypting );
  // these shouldn't happen, too:
  assert( mCurrentProfile );
  assert( mProfileCombo->currentItem() >= 0 );

  if ( !sanityCheckInFile( usingIncomingClipboard, mInClipboardRadio,
			   mInFileRequester ) ) return;
  if ( !sanityCheckOutFile( usingOutgoingClipboard ) ) return;

  // set up input file:
  QString inFileName;
  if ( usingIncomingClipboard ) {
    inFileName = getSafeTmpNameQ() + ".1.asc";
    if ( !myClipboard::saveFromClipboard( inFileName ) ) {
      KMessageBox::sorry(this,i18n("Could not retrieve anything from clipboard."));
      return;
    }
  } else
    inFileName = mInFileRequester->url();

  // set up output file:
  QString outFileName;
  if ( usingOutgoingClipboard )
    outFileName = getSafeTmpNameQ() + ".2.asc";
  else
    outFileName = mOutFileRequester->url();

  // set up recipients:
  QString recipients;
  if ( mEncryptCheck->isChecked() ) {
    QStringList l = mEncryptToList->getSelectedList();
    for ( QStringList::Iterator it = l.begin() ; it != l.end() ; ++it )
      recipients += mCurrentProfile->formatRecipIDForEncryption( *it );
    if ( mEncryptToSelfCheck->isChecked() )
      recipients += mCurrentProfile->formatRecipIDForEncryption( *qlistSecKeyIDs.at(mSignCombo->currentItem()) );
  }

  // run backend: ### FIXME: check return value!
  mCurrentProfile->encryptFile( inFileName, outFileName,
				*qlistSecKeyIDs.at(mSignCombo->currentItem()),
				recipients, mSignDetachedRadio->isChecked(),
				signing, encrypting,
				mAsciiArmorCheck->isChecked(),
				mSignTextmodeCheck->isChecked(),
				mSignClearRadio->isChecked() );

  if ( usingIncomingClipboard ) QFile::remove( inFileName );

  if ( usingOutgoingClipboard ) {
    myClipboard::saveIntoClipboard( outFileName );
    QFile::remove( outFileName );
    KMessageBox::information( this, i18n("If decryption was successful,\n"
					 "please paste back the result "
					 "before hitting ok.") );
  }
  CryptDialogBase::slotOk();
}

void kPGPEncrypt::slotProfileSelected( int listIndex )
{
  qlistSecKeyIDs.clear();

  mCurrentProfile = profileManager.profileForName( mProfileCombo->text(listIndex) );
  mCurrentProfile->loadKeyring();

  mEncryptToList->setProfile( mCurrentProfile );
  mEncryptToList->loadKeyring();

  mSignCombo->clear();

  uint count = mCurrentProfile->keyring->count();
  for ( uint i = 0 ; i < count ; ++i ) {
    // Per KEY
    pkiKey * key = mCurrentProfile->keyring->at(i);
    if ( key && key->haveSecret && !key->isRevoked && !key->isExpired ) {
      qlistSecKeyIDs << key->getKeyID();
      if ( key->names.isEmpty() )
	mSignCombo->insertItem( i18n("???? (%2)").arg( key->getKeyType() ) );
      else
	mSignCombo->insertItem( i18n("%1 <%2> (%3)")
				.arg( key->names.first()->getDisplayName() )
				.arg( key->names.first()->getEmailAddress() )
				.arg( key->getKeyType() ) );
    }
  }
}

