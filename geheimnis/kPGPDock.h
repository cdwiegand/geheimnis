
// -*- c++ -*-

#ifndef GEHEIMNISDOCKED_H
#define GEHEIMNISDOCKED_H

#include <kmainwindow.h>

class QMouseEvent;
class QPoint;
class QPopupMenu;

class kPGPDock : public KMainWindow {
        Q_OBJECT
                 
	public:
		kPGPDock();

	public slots:
		void mousePressEvent(QMouseEvent *e);
		void mouseDoubleClickEvent(QMouseEvent *e);
		void menuCallback(int id);
		void popupMenu(const QPoint & p);

	private:
		QPopupMenu *menu;
};

#endif

