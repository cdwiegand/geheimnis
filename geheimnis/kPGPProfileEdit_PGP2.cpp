
#include "kPGPProfileEdit_PGP2.h"

//#include "defines.h"


#include <kconfig.h>
#include <klocale.h>
#include <kdialog.h>
//#include <kmessagebox.h>
#include <kurlrequester.h>

#include <qlayout.h>
#include <qlabel.h>

#include "kPGPProfileEdit_PGP2.moc"

PGP2ProfileEdit::PGP2ProfileEdit( QWidget * parent, const char * name )
  : ProfileEditBase( parent, name )
{
  QGridLayout * glay = new QGridLayout( this, 2, 2 /* rows x cols */,
					0, KDialog::spacingHint() );
  glay->setRowStretch( 1, 1 );
  glay->setColStretch( 1, 1 );

  txtPGPBin = new KURLRequester( this );
  glay->addWidget( txtPGPBin, 0, 1 );
  glay->addWidget( new QLabel( txtPGPBin, i18n("P&GP command:"), this ), 0, 0 );
}

void PGP2ProfileEdit::clear()
{
  txtPGPBin->clear();
}

void PGP2ProfileEdit::reallyLoadProfile( KConfig * cfg )
{
  txtPGPBin->setURL( cfg->readEntry( "binPGP2", "pgp" ) );
}

void PGP2ProfileEdit::reallySaveProfile( KConfig * cfg ) const
{
  cfg->writeEntry( "binPGP2", txtPGPBin->url() );
}

