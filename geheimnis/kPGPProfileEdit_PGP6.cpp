
#include "kPGPProfileEdit_PGP6.h"

#include <kconfig.h>
#include <klocale.h>
#include <kdialog.h>
#include <kurlrequester.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qgroupbox.h>

#include "kPGPProfileEdit_PGP6.moc"

PGP6ProfileEdit::PGP6ProfileEdit( QWidget * parent, const char * name )
  : ProfileEditBase( parent, name )
{
  QVBoxLayout * vlay = new QVBoxLayout( this, 0, KDialog::spacingHint() );

  // "PGP Command" requster and label:
  QHBoxLayout * hlay = new QHBoxLayout( vlay ); // inherits spacing
  txtPGPBin = new KURLRequester( this );
  hlay->addWidget( new QLabel( txtPGPBin, i18n("P&GP command:"), this ) );
  hlay->addWidget( txtPGPBin );

  // "Keyrings" group (see gpg widget for the tricks we use here):
  QGroupBox * group = new QGroupBox( 1, Qt::Horizontal, i18n("Keyrings"), this );
  QWidget * ingroup = new QWidget( group );
  QGridLayout * glay = new QGridLayout( ingroup, 2, 2 /* rows x cols */,
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
  // FIXME: It shouldn't crash!!!
  QString msg = i18n("<qt>Geheimnis can handle PGP 6.5 only when the latter "
		     "runs in <em>native mode</em>. Therefore, please check "
		     "that there is no line like <tt>Compatible=on</tt> in "
		     "your <tt>pgp.cfg</tt> or Geheimnis will crash when "
		     "calling PGP.</qt>");
  vlay->addWidget( new QLabel( msg, this ) );
  vlay->addStretch( 1 );
}

void PGP6ProfileEdit::clear()
{
  txtPGPBin->clear();
  txtPublicKR->clear();
  txtSecretKR->clear();
}

void PGP6ProfileEdit::reallyLoadProfile( KConfig * cfg )
{
  txtPGPBin->setURL( cfg->readEntry( "binPGP6", "pgp" ) );
  txtPublicKR->setURL( cfg->readEntry( "PublicKeyring", "~/.pgp/pubring.pkr" ) );
  txtSecretKR->setURL( cfg->readEntry( "SecretKeyring", "~/.pgp/secring.skr" ) );
}

void PGP6ProfileEdit::reallySaveProfile( KConfig * cfg ) const
{
  cfg->writeEntry( "binPGP6", txtPGPBin->url() );
  cfg->writeEntry( "PublicKeyring", txtPublicKR->url() );
  cfg->writeEntry( "SecretKeyring", txtSecretKR->url() );
}

