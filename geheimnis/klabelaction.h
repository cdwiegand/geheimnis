
#ifndef KLABELACTION_H
#define KLABELACTION_H

#include <kaction.h>

class QLabel;
class QString;
class QWidget;

/**
 * @short A simple action that makes a label when plugged into a toolbar.
 * @author Marc Mutz <mutz@kde.org> (but code stolen from KonqLabelAction)
 */

class KLabelAction : public KAction
{
  Q_OBJECT
public:
  KLabelAction( const QString & text, QObject * parent=0, const char * name=0 );

  virtual int plug( QWidget * widget, int index=-1 );
  virtual void unplug( QWidget * widget );
  QLabel * label() { return mLabel; }
private:
  QLabel * mLabel;
};


#endif // KLABELACTION_H
