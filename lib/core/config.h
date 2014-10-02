#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <string>

typedef struct ServerConfig {
	std::string	version;
	std::string	aetitle;
	std::string savepath;
	int			port;
	int			timeout;
	int			maxreq;
	bool			aecheck;
} conf;

class Config {
private:
	ServerConfig initDefault ();
	struct ServerConfig sc;

public:
	Config () {};
	~Config() {};

	ServerConfig Load (const char *filename);
	void Show ();
};

#endif
