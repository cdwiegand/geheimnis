// -*- c++ -*-

#ifndef MISC_H
#define MISC_H

#include <qstring.h>
#include <stdio.h>

class QStrList;
class QComboBox;
class KConfig;
class QProgressDialog;
class QWidget;
class QStringList;
class mySession;

// -----------------------------------------------------------------
QString getSafeTmpNameQ();

QString extractDisplayName( const QString & stringIn );
QString extractEmailAddress( const QString & stringIn );

QWidget *droppedFile(int action=0, const QString & infile=QString::null,
			 const QString & outfile=QString::null, mySession *theSession = NULL);

const QStringList & getKeyserverList();

void cpString(FILE *inFile, char buf[65530]);

int pipe_pgp(const char *cmd, FILE **in, FILE **out);
int runProcessFG(const QString & pgpAppString, int keepopen = 1);

int getItemCount(const char *incoming, const char delimiter);
char *getItem(const char *testString, const char delimiter, unsigned int itemNum);

void checkPrefs();

void incrQPD(QProgressDialog *qpd);

// New in 2.0:
void showBadProfileDialog();
void showBadKeyIDDialog();
void showUnsupportedDialog();

#endif

