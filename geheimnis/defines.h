// Not all of these may actually be in use! --chris

#ifndef DEFINES_H
#define DEFINES_H

#define DEBUG
#define MY_DEBUG
#define MYSPACING 5

// The following is how big of a string it allocates for the command line...
#define MAX_COMMAND_LINE_LENGTH 4096

// And this is how long the email address can be, it is 1024 for PGP5, I think...
#define EMAIL_ADDRESS_MAX_LENGTH 1024

// This is how long a returned line from PGP/GPG can be as far as interactive piping...
#define CONSOLE_IO_LINE_LENGTH 256

// This is how big a key id & type from gpg/pgp is
#define KEY_ID_LENGTH 64
#define KEY_TYPE_LENGTH 10
#define KEY_SIZE_LENGTH 8
#define KEY_FP_LENGTH 128

#define GPG_NAME "GnuPG"
#define PGP2_NAME "PGP 2.6"
#define PGP5_NAME "PGP 5.0"
#define PGP6_NAME "PGP 6.5"
#define PGP7_NAME "PGP 7.0"
// um... PGP7 hopefully not used yet.. --cdw

#define PROFILE_NAME_LEN 256

// When you change MY_VERSION, change it in ABOUT_THIS_PROGRAM too.
#define MY_REVISION 198
#define MY_APP_NAME "Geheimnis 2.0 Beta build 198"
#define MY_VERSION "2.0beta (v1.98)"
#define MYAPPDIR "geheimnis"

class pkiSig;
class pkiName;
class pkiSubKey;
class pkiKey;
class pkiKeyring;
class Profile;

#endif

