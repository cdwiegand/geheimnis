// -*- c++ -*-

#ifndef KPGPKEYADD_H
#define KPGPKEYADD_H

#include <kdialogbase.h>
#include <qstring.h>

class Profile;
class kPGPKeyList;
class QButtonGroup;
class KURLRequester;
class QComboBox;
class QLineEdit;


class KeyImportDialog : public KDialogBase {
  Q_OBJECT

public:
  KeyImportDialog( Profile * profile, kPGPKeyList * passedKPGPKeyList,
		   const QString & keyID=QString::null,
		   QWidget * parent=0, const char * name=0 );
  void setFile( const QString & c );
  void setKeyID( const QString & c );
  void setClipboard();

  enum { File, Keyserver, Clipboard };

protected slots:
  void slotOk();

private:
  Profile     * mProfile;
  kPGPKeyList   * mKeyWindow;

  QButtonGroup  * mButtonGroup;
  KURLRequester * mFileRequester;
  QComboBox     * mKeyserverCombo;
  QLineEdit     * mKeyserverQuery;

  void mainFun();
};

#endif 




