
#include "kPGPDecrypt.h"

#include "myProfile.h"
#include "profilemanager.h"
#include "myClipboard.h"
#include "misc.h"
#include "gdebug.h"

#include <kmessagebox.h>
#include <kurlrequester.h>
#include <klocale.h>

#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qfile.h>
#include <qbuttongroup.h>

#include <assert.h>

#include "kPGPDecrypt.moc"


kPGPDecrypt::kPGPDecrypt( const QString & infile, const QString & outfile,
			  bool detachedSignature,
			  QWidget * parent, const char * name )
  : CryptDialogBase( i18n("Decrypt / Verify"), parent, name )
{
  // tmp. vars:
  QButtonGroup * group;

  QVBoxLayout * vlay = new QVBoxLayout( plainPage(), 0, spacingHint() );

  // row 0: "Profile" combobox and label:
  QHBoxLayout * hlay = new QHBoxLayout( vlay ); // inherits spacing
  mProfileCombo = new QComboBox( false, plainPage() );
  mProfileCombo->insertStringList( profileManager.profileNameList() );
  hlay->addWidget( new QLabel( mProfileCombo, i18n("&Profile:"), plainPage() ) );
  hlay->addWidget( mProfileCombo, 1 );

  // row 1: "Input" group:
  group = createClipOrFileGroup( i18n("Input"),
				 i18n("&Use content of clipboard"),
				 i18n("&File"),
				 mInClipboardRadio, mInFileRequester,
				 infile, plainPage() );
  vlay->addWidget( group );

  // row 2: "Decrypt" checkbox:
  mDecryptCheck = new QCheckBox( i18n("&Decrypt"), plainPage() );
  vlay->addWidget( mDecryptCheck );

  QString realOutFile;
  if ( outfile.isEmpty() ) {
#if QT_VERSION < 300
    if ( infile.right(4) == ".pgp"
	 || infile.right(4) == ".sig" || infile.right(4) == ".asc" )
#else
    if ( infile.endsWith(".pgp")
	 || infile.endsWith(".sig") || infile.endsWith(".asc") )
#endif
      realOutFile = infile.left( infile.length() - 4 );
  } else
    realOutFile = outfile;

  // row 3: "Output" group:
  group = createClipOrFileGroup( i18n("Output"),
				 i18n("&Write to clipboard"),
				 i18n("F&ile"),
				 mOutClipboardRadio, mOutFileRequester,
				 realOutFile, plainPage() );

  vlay->addWidget( group );

  // disable whole group if "decrypt" isn't checked:
  group->setEnabled( false );
  connect( mDecryptCheck, SIGNAL(toggled(bool)),
	   group, SLOT(setEnabled(bool)) );

  // row 4: "detached" check:
  mSignDetachedCheck = new QCheckBox( i18n("Detached &signature"), plainPage() );
  vlay->addWidget( mSignDetachedCheck );

  // row 5: "Signature" group:
  group = createClipOrFileGroup( i18n("Signature"),
				 i18n("O&btain from clipboard"),
				 i18n("Fi&le"),
				 mSigClipboardRadio, mSigFileRequester,
				 QString::null, plainPage() );
  vlay->addWidget( group );

  // disable whole group if "detached" isn't checked:
  mSignDetachedCheck->setChecked( detachedSignature );
  group->setEnabled( detachedSignature );
  connect( mSignDetachedCheck, SIGNAL(toggled(bool)),
	   group, SLOT(setEnabled(bool)) );

  // disable "detached" if "decrypt" is checked:
  connect( mDecryptCheck, SIGNAL(toggled(bool)),
	   SLOT(slotCheckDetachedSignatureAvailable()) );

  switch ( mProfileCombo->count() ) {
  case 0:
    gFatal() << "kPGPDecrypt instantated without profiles available!" << endl;
    break; // make compiler happy
  case 1:
    break;
  default:
    profileManager.setToDefaultProfile( mProfileCombo );
    break;
  }
}

void kPGPDecrypt::slotCheckDetachedSignatureAvailable()
{
  if ( mDecryptCheck->isChecked() ) {
    mSignDetachedCheck->setChecked( false );
    mSignDetachedCheck->setEnabled( false );
  } else
    mSignDetachedCheck->setEnabled( true );
}

void kPGPDecrypt::slotOk()
{
  bool decrypting = mDecryptCheck->isChecked();
  bool detachedSig = mSignDetachedCheck->isChecked();

  bool usingIncomingClipboard = mInClipboardRadio->isChecked();
  bool usingOutgoingClipboard = mOutClipboardRadio->isChecked();
  bool usingSignatureClipboard = mOutClipboardRadio->isChecked();

  // the GUI should prevent these from being triggered:
//  assert( mProfileCombo->currentItem() > 0 );
// The previous assest is wrong for QT2 - the first item IS zero..
  assert( !( decrypting && detachedSig ) );
  assert( !( usingIncomingClipboard && usingSignatureClipboard ) );

  Profile * profile = profileManager.profileForName( mProfileCombo->currentText() );

  // these shouldn't happen, too:
  assert( profile );
  assert( profile->getPGPVersion() > 0 );

  // sanity check filenames:
  if ( !sanityCheckInFile( usingIncomingClipboard, mInClipboardRadio,
			   mInFileRequester ) ) return;
  if ( decrypting && !sanityCheckOutFile( usingOutgoingClipboard,
					  mOutClipboardRadio,
					  mOutFileRequester ) ) return;
  else if ( !decrypting && detachedSig &&
	    !sanityCheckInFile( usingSignatureClipboard, mSigClipboardRadio,
				mSigFileRequester ) ) return;

  // set up input file:
  QString inFileName;
  if ( usingIncomingClipboard ) {
    inFileName = getSafeTmpNameQ() + ".1.asc";
    if ( !myClipboard::saveFromClipboard( inFileName ) ) {
      KMessageBox::sorry( this, i18n("Could not retrieve anything from clipboard.") );
      return;
    }
  } else
    inFileName = mInFileRequester->url();

  // set up output file (if any):
  QString outFileName;
  if ( decrypting )
    if ( usingOutgoingClipboard )
      outFileName = getSafeTmpNameQ() + ".2.asc";
    else
      outFileName = mOutFileRequester->url();

  // set up signature file (if any):
  QString sigFileName;
  if ( !decrypting && detachedSig )
    if ( usingSignatureClipboard )
      sigFileName = getSafeTmpNameQ() + ".3.asc";
    else
      sigFileName = mSigFileRequester->url();


  // run backend: ### FIXME: check return value!
  profile->decryptFile( inFileName, decrypting ? outFileName : sigFileName,
			detachedSig, !decrypting );
  
  if ( usingIncomingClipboard )
    QFile::remove( inFileName );
  if ( usingSignatureClipboard )
    QFile::remove( sigFileName );
  if ( usingOutgoingClipboard ) {
    myClipboard::saveIntoClipboard( outFileName );
    QFile::remove( outFileName );
    if ( decrypting ) {
      KMessageBox::information(this,i18n("If decryption was successful,\nplease paste back the result before hitting ok."));
    }
  }
  CryptDialogBase::slotOk();
}

