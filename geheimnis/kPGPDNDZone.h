// -*- c++ -*-

#ifndef KPGPDNDZONE_H
#define KPGPDNDZONE_H

#include <kmainwindow.h>

class QDragEnterEvent;
class QDropEvent;

class kPGPDNDZone : public KMainWindow {
        Q_OBJECT

	public:
		kPGPDNDZone( QWidget * parent=0, const char * name=0 );

        protected slots:
		void dragEnterEvent(QDragEnterEvent *qevent);
		void dropEvent(QDropEvent *e);
		void encrypt_clicked();
		void decrypt_clicked();
		void keys_clicked();
		void prefs_clicked();

	protected:
  bool queryClose(); // overridden from KMMainWindow
};

#endif


