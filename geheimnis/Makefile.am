####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = geheimnis

## INCLUDES were found outside kdevelop specific part

geheimnis_SOURCES = profilemanager.cpp profileeditbase.cpp mySession.cpp myServerInterface.cpp myProfile.cpp myKeyring.cpp myClipboard.cpp misc.cpp main.cpp kPGPSelectKey.cpp kPGPProfileEdit_PGP6.cpp kPGPProfileEdit_PGP5.cpp kPGPProfileEdit_PGP2.cpp kPGPProfileEdit_New.cpp kPGPProfileEdit_GPG.cpp kPGPPrefs.cpp kPGPLicense.cpp kPGPKeys.cpp kPGPKeyProps.cpp kPGPKeyList.cpp kPGPKeyExport.cpp kPGPKeyAdd.cpp kPGPEncrypt.cpp kPGPDock.cpp kPGPDNDZone.cpp kPGPDecrypt.cpp klabelaction.cpp dummy.cpp cryptdialogbase.cpp be_pgp6.cpp be_pgp5.cpp be_pgp2.cpp be_gpg.cpp 
geheimnis_LDADD   =  $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET)

SUBDIRS = icons 

EXTRA_DIST = geheimnisui.rc geheimnis.desktop bdebug.h be_base.h be_gpg.cpp be_gpg.h be_pgp2.cpp be_pgp2.h be_pgp5.cpp be_pgp5.h be_pgp6.cpp be_pgp6.h cryptdialogbase.cpp cryptdialogbase.h defines.h dummy.cpp gdebug.h geheimnisdndzoneui.rc geheimnis-dock.desktop geheimniskeyeditorui.rc hi16-app-geheimnis.xpm hi22-app-geheimnis.xpm hi32-app-geheimnis.xpm klabelaction.cpp klabelaction.h kPGPDecrypt.cpp kPGPDecrypt.h kPGPDNDZone.cpp kPGPDNDZone.h kPGPDock.cpp kPGPDock.h kPGPEncrypt.cpp kPGPEncrypt.h kPGPKeyAdd.cpp kPGPKeyAdd.h kPGPKeyExport.cpp kPGPKeyExport.h kPGPKeyList.cpp kPGPKeyList.h kPGPKeyProps.cpp kPGPKeyProps.h kPGPKeys.cpp kPGPKeys.h kPGPLicense.cpp kPGPLicense.h kPGPPrefs.cpp kPGPPrefs.h kPGPProfileEdit_GPG.cpp kPGPProfileEdit_GPG.h kPGPProfileEdit_New.cpp kPGPProfileEdit_New.h kPGPProfileEdit_PGP2.cpp kPGPProfileEdit_PGP2.h kPGPProfileEdit_PGP5.cpp kPGPProfileEdit_PGP5.h kPGPProfileEdit_PGP6.cpp kPGPProfileEdit_PGP6.h kPGPSelectKey.cpp kPGPSelectKey.h main.cpp misc.cpp misc.h myClipboard.cpp myClipboard.h myKeyring.cpp myKeyring.h myProfile.cpp myProfile.h myServerInterface.cpp myServerInterface.h mySession.cpp mySession.h profileeditbase.cpp profileeditbase.h profilemanager.cpp profilemanager.h 

install-data-local:
	$(mkinstalldirs) $(kde_appsdir)/Applications/
	$(INSTALL_DATA) $(srcdir)/geheimnis.desktop $(kde_appsdir)/Applications/geheimnis.desktop
	$(mkinstalldirs) $(kde_icondir)/
	$(INSTALL_DATA) $(srcdir)/hi16-app-geheimnis.xpm $(kde_icondir)/hi16-app-geheimnis.xpm
	$(mkinstalldirs) $(kde_icondir)/
	$(INSTALL_DATA) $(srcdir)/hi22-app-geheimnis.xpm $(kde_icondir)/hi22-app-geheimnis.xpm
	$(mkinstalldirs) $(kde_icondir)/
	$(INSTALL_DATA) $(srcdir)/hi32-app-geheimnis.xpm $(kde_icondir)/hi32-app-geheimnis.xpm

uninstall-local:
	-rm -f $(kde_appsdir)/Applications/geheimnis.desktop
	-rm -f $(kde_icondir)/hi16-app-geheimnis.xpm
	-rm -f $(kde_icondir)/hi22-app-geheimnis.xpm
	-rm -f $(kde_icondir)/hi32-app-geheimnis.xpm

####### kdevelop will overwrite this part!!! (end)############
# These paths are KDE specific. Use them:
# kde_appsdir         Where your application's menu entry (.desktop) should go to.
# kde_icondir         Where your icon should go to - better use KDE_ICON.
# kde_sounddir        Where your sounds should go to.
# kde_htmldir         Where your docs should go to. (contains lang subdirs)
# kde_datadir         Where you install application data. (Use a subdir)
# kde_locale          Where translation files should go to. (contains lang subdirs)
# kde_cgidir          Where cgi-bin executables should go to.
# kde_confdir         Where config files should go to (system-wide ones with default values).
# kde_mimedir         Where mimetypes .desktop files should go to.
# kde_servicesdir     Where services .desktop files should go to.
# kde_servicetypesdir Where servicetypes .desktop files should go to.
# kde_toolbardir      Where general toolbar icons should go to (deprecated, use KDE_ICON).
# kde_wallpaperdir    Where general wallpapers should go to.
# kde_templatesdir    Where templates for the "New" menu (Konqueror/KDesktop) should go to.
# kde_bindir          Where executables should go to. Use bin_PROGRAMS or bin_SCRIPTS.
# kde_libdir          Where shared libraries should go to. Use lib_LTLIBRARIES.
# kde_moduledir       Where modules (e.g. parts) should go to. Use kde_module_LTLIBRARIES.
# kde_styledir        Where Qt/KDE widget styles should go to (new in KDE 3).
# kde_designerdir     Where Qt Designer plugins should go to (new in KDE 3).

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

METASOURCES = AUTO

# the library search path. 
geheimnis_LDFLAGS = $(all_libraries) $(KDE_RPATH)

rcdir = $(kde_datadir)/geheimnis
rc_DATA = geheimnisui.rc

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/geheimnis.pot; \
	fi

