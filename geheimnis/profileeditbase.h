// -*- c++ -*-

#ifndef GEHEIMNIS_PROFILEEDITBASE_H
#define GEHEIMNIS_PROFILEEDITBASE_H

#include <qwidget.h>
#include <qstring.h>

class KConfig;

/** 
 * @short Base class for Geheimnis profile edit widgets.
 * @author Marc Mutz <mutz@kde.org>
 */

class ProfileEditBase : public QWidget {
  Q_OBJECT
public:
  ProfileEditBase( QWidget * parent=0, const char * name=0 );

  /** loads the profile with name @p profileName from the app-global
      config. */
  void loadProfile( const QString & profileName );
  /** saves the currently displayed profile in the app-global
      config. If @p profileName is null, @p mCurrentProfileName is
      used. If that is also null, an assert is hit.

      If @p sync is true, the config is synced to disk.
  */
  void saveProfile( const QString & profileName=QString::null, bool sync=true ) const;

  /** In reimplementations, clear all widgets in this method. */
  virtual void clear() = 0;

protected:
  /** In reimplementations, read the relevant data from @p config
      without worrying about groups and stuff. */
  virtual void reallyLoadProfile( KConfig * config ) = 0;
  /** In reimplementations, write the relevant data to @p config
      without worrying about groups and stuff. */
  virtual void reallySaveProfile( KConfig * config ) const = 0;

protected:
  /** Holds the profile name from the last @ref loadProfile call */
  QString mCurrentProfileName;
};

#endif // GEHEIMNIS_PROFILEEDITBASE_H
