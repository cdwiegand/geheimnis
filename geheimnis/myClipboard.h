
// -*- c++ -*-

#ifndef MYCLIPBOARD_H
#define MYCLIPBOARD_H

class QString;
class QFile;

class myClipboard {
protected:
  myClipboard() {} // protected c'tor

public:
  static bool saveIntoClipboard( const QString & filename );
  static bool saveFromClipboard( const QString & filename );

  static bool saveIntoClipboard( QFile & file );
  static bool saveFromClipboard( QFile & file );
};

#endif


