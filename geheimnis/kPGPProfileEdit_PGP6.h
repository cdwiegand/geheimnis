// -*- c++ -*-

#ifndef KPGPPROFILEEDIT_PGP6_H
#define KPGPPROFILEEDIT_PGP6_H

#include "profileeditbase.h"

class KConfig;
class KURLRequester;

class PGP6ProfileEdit : public ProfileEditBase {
		Q_OBJECT

	public:
		// public functions
		PGP6ProfileEdit( QWidget * parent=0, const char * name=0 );

		void clear();

	protected:
		void reallyLoadProfile( KConfig * config );
		void reallySaveProfile( KConfig * config ) const;

	private:
		// Child widgets
		KURLRequester *txtPGPBin;
		KURLRequester *txtPublicKR;
		KURLRequester *txtSecretKR;
};

#endif  // KPGPPROFILEEDIT_PGP6_H





