
// -*- c++ -*-

#ifndef KPGPKEYPROPS_H
#define KPGPKEYPROPS_H

#include <kdialogbase.h>

class pkiKey;

class KeyPropertiesDialog : public KDialogBase {
  Q_OBJECT

public:
  KeyPropertiesDialog( const pkiKey * myKey,
		       QWidget * parent=0, const char * name=0 );

};

#endif  // KPGPKEYPROPS_H


