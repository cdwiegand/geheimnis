#ifndef GEHEIMNIS_G_DEBUG_H
#define GEHEIMNIS_G_DEBUG_H

#include <kdebug.h>

// allocated from kdelibs/kdecore/kdebug.areas
const int debugArea = 100100;

inline kdbgstream gDebug() {
  return kdDebug( debugArea );
}

inline kdbgstream gDebug( bool cond ) {
  return kdDebug( cond, debugArea );
}

inline kdbgstream gWarning() {
  return kdWarning( debugArea );
}

inline kdbgstream gWarning( bool cond ) {
  return kdWarning( cond, debugArea );
}

inline kdbgstream gError() {
  return kdError( debugArea );
}

inline kdbgstream gError( bool cond ) {
  return kdError( cond, debugArea );
}

inline kdbgstream gFatal() {
  return kdFatal( debugArea );
}

inline kdbgstream gFatal( bool cond ) {
  return kdFatal( cond, debugArea );
}

#endif // GEHEIMNIS_G_DEBUG_H
