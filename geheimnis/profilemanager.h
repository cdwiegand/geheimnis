// -*- c++ -*-

#ifndef GEHEIMNIS_PROFILE_MANAGER_H
#define GEHEIMNIS_PROFILE_MANAGER_H

#include <qvaluelist.h>

#define profileManager ProfileManager::instance()

/**
 * @short Singleton manager for @ref Profile's
 * @author Marc Mutz <mutz@kde.org>
 */

class Profile;
class QString;
class QStringList;
class QComboBox;

class ProfileManager {
  ProfileManager();
  ~ProfileManager();
public:
  /** Returns the (only) instance of this class */
  static ProfileManager & instance();

  /** Profile factory. Always use this function if you need a profile.
      (You can't create it another way, anyway :-).
      The manager instance own the profile, so don't delete it yourself!
  */
  Profile * profileForName( const QString & profileName, bool onlyCached=false );
  QStringList profileNameList();
  Profile * defaultProfile();
  Profile * createProfile( const QString & profileName, int pgpVersion );
  void deleteProfile( const QString & profileName );
  void deleteProfile( Profile * profile );
  void setDefaultProfile( const QString & profileName );
  void setDefaultProfile( Profile * profile );

  void setToDefaultProfile( QComboBox * cb );

private:
  void removeProfileFromConfig( const QString & profileName );
  void createDefaultProfiles();
  void createProfileConfigEntry( const QString & profileName, int pgpVersion );
  Profile * mDefaultProfile;
  QValueList<Profile*> mProfiles; // can't use QPtrList, since ~Profile is private
};



#endif // GEHEIMNIS_PROFILE_MANAGER_H
