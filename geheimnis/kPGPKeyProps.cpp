
#include "kPGPKeyProps.h"

#include "myKeyring.h"
//#include "myProfile.h"
//#include "defines.h"

#include <klocale.h>

#include <qstring.h>
#include <qlabel.h>
#include <qlayout.h>

#include <assert.h>

#include "kPGPKeyProps.moc"

static const char * verboseValidity[] = {
  I18N_NOOP("Unknown."),
  I18N_NOOP("This key is explictly <b>not</b> valid."),
  I18N_NOOP("This key is somewhat valid."),
  I18N_NOOP("This key is valid."),
  I18N_NOOP("This key is implicitly valid.")
};

static const char * verboseTrust[] = {
  I18N_NOOP("Unknown."),
  I18N_NOOP("This key is explicitly <b>not</b> trusted."),
  I18N_NOOP("This key is somewhat trusted."),
  I18N_NOOP("This key is trusted."),
  I18N_NOOP("This key is implicitly trusted.")
};

static inline QLabel * makeSunkenLabel( const QString & text,
					QWidget * parent )
{
  QLabel * label = new QLabel( text, parent );
  label->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  return label;
}

KeyPropertiesDialog::KeyPropertiesDialog( const pkiKey * key,
					  QWidget * parent, const char * name )
  : KDialogBase( Plain, i18n("Key Properties"), Close, Close, parent, name )
{
  assert( key );
  
  QWidget * p = plainPage();
  QGridLayout * glay = new QGridLayout( p, 10, 2, 0, spacingHint() );
  glay->setRowStretch( 9, 1 );
  glay->setColStretch( 1, 1 );
  
  glay->addWidget( new QLabel( i18n("Key ID:"), p ), 0, 0 );
  glay->addWidget( makeSunkenLabel( key->getKeyID(), p ), 0, 1 );

  glay->addWidget( new QLabel( i18n("Size:"), p ), 1, 0 );
  glay->addWidget( makeSunkenLabel( key->getKeySize(), p ), 1, 1 );

  glay->addWidget( new QLabel( i18n("Type:"), p ), 2, 0 );
  glay->addWidget( makeSunkenLabel( key->getKeyType(), p ), 2, 1 );

  glay->addWidget( new QLabel( i18n("Fingerprint:"), p ), 3, 0 );
  glay->addWidget( makeSunkenLabel( key->getFingerprint(), p ), 3, 1 );

  glay->addWidget( new QLabel( i18n("Validity:"), p ), 4, 0 );
  glay->addWidget( makeSunkenLabel( i18n(verboseValidity[ key->getValidity() ]), p ), 4, 1 );

  glay->addWidget( new QLabel( i18n("Trust:"), p ), 5, 0 );
  glay->addWidget( makeSunkenLabel( i18n(verboseTrust[ key->getTrust() ]), p ), 5, 1 );

  glay->addWidget( new QLabel( i18n("Create date:"), p ), 6, 0 );
  glay->addWidget( makeSunkenLabel( key->getCreateDate(), p ), 6, 1 );

  glay->addWidget( new QLabel( i18n("Expire date:"), p ), 7, 0 );
  glay->addWidget( makeSunkenLabel( key->getExpireDate(), p ), 7, 1 );

  glay->addWidget( new QLabel( i18n("Special status:"), p ), 8, 0 );
  QString status;
  if ( key->isRevoked )
    status = i18n("This key has been revoked!");
  else if ( key->isExpired )
    status = i18n("This key expired!");
  else if ( key->isDisabled )
    status = i18n("This key has been disabled!");
  else
    status = i18n("None");
  glay->addWidget( makeSunkenLabel( status, p ), 8, 1 );

  connect( this, SIGNAL(closeClicked()), SLOT(deleteLater()) );
}
