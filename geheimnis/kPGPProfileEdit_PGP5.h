// -*- c++ -*-

#ifndef KPGPPROFILEEDIT_PGP5_H
#define KPGPPROFILEEDIT_PGP5_H

#include "profileeditbase.h"

class KConfig;
class KURLRequester;

class PGP5ProfileEdit : public ProfileEditBase {
		Q_OBJECT

	public:
		// public functions
		PGP5ProfileEdit( QWidget * parent=0, const char * name=0 );

		void clear();

	protected:
		void reallyLoadProfile( KConfig * config );
		void reallySaveProfile( KConfig * config ) const;

	private:
		// Child widgets
		KURLRequester *txtPGPBinK;
		KURLRequester *txtPGPBinE;
		KURLRequester *txtPGPBinS;
		KURLRequester *txtPGPBinV;
		KURLRequester *txtPublicKR;
		KURLRequester *txtSecretKR;
};

#endif  // KPGPPROFILEEDIT_PGP5_H





