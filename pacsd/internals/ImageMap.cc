#include "ImageMap.h"

#include "dcmtk/dcmdata/dcistrmf.h"
#include "dcmtk/dcmsr/dsrtypes.h"
#include "dcmtk/dcmdata/dcostrmb.h"

#include <boost/filesystem.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <sstream>
#include <iomanip>

std::string ImageMap::genSavePath (DicomSkel ds) {
	std::string path = "/var/dicom"; // TODO: load from config
	path += "/" + ds.s_year;
	path += "/" + ds.s_month;
	path += "/" + ds.s_day;
	path += "/" + ds.s_hour;
	path += "/" + ds.p_first_letter;
	path += "/" + ds.p_id;
	path += "/" + ds.s_id;

	return path;
}

std::string ImageMap::genFilename (std::string &dpath) {
	using namespace boost::filesystem;
	using namespace boost::lambda;

	int count = 1;
	directory_iterator iterator(dpath);
	for(; iterator != boost::filesystem::directory_iterator(); ++iterator) { count++; }

	std::stringstream ss_filename;
	ss_filename.str (std::string ());
	ss_filename << std::setfill('0') << std::setw(5) << count;

	std::string filename = dpath + "/" + ss_filename.str() + ".dcm";
	return filename;
}

bool ImageMap::mkSavePath (std::string &dpath) {
	using namespace boost::filesystem;

	if (exists (dpath.c_str()))
		return true;
	
	path p (dpath);
	return boost::filesystem::create_directories (p);
}

OFBool ImageMap::buffToMemory (std::string &buff, DcmDataset *dset) {
	E_TransferSyntax xfer = dset->getOriginalXfer ();
	if (xfer == EXS_Unknown) xfer = EXS_LittleEndianExplicit;

	DcmFileFormat ff(dset);
	OFCondition cond = ff.validateMetaInfo (xfer);
	if (cond.bad ()) {
		DCMNET_ERROR ("Could not validate MetaInfo: " << cond.text());
		return OFFalse;
	}

	E_EncodingType encodingType = EET_ExplicitLength;
	
	uint32_t s = ff.calcElementLength (xfer, encodingType);
	buff.resize (s);
	DcmOutputBufferStream ob (&buff[0], s);

	ff.transferInit ();
	cond = ff.write (ob, xfer, encodingType, NULL, EGL_recalcGL, EPD_withoutPadding);
	ff.transferEnd ();

	if (cond.bad ()) {
		DCMNET_ERROR ("Could not store dicom file into memory: " << cond.text());
		buff.clear ();
		return OFFalse;
	}

	return OFTrue;
}

OFCondition ImageMap::addEmptyTag (int tagId) {
	return dset->putAndInsertString (DicomTags[ tagId ], "");
}

OFBool ImageMap::checkMandatoryTags () {
	int i;
	for (i = 0; i < MandatoryTagsSize; i++) {
		OFString value;
		if (dset->findAndGetOFString(DicomMandatoryTags[i], value).bad()) {
			DCMNET_ERROR ("Could not found mandatory key " << DicomMandatoryTags[i]);
			return OFFalse;
		}
	}

	return OFTrue;
}

OFBool ImageMap::loadTags () {
	int i;
	for (i = 0; i < PacsTagsSize; i++) {
		OFString value;
		if (dset->findAndGetOFString(DicomTags[i], value).good()) {
			/* do something */
			//DCMNET_INFO ("Loaded tag " << DicomTags[i]);
		} else {
			cond = addEmptyTag (i);
			if (cond.bad ()) {
				DCMNET_ERROR ("Could not add empty tag " << DicomTags[i] << ": " << cond.text());
				return OFFalse;
			} else {
				//DCMNET_INFO ("Added tag " << DicomTags[i]);
			}
		}

		// setting up struct skel
		if (DicomTags[i] == DCM_StudyDate) {
			if (value.size() == 0) value = "00000000";
			const char *s = value.c_str ();
			std::stringstream year, month, day;

			year	<< s[0] << s[1] << s[2] << s[3];
			month	<< s[4] << s[5];
			day	<< s[6] << s[7];

			skel.s_year	= year.str ();
			skel.s_month= month.str ();
			skel.s_day	= day.str ();
		} else if (DicomTags[i] == DCM_StudyTime) {
			if (value.size() == 0) value = "00";
			const char *s = value.c_str ();
			std::stringstream hour;

			hour << s[0] << s[1];
			skel.s_hour = hour.str();
		} else if (DicomTags[i] == DCM_StudyID) {
			if (value.size() == 0) value = "0";
			std::stringstream sid;
			sid << value;

			skel.s_id = sid.str ();
		} else if (DicomTags[i] == DCM_PatientName) {
			if (value.size() == 0) value = "A";
			const char *s = value.c_str ();
			std::stringstream pname;

			pname << s[0];
			skel.p_first_letter = pname.str ();
		} else if (DicomTags[i] == DCM_PatientID) {
			if (value.size() == 0) value = "JohnDoe";
			std::stringstream pid;
			pid << value;

			skel.p_id = pid.str ();
		}
	}

	return OFTrue;
}

OFBool ImageMap::storeDicomImage () {
	OFBool modified = OFFalse;

	if (!checkMandatoryTags ())
		return OFFalse;

	if (!loadTags ())
		return OFFalse;

	DcmFileFormat ff(dset);
	//const char *dpath = genSavePath (skel);
	std::string dpath = genSavePath (skel);
	DCMNET_INFO ("Generated save path is " << dpath.c_str());

	if (!mkSavePath (dpath)) {
		DCMNET_ERROR ("Could not create save path: " << dpath);
		return OFFalse;
	} else {
		DCMNET_INFO ("Directory " << dpath << " was created");
	}

	std::string filename = genFilename (dpath);
	DCMNET_INFO ("Generated filename is " << filename.c_str());

	if (ff.saveFile (filename.c_str()).good()) {
		DCMNET_INFO ("Files " << filename.c_str() << " was stored");
		// TODO: mysql handling
	} else {
		DCMNET_ERROR ("Could not store file " << filename.c_str() << ": " << cond.text());
		return OFFalse;
	}

	return OFTrue;
}
