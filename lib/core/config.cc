#include "config.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem.hpp>
#include <iomanip> // for std::setw ()
#include <sstream> // for stringstream

ServerConfig Config::initDefault () {
	std::cout << "I: Config was not defined, used default values" << std::endl;
	sc.version	= "1.1";
	sc.aetitle	= "PACS-SIMPLE";
	sc.maxreq	= 0; // unlimited
	sc.port		= 1111;
	sc.timeout	= 30;
	sc.aecheck	= true;

	return sc;
}

ServerConfig Config::Load (const char *filename) {
	try {
		boost::filesystem::exists (filename);

		boost::property_tree::ptree pt;
		boost::property_tree::ini_parser::read_ini (filename, pt);

		sc.version	= pt.get<std::string>("main.version", "1.1");
		sc.aetitle	= pt.get<std::string>("main.aetitle", "PACS-SIMPLE");
		sc.maxreq	= pt.get<int>("main.maxreq");
		sc.port		= pt.get<int>("main.port", 1111);
		sc.timeout	= pt.get<int>("main.timeout", 30);
		sc.aecheck	= pt.get<bool>("pacs.aecheck", true);

	}
	catch (std::exception &e) {
		sc = initDefault ();
	}

	return sc;
}

void Config::Show () {
	std::cout << "Loaded configuration:" << std::endl;
	std::cout << std::setw(30) << "version: "	<< sc.version	<< std::endl;
	std::cout << std::setw(30) << "AETitle: "	<< sc.aetitle	<< std::endl;
	std::cout << std::setw(30) << "port: "		<< sc.port		<< std::endl;
	std::cout << std::setw(30) << "timeout: "	<< sc.timeout	<< std::endl;

	std::stringstream ss;
	ss << sc.maxreq;
	std::string reqlimit = ss.str ();
	if (sc.maxreq == 0)
		std::string reqlimit = "unlimited";
	std::cout << std::setw(30) << "request limited: " << reqlimit << std::endl;

	std::string testAE = "no";
	if (sc.aecheck)
		testAE = "yes";
	std::cout << std::setw(30) << "checking AETitle: " << testAE << std::endl;
}
