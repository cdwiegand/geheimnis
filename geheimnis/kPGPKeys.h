// -*- c++ -*-

#ifndef KPGPKEYS_H
#define KPGPKEYS_H

#include <kmainwindow.h>

class Profile;
class kPGPKeyList;
class KComboBox;
class QString;

class KeyEditorWindow : public KMainWindow {
  Q_OBJECT

public:
  KeyEditorWindow( QWidget * parent=0, const char * name=0 );
  ~KeyEditorWindow();
		
protected slots:
  // action targets:
  void slotNewKeyPair();
  void slotImportKey();
  void slotUpdateKey();
  void slotExportPublicKey();
  void slotProperties();
  void slotEditKey();
  void slotToggleEnableDisableKey();
  void slotDeleteKey();
  void slotDeleteSig();
  void slotNewUserID();
  void slotSignKey();
  void slotRevokeSig();
  void slotRevokeKey();
  void slotReloadKeyring();
  void slotProfileSelected(int);

  void slotReloadKeyring( bool force );

  void slotUpdateActionStatus();

#if QT_VERSION < 300
  // emulate KDE 3's stateChanged()...
protected:
  void stateChanged( const QString & state );
#endif

private:
  kPGPKeyList *mKeyList;
  Profile * mCurrentProfile;

  // Private functions...
  void setupActions();
  void setupView();
};

#endif  // KPGPKEYS_H


