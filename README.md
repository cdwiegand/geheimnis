# Ancient webpage introducing the program

This software is OSI Certified Open Source Software.
OSI Certified is a certification mark of the Open Source Initiative.

Download the KDE 3 CVS tarball here (dated 2002-Nov-03).

## Prerequisites

Geheimnis is compatible with PGP 6.5, PGP 5.0, PGP 2.6, and GnuPG 1.0x.

Geheimnis is available for KDE 1, KDE 2.2x, and KDE 3.x.

If you wish to download RPMs, you can find some on rpmfind.net, or from sourceforge's download site.

## Building (optional)

If you want to build Geheimnis from the source code, you will need one of the three following sets:

KDE 1.x libraries and headers, and QT 1.4x libraries and headers,
KDE 2.x libraries and headers, and QT 2.x libraries and headers,
KDE 3.x libraries and headers, and QT 3.x libraries and headers,
To compile the KDE-based versions, type:
``
make -f Makefile.dist # (only if CVS version)
./configure
make
make install
```

## Installing

To install the KDE versions built from source code, type make install. To install the KDE versions from the binaries (rpm, deb, etc.), use your computer's package installer (RedCarpet, rpm -i geheimnis.rpm, dpkg -i geheimnis.deb, etc.)

Use it! Tell me!

If it doesn't work, please join the mailing list, Anyone can subscribe.

# Ancient CVS instructions

If you run a CVS version, run
  make -f Makefile.dist ; ./configure ; make ; make install

Note: This is for KDE 3.x only! KDE 2 will NOT compile this!

If you compile from a tar ball, run only
  ./configure ; make ; make install

Be sure to set your QTDIR and KDEDIR accordingly:
If you use sh/bash:
  export KDEDIR=/path/to/kde/dir
  export QTDIR=/path/to/qt/dir
If you use csh/tcsh:
  setenv KDEDIR /path/to/kde/dir
  setenv QTDIR /path/to/qt/dir

http://geheimnis.sourceforge.net/

