
#include "kPGPLicense.h"

#include <klocale.h>
#include <kaboutdata.h>
#if QT_VERSION < 300
#include <kapp.h>
#else
#include <kapplication.h>
#endif

#include <qtextview.h>
#include <qlayout.h>
#include <qlabel.h>

#include "kPGPLicense.moc"

kPGPLicense::kPGPLicense( QWidget * parent, const char * name )
  : KDialogBase( Plain, i18n("License Agreement"), Ok|Cancel, Ok,
		 parent, name )
{
  QVBoxLayout * vlay = new QVBoxLayout( plainPage(), 0, spacingHint() );

  // row 0: label
  QString msg = i18n("Please read the following license agreement.\n"
		     "If you do not agree with the following license, "
		     "you may not use Geheimnis.");
  vlay->addWidget( new QLabel( msg, plainPage() ) );

  // row 1: text view containing the license
  QTextView * licenseView = new QTextView( plainPage() );
  licenseView->setMinimumSize( 600, 400 );

  // load the license into the viewer:
  QString license = i18n("<qt><p>In order to conform to the UCITA and similar "
			 "laws, you must agree to the following license "
			 "agreement. If you do not, you may not use "
			 "Geheimnis.</p>"
			 "<p>Sorry it had to come to this, but I have to "
			 "cover <em>my</em> butt too.</p>"
			 "<p>Chris Wiegand</p><hr>"
			 "<pre>\n"
			 "%1\n"
			 "</pre></qt>").arg( kapp->aboutData()->license() );
  licenseView->setText( license );

  vlay->addWidget( licenseView, 1 ); // stretch is here

  // nice text for the buttons:
  setButtonOKText( i18n("I &agree") );
  setButtonCancelText( i18n("I do &not agree") );
}
