
#include "misc.h"
#include "bdebug.h"
#include "gdebug.h"
#include "defines.h"

// I moved these here so as not to create a loop betwixt kPGP* and keyring and misc...
#include "kPGPPrefs.h"
#include "kPGPKeys.h"
#include "kPGPDecrypt.h"
#include "kPGPEncrypt.h"
#include "kPGPDNDZone.h"
#include "kPGPProfileEdit_New.h"

#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmainwindow.h>
#if QT_VERSION < 300
#include <kapp.h>
#else
#include <kapplication.h>
#endif

#include <qstringlist.h>
#include <qcombobox.h>
#include <qfileinfo.h>
#include <qprogressdialog.h>
#include <qstrlist.h>

#include <stdlib.h>
#include <unistd.h>

const QStringList & getKeyserverList()
{
  static const QStringList keyServers
    = QStringList()
      << "pgpkeys.mit.edu"
      << "wwwkeys.pgp.net"
      << "keys.pgp.com"
      << "pgp5.ai.mit.edu"
      << "pgp.cc.gatech.edu"
      << "pgp.es.net"
      << "www.pgpkeys.wazoo.com"
      << "picard.uni-paderborn.de"
      << "pgp.zdv.uni-mainz.de"
      << "horowitz.surfnet.nl"
      << "levis.uit.no"
      << "keys.pgp.dk"
      << "blackhole.pca.dfn.de"
      << "pgpkeys.ulpgc.es"
      << "wwwkeys.ch.pgp.net"
      << "wp.kfunigraz.ac.at"
      << "pgp.rediris.es"
      << "pgp.lsi.upc.es"
      << "pks.pgp.cz"
      << "sunsite.icm.edu.pl"
      << "ds.carnet.hr"
      << "keys.iif.hu"
      << "lo13.univ.szczecin.pl"
      << "pgp.nic.ad.jp";

  return keyServers;
}


// -----------------------------------------------------------------

static void *myMalloc(size_t size)
{
	void *newPtr;
	newPtr = malloc(size);
	if (!newPtr) {
		fprintf(stderr,"Fatal Error: not able to malloc (%d bytes)!\n",size);
		exit(-2);
	}
	return newPtr;
}

static inline char *getSafeTmpName() {
	char *tmp1 = NULL;

	tmp1 = (char *) myMalloc(1024);
	if (tmp1) {
		snprintf(tmp1,1023,"%s/geheimnis.XXXXXX",getenv("HOME"));
		mkstemp(tmp1);
	}
	return tmp1;
}

// returns a QString equiv of getSafeTmpName
QString getSafeTmpNameQ() {
	char *tmp = getSafeTmpName();
	QString ret(tmp);
	if (tmp) free(tmp);
	return ret;
}

QString extractDisplayName( const QString & stringIn )
{
  int openAngle = stringIn.find('<');
  int closeAngle = stringIn.find( '>', openAngle );
  if ( openAngle >= 0 && closeAngle >= 0 )
    return stringIn.left( openAngle );
  else
    return stringIn;
}

QString extractEmailAddress( const QString & stringIn )
{
  int openAngle = stringIn.find('<');
  int closeAngle = stringIn.find( '>', openAngle );
  if ( openAngle >= 0 && closeAngle >= 0 )
    return stringIn.mid( openAngle + 1, closeAngle - openAngle - 1 );
  else
    return stringIn;
}

int runProcessFG(const QString & pgpAppString, int keepopen)
{
	QString sRunMe, sCmdLine, sTmp;
	int c, loc;
	KConfig *cfg = NULL;

	cfg = kapp->config();
	if (cfg == NULL) return -1; // failed...
	cfg->setGroup("__Global");

	if (keepopen == 1) {
		sCmdLine += cfg->readEntry("PathGeheimnis","geheimnis");
		sCmdLine += " keepopen ";
	}
	sCmdLine += " "; // just to be safe...
	sCmdLine += pgpAppString;

	sRunMe = cfg->readEntry("Terminal","konsole -e %C");

	// Run through finding the %T, if any...
	loc = sRunMe.find("%T",0,FALSE);
	if (loc > -1)
		sRunMe.replace(loc,2,i18n("Geheimnis"));

	// Now, hash through, finding the %C, if any. If none, append to end..
	loc = sRunMe.find("%C",0,FALSE);
	if (loc > -1)
		// remove the location, two chars long, then insert at that point...
		sRunMe.replace(loc,2,sCmdLine);
	else
		sRunMe += sCmdLine;

	bDebug() << "Trying: " << sRunMe << endl;

	c = system(sRunMe.latin1());

	bDebug() << "Result: " << c << endl;

	return c;
}

QWidget *droppedFile(int action, const QString & infile, 
			 const QString & outfile, mySession *theSession) {
	QWidget *a = NULL;
	KConfig * cfg = kapp->config();

	if (action == 6 && infile.isEmpty()) {
	    // if no action specified (action == 0) and no files specified
	    // we check prefs to see if we open key management or not
            if (cfg->readBoolEntry( "AutoKeyManagement", false ))
	      action = 1; // key management window if nothing else...
	}

	if (action == 0) {
		int iStart = infile.findRev('.');
		if (iStart > 0) {
		        // t is used to prevent changing infile
		        QString t = infile.right(infile.length() - iStart).lower();

			if (t.compare("asc") == 0 || t.compare("pgp") == 0)
			    action = 3;
			else if (t.compare("sig") == 0)
			    action = 4;
		}
	}

	if (action == 0) {
		QStringList theList;
		theList.append(infile);
		switch (
			KMessageBox::questionYesNoList(
				NULL,
				i18n("I can't determine if this file is encrypted or not.\nPlease tell me what you want me to do."),
				theList,
				i18n("Geheimnis"),
				i18n("Encrypt/Sign file"),
				i18n("Decrypt/Verify file")
			)
		) {
				case KMessageBox::Yes: // encrypt/sign
					action = 2;
					break;
				case KMessageBox::No: // decrypt/verify
					action = 3;
					break;
		}
	}

	switch (action) {
		case 0:
		case 6: // deprecated, technically, but we still support it, it's easier...
			a = new kPGPDNDZone();
			break;

		case 1: // for key management
			a = new KeyEditorWindow();
			break;

		case 2: // for signing / encrypting files
			a = new kPGPEncrypt(infile, outfile);
			break;

		case 3: // for inline signatures / all encrypted files
			a = new kPGPDecrypt(infile, outfile, false);
			break;

		case 4: // for detached signatures...
			a = new kPGPDecrypt(infile, outfile, true);
			break;
	}
	return a;
}

void cpString(FILE *inFile, char buf[65530])
{
	// DO NOT REPLACE ME, GNUPG NEEDS ME FOR BUFFERING... -- chris
	char ch;
	int i = 0;

	ch = fgetc(inFile);

	while((ch != EOF) && (ch != '\n'))
	{
		buf[i] = ch;
		ch = fgetc(inFile);
		i++;
	}
	buf[i] = '\0';
}

int pipe_pgp(const char *cmd, FILE **in, FILE **out)
{
         int pin[2], pout[2], child_pid;

	 bDebug() << "pipe_pgp: " << cmd << endl;

         *in = *out = NULL;

         pipe(pin);
         pipe(pout);

         if(!(child_pid = fork()))
         {
           // We're the child.
           close(pin[1]);
           dup2(pin[0], 0);
           close(pin[0]);

           close(pout[0]);
           dup2(pout[1], 1);
           close(pout[1]);

           execl("/bin/sh", "sh", "-c", cmd, NULL);
           _exit(127);
         }

         // Only get here if we're the parent.
         close(pout[1]);
         *out = fdopen(pout[0], "r");

         close(pin[0]);
         *in = fdopen(pin[1], "w");

         return(child_pid);
}

int getItemCount(const char *incoming, const char delimiter)
{
	int j = 1;
	uint i = 0;
	for (i=0;i<strlen(incoming);i++) if (incoming[i] == delimiter) j++;
	return j;
}

char *getItem(const char *incoming, const char delimiter, unsigned int fieldNum)
{
	// I support a string as long as an "unsigned long" is big,
	//     and as many fields as an "unsigned int" is big.
	// I return a pointer that must be "free"ed!

	char *tmpString = NULL, *retVal=NULL, *tmpStr = NULL;
	unsigned long l;
	unsigned int thisItem=1;

	tmpString = (char *) myMalloc(strlen(incoming)+2);
	retVal = (char *) myMalloc(strlen(incoming)+2);    // 2 just to be safe.
	strcpy(tmpString,incoming); // so I can modify it... don't need strcpy, it's malloced the strlen(incoming)+2
	strcpy(retVal,"");

	// I only set startChar and endChar on a delimiter so that if the
	// requested item is the 1st item, it starts and ends on item #1.

	for (l=0;l<=strlen(tmpString);l++) {
		if (tmpString[l] == delimiter) {
			thisItem++;
		} else {
			if (thisItem == fieldNum) {
				tmpStr = tmpString+l; // we want to set the "tmpStr" to a particular starting place within tmpString;
				strncat(retVal,tmpStr,1); // now, copy 1 ONE character...
			}
		}
	}

	if (tmpString != NULL) free(tmpString);
	return retVal;
}


void checkPrefs()
{
	/* Note: this function is called from main.c, kPGPKeys.cpp, and
	 * possibly others to verify that we have at least one profile...
	 */
	KConfig *cfg = NULL;
	QString a;
	kPGPPrefs *pe = NULL;

	cfg = kapp->config();
	cfg->setGroup("__Global");

#if 0
	QStrList profileList;

	// Now we add anything new...
	cfg->readListEntry("ProfileList",profileList,';');
	if (profileList.count() == 0) {
		// addDefaultProfiles(cfg);
		KMessageBox::information(NULL,i18n("Welcome to Geheimnis! We will now setup a profile for\nuse with Geheimnis. You can configure as many profiles\nas you want. Usually, most people configure one profile\nfor each encryption program they have installed.\n\nClick OK to configure your first profile."),i18n("Geheimnis"));
		if (!pe) pe = new kPGPPrefs();
		pe->show();
	}
#endif

	cfg->setGroup("__Global");
	
	// Write our revision for future upgrades...
	cfg->setGroup("__Global");
	if (cfg->readNumEntry("Revision",0) < MY_REVISION) {
		// New revision!
		if (KMessageBox::questionYesNo(NULL,i18n("This is the first time you're starting this version of Geheimnis.\nDo you want to look at the ChangeLog?"),i18n("New Revision")) == KMessageBox::Yes)
			kapp->invokeHTMLHelp("geheimnis/changelog.html","");
			// display changelog...
	}

	cfg->writeEntry("Revision",MY_REVISION);

	cfg->sync();
	return;
}

void incrQPD(QProgressDialog *qpd)
{
	qpd->setProgress(qpd->progress() + 1);
	if (qpd->progress() >= qpd->totalSteps()) qpd->setProgress(0);
	qApp->processEvents();
}

void showBadProfileDialog() {
	KMessageBox::error(NULL,i18n("There was an error with the profile you selected. Please select another, or close and re-open this window."));
}

void showBadKeyIDDialog() {
	KMessageBox::error(NULL,i18n("You must select a key to do that!"));
}

void showUnsupportedDialog() {
	KMessageBox::sorry(NULL,i18n("Sorry, but your encryption program does not support that feature."));
}

