// -*- c++ -*-
#ifndef KPGPPREFS_H
#define KPGPPREFS_H

#include <kdialogbase.h>

class KURLRequester;
class QString;
class QComboBox;
class QLineEdit;
class QCheckBox;
class QPushButton;
class QWidgetStack;
class GPGProfileEdit;
class PGP2ProfileEdit;
class PGP5ProfileEdit;
class PGP6ProfileEdit;

class kPGPPrefs : public KDialogBase {
		Q_OBJECT

	public:
		kPGPPrefs( QWidget * parent=0, const char * name=0 );

	protected slots:
		void cmdNewP_clicked();
		void cmdRemoveP_clicked();
		void cbPGPVersion_activated(const QString &a);
		void chkDefault_clicked();
		void slotOk(); // overridden from KDialogBase

	protected:
		void updateList();
		void selectProfile(const QString & profileName);
		void saveChanges();

	private:
		// Page 1
		QComboBox       *cmbTerminal;
		QLineEdit       *txtComment;
		QCheckBox       *chkKeyTreeView;
		KURLRequester   *cmbPKO;
		QCheckBox       *chkUseCurl;
		QComboBox       *cboKeyserver;
		QCheckBox       *chkAutoKeyManagement;

		// Page 2
		QComboBox       *cbPGPVersion;
		QCheckBox       *chkDefault;
		QPushButton     *cmdNewP;
		QPushButton     *cmdRemoveP;
		QWidgetStack    *mWidgetStack;
};

#endif
