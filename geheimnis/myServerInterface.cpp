
#include "myServerInterface.h"

#include "bdebug.h"
#include "misc.h"

#include <kconfig.h>
#if QT_VERSION < 300
#include <kapp.h>
#else
#include <kapplication.h>
#endif

#include <qstring.h>

#include <stdlib.h>

void myServerInterface::downloadKey(const QString & keyID, const QString & outputFile) {
	QString appString;
	QString keyServer("pgp.mit.edu");
	QString tmp;
	KConfig *cfg = NULL;

	cfg = kapp->config();
	if (cfg) {
		tmp = cfg->readEntry("Keyserver");
		if (tmp.length() > 3) keyServer = tmp;
	}

	appString = QString("curl -f -s -o \"%1\" http://%2:11371/pks/lookup?search=%3&op=get").arg(outputFile).arg(keyServer).arg(keyID);
	runProcessFG(appString,0);
}


// The following is UNTESTED AND may be BROKEN
void myServerInterface::listKeys(const QString & searchString) {
	FILE *inFile, *outFile; // DO NOT FREE ME (I don't think so, that is. )
	char buf[65530]; // FIXME: Un-hard code these if possible.
	char *tmpBuf = NULL;
                                            	QString appString;
	QString keyServer("pgp.mit.edu");
	QString tmp;
	KConfig *cfg = NULL;

	cfg = kapp->config();
	if (cfg) {
		tmp = cfg->readEntry("Keyserver");
		if (tmp.length() > 3) keyServer = tmp;
	}

	appString = QString("curl -f -s http://%1:11371/pks/lookup?search=%2&op=index").arg(keyServer).arg(searchString);

	pipe_pgp(appString.latin1(),&inFile,&outFile);
	//qApp->processEvents();

	if (outFile == NULL) return;
	while (!feof(outFile)) {
		//qApp->processEvents();
		fgets(buf,65530,outFile);

		bDebug() << "Curl In: '" << buf << "'" << endl;

		tmpBuf = (char *) malloc(5);
		strncpy(tmpBuf,buf,3);
		if (strncmp(tmpBuf,"pub",3) == 0) {
			// found a line we want!
			// FIXME do something with the line...
		}
	}
	fclose(outFile);
}



