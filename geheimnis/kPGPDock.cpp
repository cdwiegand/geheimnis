
#include "kPGPDock.h"

#include "misc.h"

#include <kwin.h>
#include <kiconloader.h>
#include <klocale.h>
#if QT_VERSION < 300
#include <kapp.h>
#else
#include <kapplication.h>
#endif

#include <qstring.h>
#include <qevent.h>
#include <qpoint.h>
#include <qlabel.h>
#include <qpopupmenu.h>

#include "kPGPDock.moc"

// FIXME move to defines.h
#define M_OPEN_ENCRYPT 1002
#define M_OPEN_DECRYPT 1003
#define M_OPEN_MAINWIN 1004
#define M_EXIT 1000

kPGPDock::kPGPDock()
  : KMainWindow()
{
	QPixmap pm = kapp->iconLoader()->loadIcon("geheimnis",KIcon::NoGroup,22);
	QLabel * label = new QLabel( QString::null, this );
	label->setPixmap( pm );
	
	//setBackgroundPixmap(pm);
	setMaximumSize(16,24);

	KWin::setSystemTrayWindowFor(winId(),winId());
	show();

	// make popup menu
	menu = new QPopupMenu;
	menu->insertItem(i18n("Encrypt Clipboard"), M_OPEN_ENCRYPT);
	menu->insertItem(i18n("Decrypt Clipboard"), M_OPEN_DECRYPT);
	menu->insertSeparator();
	menu->insertItem(i18n("Open Geheimnis"), M_OPEN_MAINWIN);
	menu->insertSeparator();
	menu->insertItem(i18n("Exit"), M_EXIT);

	connect(menu, SIGNAL(activated(int)),
		this, SLOT(menuCallback(int)));
}

void kPGPDock::mouseDoubleClickEvent(QMouseEvent *) {
	menuCallback(M_OPEN_MAINWIN);
}

void kPGPDock::mousePressEvent(QMouseEvent *e) {
	if(e->button() == RightButton) {
		QPoint p;
		p = mapToGlobal(e->pos());
		popupMenu(p);
	}
}

void kPGPDock::menuCallback(int id) {
	QWidget *win = NULL;
	QString sClipboard("--clipboard");
	QString sBlank("");
	switch(id) {
		case M_EXIT:
			kapp->quit();
			break;
		case M_OPEN_MAINWIN:
			win = droppedFile(6,sBlank,sBlank);	
			break;
		case M_OPEN_DECRYPT:
			win = droppedFile(3,sClipboard,sClipboard);
			break;
		case M_OPEN_ENCRYPT:
			win = droppedFile(2,sClipboard,sClipboard);
			break;
	}
	if (win) win->show();
}

void kPGPDock::popupMenu(const QPoint & p) {
	menu->popup(p);
}

