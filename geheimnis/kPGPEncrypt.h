// -*- c++ -*-

#ifndef KPGPENCRYPT_H
#define KPGPENCRYPT_H

#include "cryptdialogbase.h"

#include <qstring.h>
#include <qstringlist.h>

class kPGPKeyList;
class Profile;
class QComboBox;
class QCheckBox;
class KURLRequester;
class QRadioButton;

class kPGPEncrypt : public CryptDialogBase {
		Q_OBJECT

	public:
		// public functions
		kPGPEncrypt( const QString & infile=QString::null,
			     const QString & outfile=QString::null,
			     QWidget * parent=0, const char * name=0 );
		virtual ~kPGPEncrypt();

	protected:
		bool sanityCheckOutFile( bool & clipboard,
					 QRadioButton * radio=0,
					 KURLRequester * requester=0 );

	protected slots:
		void slotOk(); // overridden from KDialogBase
		void slotCheckDetachedSigPossible();
		void slotEnableOkButton();
		void slotProfileSelected( int listIndex );

	private:
		// Child widgets
		QComboBox    *mProfileCombo;
		KURLRequester *mInFileRequester;
		KURLRequester *mOutFileRequester;
		QRadioButton *mInClipboardRadio;
		QRadioButton *mOutClipboardRadio;

		QCheckBox    *mSignCheck;
		QComboBox    *mSignCombo;
		QRadioButton *mSignDetachedRadio;
		QRadioButton *mSignClearRadio;
		QCheckBox    *mSignTextmodeCheck;

		QCheckBox    *mEncryptCheck;
		kPGPKeyList  *mEncryptToList;
		QCheckBox    *mAsciiArmorCheck;
		QCheckBox    *mEncryptToSelfCheck;

		QStringList   qlistSecKeyIDs;
		Profile    *mCurrentProfile;
};

#endif  // KPGPENCRYPT_H


