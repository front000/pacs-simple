#ifndef _SUBROUTINE_H_
#define _SUBROUTINE_H_

#include "boost/filesystem.hpp"
#include <boost/lambda/bind.hpp>
#include <sstream>
#include <iomanip>

//#include "../dicom/request.h"
//#include "../internals/ImageMap.h"

std::string HRRequest (DicomRequestType request) {
	switch (request) {
		case DicomRequestType_Echo:
			return "C-ECHO";

		case DicomRequestType_Store:
			return "C-STORE";

		case DicomRequestType_Move:
			return "C-MOVE";

		case DicomRequestType_Find:
			return "C-FIND";

		case DicomRequestType_Get:
			return "C-GET";

		default:
			return "unknown command";
	}
}

const char *genSavePath (DicomSkel ds) {
	std::string path = "/var/dicom"; // TODO: load from config
	path += "/" + ds.s_year;
	path += "/" + ds.s_month;
	path += "/" + ds.s_day;
	path += "/" + ds.s_hour;
	path += "/" + ds.p_first_letter;
	path += "/" + ds.p_id;
	path += "/" + ds.s_id;

	return path.c_str ();
}

const char *genFilename (const char *dpath) {
	using namespace boost::filesystem;
	using namespace boost::lambda;

	path p (dpath);

	// count files in dpath
	int count = std::count_if (
		directory_iterator (p),
		directory_iterator (),
		bind (
			static_cast<bool(*)(const path&)>(is_regular_file),
			bind(&directory_entry::path, _1)
		)
	);

	count += 1;
	std::stringstream ss_filename;
	ss_filename << std::setfill('0') << std::setw(5) << count;

	std::stringstream ss_dpath;
	ss_dpath << dpath;

	return (ss_dpath.str() + "/" + ss_filename.str() + ".dcm").c_str();
}

bool mkSavePath (const char *path) {
	boost::filesystem::path dir (path);
	return boost::filesystem::create_directories (path);
}

#endif
