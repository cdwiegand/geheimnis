
#include "kPGPProfileEdit_PGP5.h"

#include <kconfig.h>
#include <klocale.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kurlrequester.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qgroupbox.h>

#include "kPGPProfileEdit_PGP5.moc"

PGP5ProfileEdit::PGP5ProfileEdit( QWidget * parent, const char * name )
  : ProfileEditBase( parent, name )
{
  QGridLayout * glay;
  QGroupBox   * group;
  QWidget     * ingroup;
  QVBoxLayout * vlay = new QVBoxLayout( this, 0, KDialog::spacingHint() );

  // "Commands" group (see gpg widget for explanation og the tricks used):
  group = new QGroupBox( 1, Qt::Horizontal, i18n("Commands"), this );
  ingroup = new QWidget( group );
  glay = new QGridLayout( ingroup, 2, 4 /* rows x cols */,
			  0, KDialog::spacingHint() );
  glay->setColStretch( 1, 1 );

  // pgpk:
  txtPGPBinK = new KURLRequester( ingroup );
  glay->addWidget( new QLabel( txtPGPBinK, i18n("pgp&k:"), ingroup ), 0, 0 );
  glay->addWidget( txtPGPBinK, 0, 1 );
  // pgpe:
  txtPGPBinE = new KURLRequester( ingroup );
  glay->addWidget( new QLabel( txtPGPBinE, i18n("p&gpe:"), ingroup ), 1, 0 );
  glay->addWidget( txtPGPBinE, 1, 1 );
  // pgps:
  txtPGPBinS = new KURLRequester( ingroup );
  glay->addWidget( new QLabel( txtPGPBinS, i18n("pgp&s:"), ingroup ), 2, 0 );
  glay->addWidget( txtPGPBinS, 2, 1 );
  // pgpk:
  txtPGPBinV = new KURLRequester( ingroup );
  glay->addWidget( new QLabel( txtPGPBinV, i18n("pgp&v:"), ingroup ), 3, 0 );
  glay->addWidget( txtPGPBinV, 3, 1 );

  vlay->addWidget( group );

  // "Keyrings" group:
  group = new QGroupBox( 1, Qt::Horizontal, i18n("Keyrings"), this );
  ingroup = new QWidget( group );
  glay = new QGridLayout( ingroup, 2, 2 /* rows x cols */,
			  0, KDialog::spacingHint() );
  glay->setColStretch( 1, 1 );

  // public:
  txtPublicKR = new KURLRequester( ingroup );
  glay->addWidget( new QLabel( txtPublicKR, i18n("P&ublic keyring:"), ingroup ), 0, 0 );
  glay->addWidget( txtPublicKR, 0, 1 );
  // secret:
  txtSecretKR = new KURLRequester( ingroup );
  glay->addWidget( new QLabel( txtSecretKR, i18n("Secret ke&yring:"), ingroup ), 1, 0 );
  glay->addWidget( txtSecretKR, 1, 1 );

  vlay->addWidget( group );
  vlay->addStretch( 1 );
}

void PGP5ProfileEdit::clear()
{
  txtPGPBinK->clear();
  txtPGPBinE->clear();
  txtPGPBinS->clear();
  txtPGPBinV->clear();
  txtPublicKR->clear();
  txtSecretKR->clear();
}

void PGP5ProfileEdit::reallyLoadProfile( KConfig * cfg )
{
  txtPGPBinK->setURL( cfg->readEntry( "binPGP5K", "pgpk") );
  txtPGPBinE->setURL( cfg->readEntry( "binPGP5E", "pgpe") );
  txtPGPBinS->setURL( cfg->readEntry( "binPGP5S", "pgps") );
  txtPGPBinV->setURL( cfg->readEntry( "binPGP5V", "pgpv") );
  txtPublicKR->setURL( cfg->readEntry( "PublicKeyring", "~/.pgp/pubring.pkr") );
  txtSecretKR->setURL( cfg->readEntry( "SecretKeyring", "~/.pgp/secring.skr") );
}

void PGP5ProfileEdit::reallySaveProfile( KConfig * cfg ) const
{
  cfg->writeEntry( "binPGP5K", txtPGPBinK->url() );
  cfg->writeEntry( "binPGP5E", txtPGPBinE->url() );
  cfg->writeEntry( "binPGP5S", txtPGPBinS->url() );
  cfg->writeEntry( "binPGP5V", txtPGPBinV->url() );
  cfg->writeEntry( "PublicKeyring", txtPublicKR->url() );
  cfg->writeEntry( "SecretKeyring", txtSecretKR->url() );
}

