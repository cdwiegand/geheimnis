
#include "kPGPKeys.h"

#include "klabelaction.h"
#include "misc.h"
#include "myProfile.h"
#include "profilemanager.h"
#include "kPGPKeyList.h"
#include "kPGPKeyAdd.h"
#include "kPGPSelectKey.h"
#include "kPGPKeyExport.h"
#include "kPGPKeyProps.h"
#include "gdebug.h"

#include <klocale.h>
#include <kaction.h>
#include <kstdaction.h>

#include <qstring.h>

#include <assert.h>

#include "kPGPKeys.moc"

#define CHECK_PROFILE assert( mCurrentProfile )
#define CHECK_MULTI_KEY while (false) {} // todo..
#define CHECK_SINGLE_KEY while (false) {} // todo..
#define CHECK_MULTI_SIG while (false) {} // todo..
#define CHECK_SINGLE_SIG while (false) {} // todo..
#define CHECK_OWN_KEY while (false) {} // todo..
#define CHECK_OWN_SIG while (false) {} // todo..
#define CHECK_OTHER_KEY while (false) {} // todo..
#define CHECK_ALL_DISABLED_OR_ENABLED while (false) {}

KeyEditorWindow::KeyEditorWindow( QWidget * parent, const char * name )
  : KMainWindow( parent, name ), mCurrentProfile(0)
{
  setCaption( i18n("Key Management") );

  setupActions();
  setupView();
  slotProfileSelected(0);
}

void KeyEditorWindow::setupView()
{
  mKeyList = new kPGPKeyList( this );
  setCentralWidget( mKeyList );
  connect( mKeyList, SIGNAL(selectionChanged()),
	   SLOT(slotUpdateActionStatus()) );
}

void KeyEditorWindow::setupActions()
{
  KAction * action;

  // "New Key Pair..."
  action = new KAction( i18n("&New Key Pair..."), "genKeys", CTRL+Key_N,
			this, SLOT(slotNewKeyPair()),
			actionCollection(), "new_key_pair" );
  action->setToolTip( i18n("Generate a new key pair "
			   "(both secret and public key)") );

  // "Import Key..."
  action = new KAction( i18n("&Import Key..."), "importKey", CTRL+Key_I,
			this, SLOT(slotImportKey()),
			actionCollection(), "import_key" );
  action->setToolTip( i18n("Import keys from file or keyserver") );

  // "Update Key..."
  action = new KAction( i18n("&Update Key..."), "importKey", CTRL+Key_I,
			this, SLOT(slotUpdateKey()),
			actionCollection(), "update_key" );
  action->setToolTip( i18n("Update selected keys from a keyserver") );

  // "Export Public Key..."
  action = new KAction( i18n("&Export Public Key..."), "exportKey", 0,
			this, SLOT(slotExportPublicKey()),
			actionCollection(), "export_public_key" );
  action->setToolTip( i18n("Export keys to a file or keyserver") );

  // "Properties..."
  action = new KAction( i18n("&Properties..."), "keyInfo", CTRL+Key_P,
			this, SLOT(slotProperties()),
			actionCollection(), "key_properties" );
  action->setToolTip( i18n("Show properties of this key.") );

  // "Edit..."
  action = new KAction( i18n("&Edit..."), "editKey", CTRL+Key_E,
			this, SLOT(slotEditKey()),
			actionCollection(), "edit_key" );
  action->setToolTip( i18n("Edit the key's properties") );

  // "Enable/Disable..."
  action = new KAction( i18n("Enable/Disable.."), "disableKey", 0,
			this, SLOT(slotToggleEnableDisableKey()),
			actionCollection(), "toggle_enable_disable_key" );
  action->setToolTip( i18n("Enable or disable a key. A disabled key is no "
			   "longer used for calculating trust") );

  // "Delete..." (key)
  action = new KAction( i18n("&Delete..."), "removeKey", Key_Delete,
			this, SLOT(slotDeleteKey()),
			actionCollection(), "delete_key" );
  action->setToolTip( i18n("Remove the key from your local keyring.") );

  // "Delete..." (signature)
  action = new KAction( i18n("&Delete..."), "removeSig", Key_Delete,
			this, SLOT(slotDeleteSig()),
			actionCollection(), "delete_sig" );
  action->setToolTip( i18n("Remove the signature from your local copy "
			   "of the key.") );

  // "New User ID..."
  action = new KAction( i18n("New User ID..."), "addName", 0,
			this, SLOT(slotNewUserID()),
			actionCollection(), "new_uid" );
  action->setToolTip( i18n("Add a new User ID, "
			   "e.g. for a new email account") );

  // "Sign..."
  action = new KAction( i18n("&Sign..."), "signKey", 0,
			this, SLOT(slotSignKey()),
			actionCollection(), "sign_key" );
  action->setToolTip( i18n("Sign a key to certify that it belongs to the "
			   "user represented by it's user ID's") );

  // "Revoke..." (signature)
  action = new KAction( i18n("&Revoke..."), "revokeSig", 0,
			this, SLOT(slotRevokeSig()),
			actionCollection(), "revoke_sig" );
  action->setToolTip( i18n("Revoke your signature, so that other's won't "
			   "trust it anymore") );

  // "Revoke..." (key)
  action = new KAction( i18n("&Revoke..."), "revokeKey", 0,
			this, SLOT(slotRevokeKey()),
			actionCollection(), "revoke_key" );
  action->setToolTip( i18n("Revoke the key, i.e. tell the world not to "
			   "use it anymore") );

  // "Reload Keyring..."
  action = new KAction( i18n("Reload keyring"), "reload", 0,
			this, SLOT(slotReloadKeyring()),
			actionCollection(), "reload_keyring" );
  action->setToolTip( i18n("Force reload of keyring, e.g. if you changed it "
			   "using GnuPG or PGP directly") );

  // "Profile:" label:
  (void) new KLabelAction( i18n("Profile:"),
			   actionCollection(), "select_profile_label" );

  // "Profile" selection:
  KSelectAction * profileAction =
    new KSelectAction( i18n("Select Profile"), QString::null, 0,
		       0, 0, // don't connect to activated(), see below:
		       actionCollection(), "select_profile" );
  profileAction->setItems( profileManager.profileNameList() );
  assert( !profileManager.profileNameList().isEmpty() );
  connect( profileAction, SIGNAL(activated(int)),
	   SLOT(slotProfileSelected(int)) );

  // "Close"
  (void) KStdAction::close( this, SLOT(close()), actionCollection() );

  createGUI( "geheimniskeyeditorui.rc", false );
}

KeyEditorWindow::~KeyEditorWindow() {
}

void KeyEditorWindow::slotNewKeyPair()
{ // preliminaries: profile set
  CHECK_PROFILE;

  mCurrentProfile->genKeyPair();
  slotReloadKeyring( false );
}

void KeyEditorWindow::slotImportKey()
{ // preliminaries: profile set
  CHECK_PROFILE;

  (new KeyImportDialog( mCurrentProfile, mKeyList,
			mKeyList->getSelectedKeyID(), this ))->show();
  // will call mKeyList->loadKeyring() on successful add...
}

void KeyEditorWindow::slotUpdateKey()
{ // preliminaries: profile set, key(s) selected
  CHECK_PROFILE;
  CHECK_MULTI_KEY;

  (new KeyImportDialog( mCurrentProfile, mKeyList,
			mKeyList->getSelectedKeyID(), this ))->show();
  // will call mKeyList->loadKeyring() on successful add...
}

void KeyEditorWindow::slotExportPublicKey()
{ // preliminaries: profile set, key(s) selected
  CHECK_PROFILE;
  CHECK_MULTI_KEY;

  (new kPGPKeyExport( mCurrentProfile, mKeyList->getSelectedKeyID(), this))->show();
  // No update needed - we just exported!
}

void KeyEditorWindow::slotProperties()
{ // preliminaries: profile set, key selected
  CHECK_PROFILE;
  CHECK_SINGLE_KEY;

  pkiKey * key = mCurrentProfile->getKey( mKeyList->getSelectedKeyID() );
  if ( key )
    (new KeyPropertiesDialog( key, this ))->show(); // deletes itself on close!
}

void KeyEditorWindow::slotEditKey()
{ // preliminaries: profile set, key selected
  CHECK_PROFILE;
  CHECK_SINGLE_KEY;

  mCurrentProfile->editKey( mKeyList->getSelectedKeyID() );
  slotReloadKeyring( false );
}

void KeyEditorWindow::slotToggleEnableDisableKey()
{ // preliminaries: profile set, key(s) selected,
  //                all keys either disabled or enabled
  CHECK_PROFILE;
  CHECK_MULTI_KEY;
  CHECK_ALL_DISABLED_OR_ENABLED;

  mCurrentProfile->disEnableKey( mKeyList->getSelectedKeyID() );
  slotReloadKeyring( false );
}

void KeyEditorWindow::slotDeleteKey()
{ // preliminaries: profile set, key(s) selected
  CHECK_PROFILE;
  CHECK_MULTI_KEY;

  mCurrentProfile->removeKey( mKeyList->getSelectedKeyID() );
  slotReloadKeyring( false );
}

void KeyEditorWindow::slotDeleteSig()
{ // preliminaries: profile set, sig(s) selected
  CHECK_PROFILE;
  CHECK_MULTI_SIG;

  // not yet implemented:
  //  mKeyList->removeSig( mKeyList->getSelectedKeyID() );
}

void KeyEditorWindow::slotNewUserID()
{ // preliminaries: profile set, own key selected
  CHECK_PROFILE;
  CHECK_OWN_KEY;

  mCurrentProfile->addName( mKeyList->getSelectedKeyID() );
  slotReloadKeyring( false );
}

void KeyEditorWindow::slotSignKey()
{ // preliminaries: profile set, other key selected,
  CHECK_PROFILE;
  CHECK_OTHER_KEY;

  QString keyIDToSign = mKeyList->getSelectedKeyID();
  QString ownKeyID =
    SelectSecretKeyDialog::getSecretKey( mCurrentProfile,
					 i18n("&Select secret key to use for "
					      "signing:") );
  mCurrentProfile->signKey( keyIDToSign, ownKeyID );
  slotReloadKeyring( false );
}

void KeyEditorWindow::slotRevokeSig()
{ // preliminaries: profile set, own sig selected
  CHECK_PROFILE;
  CHECK_OWN_SIG;

  mCurrentProfile->revokeSignature( mKeyList->getSelectedKeyID() );
  slotReloadKeyring( false );
}

void KeyEditorWindow::slotRevokeKey()
{ // preliminaries: profile set, own key selected
  CHECK_PROFILE;
  CHECK_OWN_KEY;

  mCurrentProfile->removeRevokeKey( mKeyList->getSelectedKeyID() );
  slotReloadKeyring( false );
}

void KeyEditorWindow::slotReloadKeyring()
{
  slotReloadKeyring( true );
}

void KeyEditorWindow::slotReloadKeyring( bool force )
{ // prelimniaries: profile set
  CHECK_PROFILE;

  if ( force ) {
    mKeyList->setEnabled( false );
    mCurrentProfile->loadKeyring(); // loads into profile's memory
    mKeyList->setEnabled( true );
  }
  mKeyList->loadKeyring(); // shows on screen
}

void KeyEditorWindow::slotProfileSelected(int idx)
{
  QString profileName = *profileManager.profileNameList().at( idx );
  if ( mCurrentProfile && mCurrentProfile->name() == profileName )
    return; // nothing has changed...

  gDebug() << "profileName = \"" << profileName << "\"." << endl;
  mCurrentProfile = profileManager.profileForName( profileName );
  gDebug() << "curProfile set." << endl;
  if ( mCurrentProfile ) {
    mKeyList->setProfile( mCurrentProfile );
    // Force a COMPLETE reload
    gDebug() << "before loadKeyring" << endl;
    slotReloadKeyring();
    gDebug() << "after loadkeyring" << endl;
  } else {
    showBadProfileDialog();
  }
  slotUpdateActionStatus();
}

void KeyEditorWindow::slotUpdateActionStatus()
{
  if ( !mCurrentProfile )
    // disable almost all actions:
    stateChanged( "StateNoProfile" );
  else if ( mKeyList->selectedItems().isEmpty() )
    stateChanged( "StateNoSelection" );
  else
    stateChanged( "StateSelection" );
}

#if QT_VERSION < 300

// emulate KDE 3's stateChanged()
// this is slow but should work...

#include <qdom.h>

void KeyEditorWindow::stateChanged( const QString & state )
{
  QDomElement stateElement;

  // search for the right <State> element:
  QDomNodeList nodeList = domDocument().elementsByTagName( "State" );
  uint count = nodeList.length();
  for ( int i = 0 ; i < count ; ++i ) {
    QDomElement e = nodeList.item(i).toElement();
    assert( !e.isNull() );
    if ( e.attribute( "name" ) == state ) {
      stateElement = e;
      break;
    }
  }

  if ( stateElement.isNull() ) {
    gWarning() << "No <State> element with name = \"" << state << "\" found!"
	       << endl;
    return;
  }

  // first, the actions to enable (only evaluates the first <enable> element)
  QDomElement enable = stateElement.elementsByTagName( "enable" ).item(0).toElement();
  if ( !enable.isNull() ) {
    for ( QDomNode n = enable.firstChild() ; !n.isNull() ; n = n.nextSibling() )
      if ( n.isElement() ) {
	QString actionName = n.toElement().attribute( "name" );
	if ( !actionName.isEmpty() )
	  action( actionName.latin1() )->setEnabled( true );
      }
  }

  // then, the actions to disable (only evaluates the first <disable> element)
  QDomElement disable = stateElement.elementsByTagName( "disable" ).item(0).toElement();
  if ( !disable.isNull() ) {
    for ( QDomNode n = disable.firstChild() ; !n.isNull() ; n = n.nextSibling() )
      if ( n.isElement() ) {
	QString actionName = n.toElement().attribute( "name" );
	if ( !actionName.isEmpty() )
	  action( actionName.latin1() )->setEnabled( false );
      }
  }
}

#endif
