
#include "kPGPPrefs.h"

#include "kPGPProfileEdit_GPG.h"
#include "kPGPProfileEdit_PGP2.h"
#include "kPGPProfileEdit_PGP5.h"
#include "kPGPProfileEdit_PGP6.h"
#include "kPGPProfileEdit_New.h"
#include "myProfile.h"
#include "profilemanager.h"
#include "defines.h"
#include "misc.h"

#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurlrequester.h>
#include <kiconloader.h>
#include <kdebug.h>
#if QT_VERSION < 300
#include <kapp.h>
#else
#include <kapplication.h>
#endif

#include <qstring.h>
#include <qlayout.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qfileinfo.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qwidgetstack.h>

#include <assert.h>

#include "kPGPPrefs.moc"

kPGPPrefs::kPGPPrefs( QWidget * parent, const char * name )
  : KDialogBase( IconList, i18n("Configure Geheimnis"),
		 Help|Ok|Cancel, Ok, parent, name )
{
  // tmp. vars:
  QWidget * page;
  QGridLayout * glay;

  // "General" tab:
  page = addPage( i18n("General"), i18n("Settings common to all backends"),
		  kapp->iconLoader()->loadIcon("misc", KIcon::NoGroup,
					       KIcon::SizeMedium) );
  
  glay = new QGridLayout( page, 9, 2 /* rows x cols */, 0 /*margin*/,
			  spacingHint() );
  glay->setColStretch( 1, 1 );
  glay->setRowStretch( 8, 1 ); // last row is spacer

  // row 0: "Terminal" combobox and label:
  cmbTerminal = new QComboBox( true, page );
  cmbTerminal->insertItem("konsole -caption \"%T\" -e %C");
  cmbTerminal->insertItem("gnome-terminal -t \"%T\" -e \"%C\"");
  cmbTerminal->insertItem("xterm -T \"%T\" -e \"%C\"");
  cmbTerminal->insertItem("rxvt -title \"%T\" -e \"%C\"");
  cmbTerminal->setMinimumSize(cmbTerminal->sizeHint());
  glay->addWidget( new QLabel( cmbTerminal, i18n("&Terminal:"), page ), 0, 0 );
  glay->addWidget( cmbTerminal, 0, 1 );

  // row 1: label
  glay->addMultiCellWidget( new QLabel( i18n("%C will be replaced with the command to run.\n"
					     "%T will be replaced with the title."), page ), 1, 1, 0, 1 );

  // row 2: "Comment" lineedit and label
  txtComment = new QLineEdit( page );
  glay->addWidget( new QLabel( txtComment, i18n("Co&mment:"), page ), 2, 0 );
  glay->addWidget( txtComment, 2, 1 );

  // row 3: "Path to Geheimnis" KURLRequester and label:
  cmbPKO = new KURLRequester( page );
  glay->addWidget( new QLabel( cmbPKO, i18n("&Path to geheimnis:"), page ), 3, 0 );
  glay->addWidget( cmbPKO, 3, 1 );

  // row 4: "View keys as tree" checkbox:
  chkKeyTreeView = new QCheckBox( i18n("&View keys as tree"), page );
  glay->addMultiCellWidget( chkKeyTreeView, 4, 4, 0, 1 );

  // row 5: "Use CURL [...]" checkbox:
  chkUseCurl = new QCheckBox( i18n("&Use curl to access keyserver"), page );
  glay->addMultiCellWidget( chkUseCurl, 5, 5, 0, 1 );

  // row 6: "Keyserver" combobox and label:
  cboKeyserver = new QComboBox( true, page );
  cboKeyserver->insertStringList( getKeyserverList() );
  cboKeyserver->setMinimumSize(cboKeyserver->sizeHint());
  glay->addWidget( new QLabel( cboKeyserver, i18n("&Keyserver:"), page ), 6, 0 );
  glay->addWidget( cboKeyserver, 6, 1 );

  // row 7: "autom. show key management" checkbox:
  chkAutoKeyManagement = new QCheckBox( i18n("&Automatically open key management by default"), page);
  glay->addMultiCellWidget( chkAutoKeyManagement, 7, 7, 0, 1);


  // ----------------------- Page 2 ----------------------------

  page = addPage( i18n("Backends"), i18n("Backend-specific settings"),
		  kapp->iconLoader()->loadIcon( "encrypted", KIcon::NoGroup,
						KIcon::SizeMedium ) );
  glay = new QGridLayout( page, 4, 3 /* rows x cols */, 0 /*margin*/,
			  spacingHint() );
  glay->setRowStretch( 3, 1 );
  glay->setColStretch( 1, 1 );
  glay->setColStretch( 2, 1 );

  // row 0: "Profile" combobox and label:
  cbPGPVersion = new QComboBox( false, page );
  cbPGPVersion->insertStringList( profileManager.profileNameList() );
  cbPGPVersion->setMinimumSize( cbPGPVersion->sizeHint() );
  connect( cbPGPVersion, SIGNAL(activated(const QString &)),
	   this, SLOT(cbPGPVersion_activated(const QString &)) );
  glay->addWidget( new QLabel( cbPGPVersion, i18n("&Profile:"), page ), 0, 0 );
  glay->addMultiCellWidget( cbPGPVersion, 0, 0, 1, 2 );

  // row 1: "Default profile" checkbox:
  chkDefault = new QCheckBox( i18n("&Default profile"), page );
  connect( chkDefault, SIGNAL(clicked()),
	   this, SLOT(chkDefault_clicked()) );
  glay->addMultiCellWidget( chkDefault, 1, 1, 1, 2 );

  // row 2: "New", "Edit" and "Remove" button row:
  cmdNewP = new QPushButton( i18n("&New..."), page );
  cmdNewP->setMinimumSize( cmdNewP->sizeHint() );

  cmdRemoveP = new QPushButton( i18n("&Remove"), page );
  cmdRemoveP->setMinimumSize( cmdRemoveP->sizeHint() );
  
  connect( cmdNewP, SIGNAL(clicked()),
	   this, SLOT(cmdNewP_clicked()) );
  connect( cmdRemoveP, SIGNAL(clicked()),
	   this, SLOT(cmdRemoveP_clicked()) );

  glay->addWidget( cmdNewP, 2, 1 );
  glay->addWidget( cmdRemoveP, 2, 2 );

  // row 3: widget stack holding edit widgets for the currently
  //        selected profile:
  mWidgetStack = new QWidgetStack( page );
  glay->addMultiCellWidget( mWidgetStack, 3, 3, 0, 2 );

  mWidgetStack->addWidget( new QWidget( mWidgetStack ), 0 ); // empty
  
  
  mWidgetStack->addWidget( new GPGProfileEdit( mWidgetStack ), 1 );
  mWidgetStack->addWidget( new PGP2ProfileEdit( mWidgetStack ), 2 );
  mWidgetStack->addWidget( new PGP5ProfileEdit( mWidgetStack ), 5 );
  mWidgetStack->addWidget( new PGP6ProfileEdit( mWidgetStack ), 6 );

  KConfig * cfg = kapp->config();
  KConfigGroupSaver saver( cfg, "__Global" );

  cmbTerminal->setEditText( cfg->readEntry( "Terminal",
					    "konsole -caption %T -e %C") );
  if ( !cfg->hasKey( "Comment" ) )
    txtComment->setText( i18n("Made with %1").arg(MY_APP_NAME) );
  else
    txtComment->setText( cfg->readEntry( "Comment" ) );

  chkAutoKeyManagement->setChecked( cfg->readBoolEntry( "AutoKeyManagement", false ) );
  cmbPKO->setURL( cfg->readEntry( "PathGeheimnis", "geheimnis" ) );
  chkKeyTreeView->setChecked( cfg->readBoolEntry( "KeyTreeView", true ) );
  chkUseCurl->setChecked( cfg->readBoolEntry( "UseCurl", false ) );
  cboKeyserver->setEditText( cfg->readEntry( "Keyserver", "pgpkeys.mit.edu" ) );

  updateList(); // needed?
  cbPGPVersion_activated( cbPGPVersion->currentText() );
}

void kPGPPrefs::slotOk()
{
  saveChanges();
  KDialogBase::slotOk();
}

void kPGPPrefs::saveChanges()
{
  KConfig * cfg = kapp->config();
  KConfigGroupSaver saver( cfg, "__Global" );

  // SANITY CHECK!!
  QFileInfo qfi( cmbPKO->url() );
  if ( !qfi.isExecutable() )
    KMessageBox::information( this,
			      i18n("The path you entered for geheimnis does "
				   "not lead to an executable.\n"
				   "This will cause errors with certain "
				   "operations.\n" ) );

  // Standard is if no - option set, add -e because it's pseudo-standard...
  QString tmp = cmbTerminal->currentText();
  if ( tmp.find('-') < 0 )
    tmp += " -e ";
  cfg->writeEntry( "Terminal", tmp );
  cfg->writeEntry( "KeyTreeView", chkKeyTreeView->isChecked() );
  cfg->writeEntry( "Comment", txtComment->text() );
  cfg->writeEntry( "PathGeheimnis", cmbPKO->url() );
  cfg->writeEntry( "Keyserver", cboKeyserver->currentText() );
  cfg->writeEntry( "AutoKeyManagement", chkAutoKeyManagement->isChecked() );
  cfg->writeEntry( "UseCurl", chkUseCurl->isChecked() );

  ProfileEditBase * profile = dynamic_cast<ProfileEditBase*>( mWidgetStack->visibleWidget() );
  if ( profile )
    profile->saveProfile( QString::null, false ); // keep name, don't sync

  cfg->sync();
}

void kPGPPrefs::selectProfile(const QString & profileName) {
  if ( !profileName ) return;
  for ( int i=0 ; i < cbPGPVersion->count() ; i++ )
    if ( cbPGPVersion->text(i) == profileName )
      cbPGPVersion->setCurrentItem(i);
}

void kPGPPrefs::cmdNewP_clicked() {
  NewProfileDialog pe( this );
  if ( pe.exec() == QDialog::Accepted ) {
    updateList();
    selectProfile( pe.profileName() );
// #if QT_VERSION >= 300
    cbPGPVersion_activated( pe.profileName() );
// #endif
  }
}

void kPGPPrefs::cmdRemoveP_clicked() {
  profileManager.deleteProfile( cbPGPVersion->currentText() );
  updateList();
  cbPGPVersion_activated( cbPGPVersion->currentText() );
}

void kPGPPrefs::updateList()
{
  cbPGPVersion->clear();
  cbPGPVersion->insertStringList(profileManager.profileNameList());
  cmdRemoveP->setEnabled( cbPGPVersion->count() );
}

void kPGPPrefs::cbPGPVersion_activated( const QString & profileName )
{
  kdDebug() << "kPGPPrefs::cbPGPVersion_activated( \""
	    << profileName << "\" ) called" << endl;
  ProfileEditBase * profile = 0;
  KConfig * cfg = kapp->config();

  {
    KConfigGroupSaver saver( cfg, "__Global" );
    QString defaultProfile = cfg->readEntry("DefaultProfile");
    chkDefault->setChecked( profileName == defaultProfile );
  }

  profile = dynamic_cast<ProfileEditBase*>( mWidgetStack->visibleWidget() );
  if ( profile )
    profile->saveProfile(); // keep name, sync

  KConfigGroupSaver saver( cfg, profileName );
  assert( profileManager.profileForName( profileName ) );
  int type = profileManager.profileForName( profileName )->getPGPVersion();
  profile = dynamic_cast<ProfileEditBase*>( mWidgetStack->widget( type ) );
  if ( profile )
    profile->loadProfile( profileName );
  mWidgetStack->raiseWidget( type );
}

void kPGPPrefs::chkDefault_clicked()
{
  KConfig * cfg = kapp->config();
  KConfigGroupSaver saver( cfg, "__Global" );
  if ( chkDefault->isChecked() )
    // then we have a new defaultprofile!
    cfg->writeEntry( "DefaultProfile", cbPGPVersion->currentText() );
  else
    // just removed the default profile!
    cfg->writeEntry( "DefaultProfile", "" );
  cfg->sync();
}

