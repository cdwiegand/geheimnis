// -*- c++ -*-

#ifndef KPGPPROFILEEDIT_GPG_H
#define KPGPPROFILEEDIT_GPG_H

#include "profileeditbase.h"

class QPushButton;
class QListView;
class QCheckBox;
class KURLRequester;
class KConfig;

class GPGProfileEdit : public ProfileEditBase {
		Q_OBJECT

	public:
		GPGProfileEdit( QWidget * parent=0, const char * name=0 );

		void clear();

	protected slots:
		void cmdNewKR_clicked();
		void cmdDelKR_clicked();

	protected:
		void reallyLoadProfile( KConfig * config );
		void reallySaveProfile( KConfig * config ) const;

	private:
		// Child widgets
		KURLRequester *txtPGPBin;
		QListView *lstKeyrings;
		QPushButton *cmdNewKR;
		QPushButton *cmdDelKR;
		QCheckBox *chkHonorHTTPProxy;
};

#endif  // KPGPPROFILEEDIT_GPG_H







