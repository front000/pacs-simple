#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <sstream>

class PacsdException {
private:
	std::string msg;
	
public:
	PacsdException (std::string str) { msg = str; }
	const char *what () { return msg.c_str (); }
};

#endif
