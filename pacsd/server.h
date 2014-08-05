#ifndef _SERVER_H_
#define _SERVER_H_

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmnet/diutil.h"
#include "core/config.h"

class Server {
private:
	OFCondition cond;

public:
	Server () {};
	~Server();
	T_ASC_Network *Init ();

	struct ServerConfig sc;
	T_ASC_Network *net;
};

class Association {
private:
	OFCondition cond;
	OFCondition Cleanup (T_ASC_Association *assoc);
	char buff[1024]; // received application context name
	std::string hostIP, hostTitle, userTitle, userIP;

public:
	Association () { OFBool opt_rejectWithoutImplementationUID = false; };
	~Association() {};
	//T_ASC_Association *Accept (T_ASC_Network *net);
	void *Connect (T_ASC_Network *net);

	struct ServerConfig sc;
	T_ASC_Association *assoc;
};

#endif
