#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "lib/server.h"
#include "lib/core/config.h"
#include "lib/core/exception.h"

/*
int thread_id () {
	std::cout << boost::this_thread::get_id() << std::endl;
	return 1;
}
*/

int main (int argc, char **argv) {
	try {
		// loading configuration
		Config conf;
		ServerConfig sc = conf.Load (argv[1]);
		conf.Show ();

		// initializing server
		Server s;
		s.sc = sc;
		T_ASC_Network *net = s.Init (); // <-- dcmtk: initNetwork ()

		for (;;) { // daemon mode
			Association conn;
			conn.sc = sc;
			//boost::thread t (&Association::Accept, &conn, net);
			boost::thread t (&Association::Connect, &conn, net);
			t.join ();
		}
	}

	catch (PacsdException &e) {
		std::cerr << "Exception (Pacsd): " << e.what() << std::endl;
		return 1;
	}

	catch (std::exception &e) {
		std::cerr << "Exception: " << e.what() << std::endl;
		return 1;
	}

	catch (...) {
		std::cerr << "Unknown exception" << std::endl;
		return 1;
	}

	return 0;
}

