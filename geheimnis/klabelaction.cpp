
#include "klabelaction.h"

#include <ktoolbar.h>

#include <qlabel.h>
#include <qstring.h>
#include <qwidget.h>

#include "klabelaction.moc"

KLabelAction::KLabelAction( const QString & text,
			    QObject * parent, const char * name )
  : KAction( text, 0, parent, name ), mLabel( 0 )
{
}

int KLabelAction::plug( QWidget * widget, int index )
{
  //do not call the previous implementation here

  KToolBar * tb = dynamic_cast<KToolBar*>(widget);
  if ( !tb ) return -1;

  int id = KAction::getToolButtonID();

  mLabel = new QLabel( text(), widget );
  tb->insertWidget( id, mLabel->width(), mLabel, index );

  addContainer( tb, id );

  connect( tb, SIGNAL(destroyed()), this, SLOT(slotDestroyed()) );

  return containerCount() - 1;
}

void KLabelAction::unplug( QWidget * widget )
{
  if ( widget->inherits( "KToolBar" ) ) {
    KToolBar * tb = static_cast<KToolBar*>(widget);
    if ( !tb ) return;

    int idx = findContainer( tb );

    if ( idx != -1 ) {
#if QT_VERSION < 300
      tb->removeItem( menuId( idx ) );
#else
      tb->removeItem( itemId( idx ) );
#endif
      removeContainer( idx );
    }

    mLabel = 0;
  }
}

