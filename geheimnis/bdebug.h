#ifndef GEHEIMNIS_B_DEBUG_H
#define GEHEIMNIS_B_DEBUG_H

#include <kdebug.h>

// allocated from kdelibs/kdecore/kdebug.areas.
// use for backend code
const int backendDebugArea = 100101;

inline kdbgstream bDebug() {
  return kdDebug( backendDebugArea );
}

inline kdbgstream bDebug( bool cond ) {
  return kdDebug( cond, backendDebugArea );
}

inline kdbgstream bWarning() {
  return kdWarning( backendDebugArea );
}

inline kdbgstream bWarning( bool cond ) {
  return kdWarning( cond, backendDebugArea );
}

inline kdbgstream bError() {
  return kdError( backendDebugArea );
}

inline kdbgstream bError( bool cond ) {
  return kdError( cond, backendDebugArea );
}

inline kdbgstream bFatal() {
  return kdFatal( backendDebugArea );
}

inline kdbgstream bFatal( bool cond ) {
  return kdFatal( cond, backendDebugArea );
}

#endif // GEHEIMNIS_B_DEBUG_H
