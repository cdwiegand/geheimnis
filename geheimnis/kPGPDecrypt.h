// -*- c++ -*-

#ifndef KPGPDECRYPT_H
#define KPGPDECRYPT_H

#include "cryptdialogbase.h"

#include <qstring.h>

class QComboBox;
class QCheckBox;
class QRadioButton;
class KURLRequster;

class kPGPDecrypt : public CryptDialogBase {
		Q_OBJECT

	public:
		// public functions
		kPGPDecrypt( const QString & infile=QString::null,
			     const QString & outfile=QString::null,
			     bool detachedSignature=false,
			     QWidget * parent=0, const char * name=0 );

	protected slots:
		void slotOk(); // overridden from KDialogBase
		void slotCheckDetachedSignatureAvailable();

	private:
		void showHideTextlines();

		// Child widgets
		QComboBox     *mProfileCombo;

		KURLRequester *mInFileRequester;
		QRadioButton  *mInClipboardRadio;

		KURLRequester *mOutFileRequester;
		QRadioButton  *mOutClipboardRadio;

		KURLRequester *mSigFileRequester;
		QRadioButton  *mSigClipboardRadio;

		QCheckBox     *mDecryptCheck;
		QCheckBox     *mSignDetachedCheck;
};

#endif  // KPGPDECRYPT_H


