// -*- c++ -*-

#ifndef KPGPSELECTKEY_H
#define KPGPSELECTKEY_H

#include <kdialogbase.h>

#include <qstringlist.h>

class Profile;
class QString;
class QComboBox;

class SelectSecretKeyDialog : public KDialogBase {
  Q_OBJECT

public:
  SelectSecretKeyDialog( Profile * profile, const QString & prompt,
			 QWidget * parent=0, const char * name=0 );

  QString selectedKey() const;

  static QString getSecretKey( Profile * profile, const QString & prompt );

private:
  void loadSecretKeys( Profile * profile );

private:
  QComboBox  *mSecretKeysCombo;
  QStringList mKeys;
};

#endif  // KPGPSELECTKEY_H

