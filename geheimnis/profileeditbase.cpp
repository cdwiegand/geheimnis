
#include "profileeditbase.h"

#include <kconfig.h>
#if QT_VERSION < 300
#include <kapp.h>
#else
#include <kapplication.h>
#endif

#include <assert.h>

#include "profileeditbase.moc"

ProfileEditBase::ProfileEditBase( QWidget * parent, const char * name )
  : QWidget( parent, name )
{

}

void ProfileEditBase::loadProfile( const QString & profileName )
{
  assert( !profileName.isEmpty() );

  mCurrentProfileName = profileName;

  KConfig * config = kapp->config();
  KConfigGroupSaver saver( config, profileName );
  reallyLoadProfile( config );
}

void ProfileEditBase::saveProfile( const QString & profileName, bool sync ) const
{
  QString group;
  if ( profileName.isNull() ) {
    assert( !mCurrentProfileName.isNull() );
    group = mCurrentProfileName;
  } else {
    group = profileName;
  }
  KConfig * config = kapp->config();
  KConfigGroupSaver saver( config, group );
  reallySaveProfile( config );
  if ( sync ) config->sync();
}

