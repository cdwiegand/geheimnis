
#include "kPGPSelectKey.h"

#include "myKeyring.h"
#include "myProfile.h"
//#include "kPGPKeyList.h"
//#include "defines.h"

#include <klocale.h>

#include <qstring.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
//#include <qpushbutton.h>
//#include <qlistbox.h>

#include <assert.h>

#include "kPGPSelectKey.moc"

SelectSecretKeyDialog::SelectSecretKeyDialog( Profile * profile,
					      const QString & prompt,
					      QWidget * parent,
					      const char * name )
  : KDialogBase( Plain, i18n("Secret Key Selection"), Help|Ok|Cancel, Ok,
		 parent, name )
{
  assert( profile );

  QVBoxLayout * vlay = new QVBoxLayout( plainPage(), 0, spacingHint() );

  mSecretKeysCombo = new QComboBox( false, plainPage() );
  loadSecretKeys( profile );
  QString msg = prompt.isEmpty() ? i18n("&Select secret key to use:") : prompt ;
  vlay->addWidget( new QLabel( mSecretKeysCombo, msg, plainPage() ) );
  vlay->addWidget( mSecretKeysCombo );
  vlay->addStretch( 1 );
}

QString SelectSecretKeyDialog::selectedKey() const
{
  int idx = mSecretKeysCombo->currentItem();
  return ( idx >= 0 ) ? *mKeys.at( idx ) : QString::null ;
}

QString SelectSecretKeyDialog::getSecretKey( Profile * profile,
					     const QString & prompt )
{
  SelectSecretKeyDialog dlg( profile, prompt );
  if ( dlg.exec() == QDialog::Accepted )
    return dlg.selectedKey();
  else
    return QString::null;
}

void SelectSecretKeyDialog::loadSecretKeys( Profile * profile )
{
  assert( profile );
  assert( profile->keyring );

  mSecretKeysCombo->clear();
  mKeys.clear();

  uint count = profile->keyring->count();
  for ( uint i = 0 ; i < count ; ++i ) {
    // Per KEY
    pkiKey * key = profile->keyring->at(i);
    if ( key && key->haveSecret && !key->isRevoked && !key->isExpired ) {
      mKeys << key->getKeyID();
      if ( key->names.isEmpty() )
	mSecretKeysCombo->insertItem( i18n("???? (%2)")
				 .arg( key->getKeyType() ) );
      else
	mSecretKeysCombo->insertItem( i18n("%1 <%2> (%3)")
				.arg( key->names.first()->getDisplayName() )
				.arg( key->names.first()->getEmailAddress() )
				.arg( key->getKeyType() ) );
    }
  }
}
