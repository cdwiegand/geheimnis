
#include "kPGPProfileEdit_GPG.h"

#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurlrequester.h>
#include <kfiledialog.h>
#include <kdebug.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qgroupbox.h>

#include <assert.h>

#include "kPGPProfileEdit_GPG.moc"

GPGProfileEdit::GPGProfileEdit( QWidget * parent, const char * name )
  : ProfileEditBase( parent, name )
{
  QVBoxLayout * vlay = new QVBoxLayout( this, 0, KDialog::spacingHint() );
  QHBoxLayout * hlay = new QHBoxLayout( vlay ); // inherits spacing

  txtPGPBin = new KURLRequester( this );
  hlay->addWidget( new QLabel( txtPGPBin, i18n("&GnuPG command:"), this ) );
  hlay->addWidget( txtPGPBin, 1 );

  QGroupBox * group = new QGroupBox( 1, Qt::Horizontal, i18n("Keyrings"), this );
  vlay->addWidget( group );
  // We want to let Qt manage the layout in the groupbox, because it
  // does a much better job at it than we could. But it doesn't
  // provide support for multicell widgets, then. So we fake a single
  // widget instead:
  QWidget * ingroup = new QWidget( group );
  QGridLayout * glay = new QGridLayout( ingroup, 3, 2 /* rows x cols */,
					0, KDialog::spacingHint() );
  glay->setRowStretch( 2, 1 );
  glay->setColStretch( 0, 1 );

  // Now, show the listbox for the keyrings...
  lstKeyrings = new QListView( ingroup );
  lstKeyrings->addColumn( i18n("Path") );
  lstKeyrings->addColumn( i18n("Type") );
  lstKeyrings->setAllColumnsShowFocus( true );
  // there's got to be a better way...
  //lstKeyrings->setFixedHeight( lstKeyrings->fontMetrics().lineSpacing()*5 );
  glay->addMultiCellWidget( lstKeyrings, 0, 2, 0, 0 );

  kdDebug() << "lstKeyrings->sizeHint() == QSize( "
	    << lstKeyrings->sizeHint().width() << ", "
	    << lstKeyrings->sizeHint().height() << " )" << endl;
  kdDebug() << "lstKeyrings->minimumSizeHint() == QSize( "
	    << lstKeyrings->minimumSizeHint().width() << ", "
	    << lstKeyrings->minimumSizeHint().height() << " )" << endl;

  cmdNewKR = new QPushButton( i18n("&Add..."), ingroup );
  connect( cmdNewKR, SIGNAL(clicked()),
	   this, SLOT(cmdNewKR_clicked()) );
  glay->addWidget( cmdNewKR, 0, 1 );
	
  cmdDelKR = new QPushButton( i18n("Re&move"), ingroup );
  connect( cmdDelKR, SIGNAL(clicked()),
	   this, SLOT(cmdDelKR_clicked()) );
  glay->addWidget( cmdDelKR, 1, 1 );

  chkHonorHTTPProxy = new QCheckBox( i18n("Honor H&TTP Proxy" ), this );
  vlay->addWidget( chkHonorHTTPProxy );
  vlay->addStretch( 100 );
}

void GPGProfileEdit::reallyLoadProfile( KConfig * cfg )
{
  QStringList keyrings;

  txtPGPBin->setURL( cfg->readEntry( "binGPG", "gpg" ) );
  chkHonorHTTPProxy->setChecked( cfg->readBoolEntry( "HonorHTTPProxy", false ) );

  // clear the list
  lstKeyrings->clear();

  // insert public keyrings:
  keyrings = cfg->readListEntry( "PublicKeyring", ';' );
  if ( keyrings.isEmpty() )
    keyrings << "~/.gnupg/pubring.gpg";

  for ( QStringList::Iterator it = keyrings.begin() ;
	it != keyrings.end() ; ++it )
    (void)new QListViewItem( lstKeyrings, *it, i18n("Public") );

  // insert secret keyrings:
  keyrings = cfg->readListEntry( "SecretKeyring", ';' );
  if ( keyrings.isEmpty() )
    keyrings << "~/.gnupg/secring.gpg";

  for ( QStringList::Iterator it = keyrings.begin() ;
	it != keyrings.end() ; ++it )
    (void)new QListViewItem( lstKeyrings, *it, i18n("Secret") );
}

void GPGProfileEdit::reallySaveProfile( KConfig * cfg ) const
{
  cfg->writeEntry( "binGPG", txtPGPBin->url() );

  QStringList publicKeyrings, secretKeyrings;
  for ( QListViewItemIterator it( lstKeyrings ) ; it.current() ; ++it )
    if ( it.current()->text(1) == i18n("Public") )
      publicKeyrings << it.current()->text(0);
    else
      secretKeyrings << it.current()->text(0);
  cfg->writeEntry( "PublicKeyring", publicKeyrings );
  cfg->writeEntry( "SecretKeyring", secretKeyrings );
  
  cfg->writeEntry( "HonorHTTPProxy", chkHonorHTTPProxy->isChecked() );
}

void GPGProfileEdit::clear()
{
  chkHonorHTTPProxy->setChecked( false );
  lstKeyrings->clear();
  txtPGPBin->clear();
}

void GPGProfileEdit::cmdNewKR_clicked()
{
  QString f = KFileDialog::getOpenFileName( 0, 0, this, 0 );

  if ( !f.isEmpty() ) {
    if ( KMessageBox::questionYesNo( this,
				     i18n("Is this keyring public? (If you're "
					  "not sure, answer \"Yes\".)") )
	 == KMessageBox::Yes)
      (void)new QListViewItem( lstKeyrings, f, i18n("Public") );
    else
      (void)new QListViewItem( lstKeyrings, f, i18n("Secret") );
  }
}

void GPGProfileEdit::cmdDelKR_clicked()
{
  QListViewItem * item = lstKeyrings->currentItem();

  if ( !item ) return;

  delete item;
}

