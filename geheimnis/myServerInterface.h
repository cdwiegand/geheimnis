
#ifndef MYSERVERINTERFACE_H
#define MYSERVERINTERFACE_H

class QString;

class myServerInterface {
	public:
		void downloadKey(const QString & keyID, const QString & outputFile);
		void listKeys(const QString & searchString);
};

#endif


