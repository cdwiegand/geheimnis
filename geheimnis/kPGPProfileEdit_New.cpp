
#include "kPGPProfileEdit_New.h"

#include "myProfile.h"
#include "profilemanager.h"
#include "kPGPPrefs.h"
#include "defines.h"
#include "misc.h"

#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#if QT_VERSION < 300
#include <kapp.h>
#else
#include <kapplication.h>
#endif

#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlineedit.h>

#include "kPGPProfileEdit_New.moc"

NewProfileDialog::NewProfileDialog( QWidget * parent, const char * name )
  : KDialogBase( Plain, i18n("New Profile"), Ok|Cancel/*|Help*/,
		 Ok, parent, name )
{
  QVBoxLayout * vlay = new QVBoxLayout( plainPage(), 0, spacingHint() );

  cbPGPVersion = new QComboBox( false, plainPage() );
  cbPGPVersion->insertItem("GnuPG 1.0.x");
  cbPGPVersion->insertItem("PGP 6.5");
  cbPGPVersion->insertItem("PGP 5.0");
  cbPGPVersion->insertItem("PGP 2.6x");
  connect(cbPGPVersion,SIGNAL(activated(int)),SLOT(cbPGPVersion_clicked()));
  vlay->addWidget( new QLabel( cbPGPVersion,
			       i18n("Choose the &encryption program to use:"),
			       plainPage() ) );
  vlay->addWidget( cbPGPVersion );

  txtProfileName = new QLineEdit( plainPage() );
  vlay->addWidget( new QLabel( txtProfileName,
			       i18n("Enter the &name of the new profile:"),
			       plainPage() ) );
  vlay->addWidget( txtProfileName );
}

void NewProfileDialog::cbPGPVersion_clicked() {
  if ( txtProfileName->text().stripWhiteSpace().isEmpty() )
    txtProfileName->setText( cbPGPVersion->currentText() );
}

void NewProfileDialog::slotOk()
{
  saveConfig();
  KDialogBase::slotOk();
}

QString NewProfileDialog::profileName() const
{
  return txtProfileName->text().stripWhiteSpace();
}

void NewProfileDialog::saveConfig()
{
	KConfig *cfg = NULL;

	QStringList profileList = profileManager.profileNameList();
	profileList.append(txtProfileName->text().stripWhiteSpace());

	// Now, sync the config...
	cfg = kapp->config();
	cfg->setGroup("__Global");
	cfg->writeEntry("ProfileList",profileList,';');
	cfg->sync();

	// Now, write out a simple profile with no useful data EXCEPT version...
	cfg->setGroup(txtProfileName->text().stripWhiteSpace());
	switch (cbPGPVersion->currentItem()) {
		case 0: // GnuPG 1.0x
			cfg->writeEntry("PGPVersion",1);
			break;
		case 1: // PGP 6.5
			cfg->writeEntry("PGPVersion",6);
			break;
		case 2: // PGP 5.0
			cfg->writeEntry("PGPVersion",5);
			break;
		case 3: // PGP 2.6x
			cfg->writeEntry("PGPVersion",2);
			break;
	}
	cfg->sync();

}
