
#include "profilemanager.h"

#include "myProfile.h"
#include "gdebug.h"

#if QT_VERSION < 300
#include <kapp.h>
#else
#include <kapplication.h>
#endif
#include <kconfig.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qcombobox.h>

//#include <iostream.h>
#include <stdio.h>
#include <assert.h>

static const char * const keyProfileList = "ProfileList";
static const char * const keyDefaultProfile = "DefaultProfile";
static const char * const keyProfilePgpVersion = "PGPVersion";
static const char * const groupGlobal = "__Global";

ProfileManager::ProfileManager()
  : mDefaultProfile( 0 )
{
}

ProfileManager::~ProfileManager() {
  // not sure if we could use gDebug() here:
  // std::cerr << "~ProfileManager" << endl;
  // cdw: uhh... gcc3 don't like std::cerr, maybe std::err?
  // I don't use the STL, so here's the C, it works.
  fprintf(stderr,"~ProfileManager\n");

  // _we_ have to delete the profiles, since we are Profile's only friend :-)
  for ( QValueList<Profile*>::Iterator it = mProfiles.begin() ;
	it != mProfiles.end() ; ++it )
    delete (*it);
}

ProfileManager & ProfileManager::instance() {
  static ProfileManager self;
  gFatal( !kapp ) << "ProfileManager needs a KApplication object to "
    "function properly!" << endl;
  return self;
}

QStringList ProfileManager::profileNameList() {
  KConfigGroupSaver saver( kapp->config(), groupGlobal );
  QStringList result = kapp->config()->readListEntry( keyProfileList, ';' );
  if ( result.isEmpty() ) {
    createDefaultProfiles();
    return kapp->config()->readListEntry( keyProfileList, ';' );
  } else
    return result;
}

Profile * ProfileManager::defaultProfile() {
  if ( !mDefaultProfile ) {
    KConfig * config = kapp->config();
    KConfigGroupSaver saver( config, groupGlobal );

    QString defaultProfileName = config->readEntry( keyDefaultProfile );
    if ( defaultProfileName.isEmpty() ) {
      defaultProfileName = profileNameList().first();
      config->writeEntry( keyDefaultProfile, defaultProfileName );
      config->sync();
    }
    mDefaultProfile = profileForName( defaultProfileName );
    assert( mDefaultProfile );
  }

  return mDefaultProfile;
}

Profile * ProfileManager::profileForName( const QString & profileName,
					  bool onlyCached )
{
  if ( profileName.isEmpty() ) return 0;

  // first, try if we have already one such profile:
  for ( QValueList<Profile*>::Iterator it = mProfiles.begin() ;
	it != mProfiles.end() ; ++it )
    if ( (*it)->name() == profileName ) return (*it);

  if ( onlyCached ) return 0;

  // if not, check, if such a profile is defined in the config:
  if ( profileNameList().contains( profileName ) ) {
    Profile * p = new Profile( profileName );
    mProfiles << p;
    return p;
  }

  return 0; // not found.
}

Profile * ProfileManager::createProfile( const QString & profileName,
					 int pgpVersion )
{
  if ( profileName.isEmpty() ) return 0; // sorry, no empty names
  if ( profileForName( profileName ) ) return 0; // sorry, no duplicates

  createProfileConfigEntry( profileName, pgpVersion );

  return profileForName( profileName );
}

void ProfileManager::deleteProfile( const QString & profileName ) {
  Profile * profile = profileForName( profileName, true ); // only search in cache
  if ( profile )
    deleteProfile( profile );
  else
    removeProfileFromConfig( profileName );
}

void ProfileManager::deleteProfile( Profile * profile ) {
  QString profileName = profile->name();
#if QT_VERSION < 300
  uint count = mProfiles.contains( profile );
  mProfiles.remove( profile );
#else
  uint count = mProfiles.remove( profile );
#endif
  if ( count )
    delete profile;
  profile = 0;
  removeProfileFromConfig( profileName );
}

void ProfileManager::removeProfileFromConfig( const QString & profileName ) {
  QStringList profileNames = profileNameList();
#if QT_VERSION < 300
  if ( profileNames.contains( profileName ) == 0 ) return; // nothing to do!
  else profileNames.remove( profileName );
#else
  if ( profileNames.remove( profileName ) == 0 ) return; // nothing to do!
#endif

  KConfig * config = kapp->config();
  KConfigGroupSaver saver( config, groupGlobal );
  // check whether we are going to delete the default profile:
  if ( config->readEntry( keyDefaultProfile ) == profileName )
    // and set a new default:
    setDefaultProfile( profileNames.first() );
  // remove it from the list of profiles:
  config->writeEntry( keyProfileList, profileNames );
#if QT_VERSION >= 300
  // and remove corresponding group (can't do that in KDE 2)
  config->deleteGroup( profileName );
#endif
}

void ProfileManager::setDefaultProfile( const QString & profileName ) {
  Profile * profile = profileForName( profileName );
  if ( profile )
    setDefaultProfile( profile );
}

void ProfileManager::setDefaultProfile( Profile * profile ) {
  if ( !profile ) return;
  assert( mProfiles.find( profile ) != mProfiles.end() );

  KConfig * config = kapp->config();
  KConfigGroupSaver saver( config, groupGlobal );
  config->writeEntry( keyDefaultProfile, profile->name() );
  mDefaultProfile = profile;
}

void ProfileManager::setToDefaultProfile( QComboBox * cb ) {
  QString defaultProfileName = defaultProfile()->name();

  uint count = cb->count();
  for ( uint i = 0 ; i < count ; ++i )
    if ( cb->text(i) == defaultProfileName )
      cb->setCurrentItem( i );
}

void ProfileManager::createDefaultProfiles() {
  // ### FIXME: first check whether the backend actually exists,
  // before creating a profile for it :-)
  createProfileConfigEntry( "GnuPG 1.0.6", 1 );
  createProfileConfigEntry( "PGP 5.0", 5 );
  createProfileConfigEntry( "PGP 2.6", 2 );
  createProfileConfigEntry( "PGP 6.5", 6 );
}

void ProfileManager::createProfileConfigEntry( const QString & profileName,
					       int pgpVersion )
{
  KConfig * config = kapp->config();
  KConfigGroupSaver saver( config, groupGlobal );

  // append to list of profiles: (don't call profileNameList() here,
  // since we could have been called by the latter :-o
  QStringList profileNames = kapp->config()->readListEntry( keyProfileList, ';' );
  profileNames << profileName;
  config->writeEntry( keyProfileList, profileNames, ';' );
  // and create the corresponding group with entry pgpVersion
  KConfigGroupSaver saver2( config, profileName );
  config->writeEntry( keyProfilePgpVersion, pgpVersion );
  config->sync();
}
