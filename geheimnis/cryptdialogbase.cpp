
#include "cryptdialogbase.h"

#include "gdebug.h"

#include <klocale.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#if QT_VERSION < 300
#  include <kpropsdlg.h>
#else
#  include <kpropertiesdialog.h>
#endif

#include <qstring.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <qhbox.h>

#include "cryptdialogbase.moc"

CryptDialogBase::CryptDialogBase( const QString & caption,
				  QWidget * parent, const char * name )
  : KDialogBase( Plain, caption, Help|Ok|Close, Ok,
		 parent, name, false /*not modal*/ )
{

}

QButtonGroup *
CryptDialogBase::createClipOrFileGroup( const QString & title,
					const QString & useClipText,
					const QString & fileText,
					QRadioButton *& rRadio,
					KURLRequester *& rRequester,
					const QString & fileName,
					QWidget * parent,
					const char * name )
{
  QButtonGroup * group = new QButtonGroup( 1, Qt::Vertical, title,
					   parent, name );
  group->layout()->setSpacing( spacingHint() );
  QHBox * hbox = new QHBox( group );
  hbox->setSpacing( spacingHint() );

  QRadioButton * fradio = new QRadioButton( fileText, hbox );
  group->insert( fradio, 0 );
  rRequester = new KURLRequester( hbox );
  hbox->setStretchFactor( rRequester, 1 );
  rRadio = new QRadioButton( useClipText, group );
  group->insert( rRadio, 1 ); // just in case...

  if ( fileName.lower() == "--clipboard" ) {
    rRequester->setEnabled( false );
    rRadio->setChecked( true );
  } else {
    fradio->setChecked( true );
    rRequester->setURL( fileName );
  }

  // disable requester when not checked:
  connect( fradio, SIGNAL(toggled(bool)),
	   rRequester, SLOT(setEnabled(bool)) );

  return group;
}

bool CryptDialogBase::sanityCheckInFile( bool & clipboard,
					 QRadioButton * clipRadio,
					 KURLRequester * inFileRequester )
{
  // clipboard is always Ok:
  if ( clipboard ) return true;

  // on empty filename: ask if the user meant clipboard instead:
  if ( inFileRequester->url().isEmpty() ) {
    QString msg = i18n("You chose to read input from a file,\n"
		       "but didn't give a filename.\n"
		       "\n"
		       "Read input from clipboard instead?");
    switch ( KMessageBox::questionYesNo( this, msg ) ) {
    default:
    case KMessageBox::Yes:
      clipboard = true;
      clipRadio->setChecked( true );
      return true;
    case KMessageBox::No:
      inFileRequester->setFocus();
      return false;
    }
  }

  QFileInfo inFile( inFileRequester->url() );

  if ( !inFile.exists() ) {
    QString msg = i18n("You chose to read input from\n"
		       "\"%1\".\n"
		       "This file does not exist.\n"
		       "\n"
		       "Read input from clipboard instead?")
      .arg( inFileRequester->url() );
    switch ( KMessageBox::questionYesNo( this, msg ) ) {
    default:
    case KMessageBox::Yes:
      clipboard = true;
      clipRadio->setChecked( true );
      return true;
    case KMessageBox::No:
      inFileRequester->setFocus();
      return false;
    }
  }

  if ( !inFile.isReadable() ) {
    QString msg = i18n("You chose to read input from\n"
		       "\"%1\".\n"
		       "You do not have permission to read from this file.\n"
		       "\n"
		       "View and change permissions?")
      .arg( inFileRequester->url() );
    switch ( KMessageBox::questionYesNo( this, msg ) ) {
    default:
    case KMessageBox::Yes: {
      // ### experiment:
      KPropertiesDialog * dlg
	= new KPropertiesDialog( inFileRequester->url(),
#if QT_VERSION < 300
				 (mode_t)-1 /*unused*/,
#endif
				 this, 0 /*name*/, true /*modal*/ );
      dlg->exec();
      inFile.refresh();
      if ( inFile.isReadable() ) return true;
    } // fall through:
    case KMessageBox::No:
      inFileRequester->setFocus();
      return false;
    }
  }
  
  return true;
}

bool CryptDialogBase::sanityCheckOutFile( bool & clipboard,
					  QRadioButton * ,
					  KURLRequester * outFileRequester )
{
  if ( clipboard ) return true;

  gWarning( outFileRequester->url().isEmpty() )
    << "OOPS, someone forgot to do his own checks!" << endl;

  QFileInfo outFile( outFileRequester->url() );

  if ( outFile.exists() ) {
    QString msg = i18n("You chose to write output to\n"
		       "\"%1\".\n"
		       "This file does already exist.\n"
		       "\n"
		       "Overwrite?")
      .arg( outFileRequester->url() );
    switch ( KMessageBox::questionYesNo( this, msg ) ) {
    default:
    case KMessageBox::Yes:
      return true;
    case KMessageBox::No:
      outFileRequester->setFocus();
      return false;
    }
  }

  QDir outFileDir = outFile.dir();

  // check if the dir exists:
  if ( !outFileDir.exists() ) {
    QString msg = i18n("You chose to write output to\n"
		       "\"%1\".\n"
		       "That directory does not exist.\n"
		       "\n"
		       "Create?")
      .arg( outFileRequester->url() );
    switch( KMessageBox::questionYesNo( this, msg ) ) {
    default:
    case KMessageBox::Yes:
      gDebug() << "not yet implemented!" << endl;
      return false;
    case KMessageBox::No:
      outFileRequester->setFocus();
      return false;
    }
  }

  // try creating:
  QFile file( outFileRequester->url() );
  if ( !file.open( IO_WriteOnly ) ) {
    KMessageBox::sorry( this, i18n("Could not create output file.\n"
				   "Please choose another one.") );
    outFileRequester->setFocus();
  }

  file.remove();
  return true;
}

