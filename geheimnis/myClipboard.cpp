
#include "myClipboard.h"

#include <qapplication.h>
#include <qclipboard.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>

bool myClipboard::saveIntoClipboard( const QString & filename )
{
  QFile file( filename );
  return saveIntoClipboard( file );
}

bool myClipboard::saveIntoClipboard( QFile & file )
{
  if ( file.isReadable() ||
       ( !file.isOpen() && file.open( IO_ReadOnly ) ) ) {
    QTextStream stream( &file );
    QApplication::clipboard()->setText( stream.read() );
    file.close();
    return true;
  } else
    return false;
}

bool myClipboard::saveFromClipboard( const QString & filename )
{
  QFile file( filename );
  return saveFromClipboard( file );
}

bool myClipboard::saveFromClipboard( QFile & file )
{
  if ( file.isReadable() ||
       ( !file.isOpen() && file.open( IO_WriteOnly ) ) ) {
    QTextStream stream( &file );
    stream << QApplication::clipboard()->text();
    file.close();
    return true;
  } else
    return false;
}

