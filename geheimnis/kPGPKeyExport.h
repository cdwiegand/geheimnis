// -*- c++ -*-

#ifndef KPGPKEYEXPORT_H
#define KPGPKEYEXPORT_H

#include <kdialogbase.h>
#include <qstring.h>

class Profile;
class QButtonGroup;
class QComboBox;
class KURLRequester;

class kPGPKeyExport : public KDialogBase {
  Q_OBJECT

public:
  kPGPKeyExport( Profile * profile, const QString & keyID,
		 QWidget * parent=0, const char * name=0 );

  enum { File, Keyserver, Clipboard };

protected slots:
  void slotOk(); // overridden from KDialogBase

private:
  Profile     *mProfile;
  QString        mKeyID;
  QButtonGroup  *mButtonGroup;
  KURLRequester *mFileRequester;
  QComboBox     *mKeyserverCombo;
};

#endif  // KPGPKEYEXPORT_H




