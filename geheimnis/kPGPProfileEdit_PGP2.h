// -*- c++ -*-

#ifndef KPGPPROFILEEDIT_PGP2_H
#define KPGPPROFILEEDIT_PGP2_H

#include "profileeditbase.h"

class KConfig;
class KURLRequester;

class PGP2ProfileEdit : public ProfileEditBase {
		Q_OBJECT

	public:
		// public functions
		PGP2ProfileEdit( QWidget * parent=0, const char * name=0 );

		void clear();

	protected:
		void reallyLoadProfile( KConfig * config );
		void reallySaveProfile( KConfig * config ) const;

	private:
		// Child widgets
		KURLRequester *txtPGPBin;
};

#endif  // KPGPPROFILEEDIT_PGP2_H





