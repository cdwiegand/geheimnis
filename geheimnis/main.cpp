
#include "kPGPLicense.h"
#include "misc.h"
#include "defines.h"
#include "kPGPDock.h"
#include "mySession.h"
#include "gdebug.h"

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kconfig.h>

#if QT_VERSION < 300
#include <kapp.h>
#include <kstddirs.h>
#else
#include <kapplication.h>
#include <kstandarddirs.h>
#endif

#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>

static const char *description =
    I18N_NOOP("Geheimnis: A PGP and GnuPG front-end for KDE");

static KCmdLineOptions options[] = {
    { "e", 0, 0 },
    { "encrypt", I18N_NOOP("Encrypt file"), 0 },
    { "s", 0, 0 },
    { "sign", I18N_NOOP("Sign file"), 0 },
    { "d", 0, 0 },
    { "decrypt", I18N_NOOP("Decrypt file"), 0 },
    { "v", 0, 0 },
    { "verify", I18N_NOOP("Verify file"), 0 },
    { "k", 0, 0 },
    { "keys", I18N_NOOP("Open Key Management"), 0 },
    { "i", 0, 0 },
    { "infile <file>", I18N_NOOP("Incoming file"), 0 },
    { "o", 0, 0 },
    { "outfile <file>", I18N_NOOP("Outgoing file"), 0 },
    { "ko", 0, 0 },
    { "keepopen", I18N_NOOP("Internal Use"), 0},
    { "dock", I18N_NOOP("Only show dock menu"), 0}, 
    { "mail", I18N_NOOP("Show Mail instead of OK in dialogs"), 0},
    KCmdLineLastOption
};

/* This is used to prevent zombies */
void sigchld_handler(int) {
    wait(0);
}

int keepopen(int argc, char *argv[]) {
	char c;
	char cmdLine[MAX_COMMAND_LINE_LENGTH];
	int i;
	int cmdLineLen = 0;

	strcpy(cmdLine,""); // init the line..

	for ( i = 2; i < argc; i++ ) {
		if (cmdLineLen < MAX_COMMAND_LINE_LENGTH-1) {
			if (cmdLineLen > 0) {
				strcat(cmdLine," ");
				cmdLineLen++;
			}
			strncat(cmdLine,argv[i],MAX_COMMAND_LINE_LENGTH - cmdLineLen);
			cmdLineLen += strlen(argv[i]);
		}
	}

	printf( "Executing: %s\n\n", cmdLine );
	fflush( stdout );

	i = system( cmdLine );

	printf( "\n" );
	printf( " ------------------------------------------------------------------------ \n" );
	if (i != 0) {
		printf( " WARNING: The encryption program returned error code %i.\n",i);
	}
	printf( " Please close this window when you are done to return to Geheimnis.\n");
	printf( " ------------------------------------------------------------------------ \n" );
	fflush( stdout );
	while ( 1 < 2 ) fscanf( stdin, "%c", &c ); // loop FOREVER....
	
	exit(i);
}


int main(int argc, char *argv[]) {
  // sneak attack: first, are we calling ourselves to keep the window open?
  if (argc > 2) {
    // there must be at least one more command, so min of 3 is good...
    if (strcasecmp(argv[1],"keepopen") == 0) {
      keepopen(argc,argv);
      return 0;
    }
  }
  // okay, resume your normal init

    KAboutData aboutData("geheimnis", I18N_NOOP("Geheimnis"),
			 MY_VERSION, description, KAboutData::License_LGPL,
			 "(c) 2002 Chris Wiegand",0,
			 "http://geheimnis.sourceforge.net/");

    aboutData.addAuthor("Christopher Wiegand", I18N_NOOP("Original author and current maintainer"),
			"cdwiegand@users.sourceforge.net");
    aboutData.addAuthor("Stefan Suchi", I18N_NOOP("Current maintainer"), "suchi@gmx.de");
    aboutData.addAuthor("Marc Mutz", I18N_NOOP("Port to KDE3/Qt3"),
			"mutz@kde.org");
    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions(options); // Add our own options.

    KApplication app;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    // LICENSE_CHECK_START
    // Before anything else, let's check the license (since we're going GUI anyways...)

    /* Please note: The following code (denoted by // LICENSE_CHECK_END
     * May NOT be removed from geheimnis. This check is legally required
     * for Chris Wiegand, the primary author, as he lives in the USA
     * and the CCITA and related laws DO apply to him. Should this check
     * be removed, YOU may be responsible for lawsuits against his person.
     * This software may only be used if the user is REQUIRED, before using
     * ANY part of this program, to agree to the LGPL v2 license FIRST.
     * Circumventing this license check may be against the law. I'm serious.
     * *I* have no desire to live in jail. -- Christopher Wiegand, primary author
     * I'm sorry, I don't like doing this at all, but I have to cover my butt.
     */

    // Okay, first, if we haven't agreed to the license, let's do that...
    KConfig *cfg = kapp->config();
    cfg->setGroup("__Global");
    if (cfg->readEntry("LicenseLGPLRead", "").isEmpty()) {
			kPGPLicense *theDialog = new kPGPLicense();
			theDialog->exec();
			if (theDialog->result() == QDialog::Accepted) {
				cfg->writeEntry("LicenseLGPLRead", "Yes");
				cfg->sync();
			} else {
				return -1; // Didn't accept license, sorry buster!
			}
    }

    // LICENSE_CHECK_END

    // print out the paths we'll look for the images in
    // it seems that most of the time, KDE can't install
    // the images in the right spots... KDE 1 did this FINE... grrr --cdw
    QStringList qsl = kapp->dirs()->resourceDirs("icon");
    for ( QStringList::Iterator it = qsl.begin() ; it != qsl.end() ; ++it )
      gDebug() << "kstddir: " << (*it) << endl;

    // Check arguments for the appropriate action (if any)
    mySession *theSession = new mySession();

    int action = 6;
    if (args->isSet("encrypt") || args->isSet("sign"))
	action = 2;
    if (args->isSet("decrypt") || args->isSet("verify")) {
	if (action != 6)
	    KCmdLineArgs::usage(i18n("Don't specify more than one type of action."));
	action = 3;
    }
    if (args->isSet("keys")) {
	if (action != 6)
	    KCmdLineArgs::usage(i18n("Don't specify more than one type of action."));
	action = 1;
    }
    if (args->isSet("dock")) {
      if (action != 6) 
        KCmdLineArgs::usage(i18n("Don't specify more than one type of action."));
        action = 9;
    }

    if (args->isSet("mail")) theSession->mailApp = TRUE;

    if (args->isSet("outfile") && !args->isSet("infile"))
	KCmdLineArgs::usage(i18n("Give an input file if you give an output file."));

    /* Make sure that zombies don't hold up processes */
    if (signal(SIGCHLD, sigchld_handler) == SIG_ERR) {
	fprintf(stderr, "I'm sorry, but I couldn't start my Zombie Process "
		"Handler. Please see http://geheimnis.sourceforge.net for "
		"assistance.\n");
	return -1;
    }

    checkPrefs();

    QString infile = args->getOption("infile");
    QString outfile = args->getOption("outfile");
    if (args->isSet("dock")) {
      kPGPDock * theDock = new kPGPDock();
      theDock->show();
      app.setMainWidget(theDock);
    } else {
      QWidget * toplevel = droppedFile(action, infile, outfile, theSession);
      toplevel->show();  
      app.setMainWidget(toplevel);
    }

    return app.exec();
}

