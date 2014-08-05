#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

class PacsdException {
private:
	std::string msg;
	
public:
	PacsdException (std::string str) { msg = str; }
	const char *what () { return msg.c_str (); }
};

#endif
