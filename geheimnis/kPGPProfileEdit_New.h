// -*- c++ -*-

#ifndef KPGPPROFILEEDIT_NEW_H
#define KPGPPROFILEEDIT_NEW_H

#include <kdialogbase.h>

class QComboBox;
class QLineEdit;

class NewProfileDialog : public KDialogBase {
		Q_OBJECT

	public:
		NewProfileDialog( QWidget * parent=0, const char * name=0 );
		QString profileName() const;

	protected slots:
		void slotOk();
		void cbPGPVersion_clicked();

	protected:
		void saveConfig();

	private:
		QComboBox *cbPGPVersion;
		QLineEdit *txtProfileName;
};

#endif  // KPGPPROFILEEDIT_NEW_H






