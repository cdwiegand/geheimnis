// -*- c++ -*-

#ifndef GEHEIMNIS_CRYPTDIALOGBASE_H
#define GEHEIMNIS_CRYPTDIALOGBASE_H

#include <kdialogbase.h>
#include <kurlrequester.h>

class QString;
class QRadioButton;
class KURLRequester;
class QButtonGroup;

/**
 * This class contains a few helpers that are common to the two
 * dialogs.
 *
 * @short Base class for encrypt and decrypt dialogs.
 * @author Marc Mutz <mutz@kde.org>
 */

class CryptDialogBase : public KDialogBase {
  Q_OBJECT
public:
  CryptDialogBase( const QString & caption,
		   QWidget * parent=0, const char * name=0 );

protected:
  /** Do (too many :-) checks on the infile */
  virtual bool sanityCheckInFile( bool & clipboard, QRadioButton * clipRadio,
				  KURLRequester * inFileRequester );
  /** Do (too many :-) checks on the outfile */
  virtual bool sanityCheckOutFile( bool & clipboard, QRadioButton * clipRadio,
				   KURLRequester * outFileRequester );

  /** Creates a QButtonGroup with parent @p parent and internal name
      @p name, containing two radio buttons (one of which is returned
      in @p rRadio) and a URLRequseter (which is returned in @p
      rRequester). To avoid accel collisions, you can also set the
      texts used. @p fileName contains the initial filename.
  */
  QButtonGroup * createClipOrFileGroup( const QString & title,
					const QString & useClipText,
					const QString & fileText,
					QRadioButton *& rRadio,
					KURLRequester *& rRequester,
					const QString & fileName,
					QWidget * parent,
					const char * name=0 );
};


#endif // GEHEIMNIS_CRYPTDIALOGBASE_H
