#include "StoreScp.h"
#include "internals/ImageMap.h"
#include "../core/PacsdException.h"

#include <sstream>
#include <iomanip>
#include <vector>
#include <boost/filesystem.hpp>

OFBool storeInstanceFile (DcmDataset *dset) {
	OFString savepath = genSavePath (dset);
	OFString filename = genFilename (savepath);
	OFString fullpath = savepath + "/" + filename;
	DCMNET_INFO ("Full path: " << fullpath);

	DcmFileFormat ff (dset);
	try {
		OFCondition cond = ff.saveFile (fullpath.c_str ());
		if (cond.bad ()) {
			//ss << "Could not save file " << fullpath << ": " << cond.text ();
			throw PacsdException (cond.text ());
		}
	}
	catch (PacsdException &e) {
		DCMNET_ERROR ("Could not store (method FILE) instance: " << e.what ());
	}

	return OFTrue;
}

OFString genSavePath (DcmDataset *dset) {
	std::vector<OFString> dpath;
	OFString savepath ("/var/dicom");
	OFString delim ("/"); // delimiter

	OFString value;
	if (dset->findAndGetOFString (DCM_StudyDate, value).good ()) {
		dpath.push_back (value.substr (0, 4));
		dpath.push_back (value.substr (4, 2));
		dpath.push_back (value.substr (6, 2));
	}

	if (dset->findAndGetOFString (DCM_StudyTime, value).good ()) {
		dpath.push_back (value.substr (0, 2));
	}

	if (dset->findAndGetOFString (DCM_StudyID, value).good ()) {
		dpath.push_back (value);
	}

	if (dset->findAndGetOFString (DCM_PatientName, value).good ()) {
		dpath.push_back (value.substr (0, 1));
	}

	if (dset->findAndGetOFString (DCM_PatientID, value).good ()) {
		dpath.push_back (value);
	}

	// join
	std::vector<OFString>::const_iterator it = dpath.begin ();
	for (it; it != dpath.end (); ++it) {
		savepath += delim + *it;
	}
	dpath.clear ();

	DCMNET_INFO ("Save path generated: " << savepath);

	// creating dirs: mkdir -p <path>
	namespace fs = boost::filesystem;
	if (fs::exists (savepath.c_str ())) {
		return savepath;
	} else {
		try {
			fs::path p (savepath.c_str ());
			fs::create_directories (p);
		}
		catch (std::exception const &e) {
			DCMNET_ERROR ("Could not create savepath " << savepath << ": " << e.what ());
		}
		catch (...) {
			DCMNET_ERROR ("Unknown error, when creating directory " << savepath);
		}
	}

	return savepath;
}

OFString genFilename (OFString &savepath) {
	namespace fs = boost::filesystem;

	std::size_t count = 1;
	try {
		fs::directory_iterator iterator (savepath.c_str ());
		for (; iterator != fs::directory_iterator(); ++iterator) {
			count++;
		}
	}
	catch (std::exception &e) {
		DCMNET_ERROR ("Could not access directory " << savepath << ": " << e.what ());
	}
	catch (...) {
		DCMNET_ERROR ("Unknown error, when counting files into directory " << savepath);
	}

	std::stringstream ss;
	ss.str (std::string ());
	ss << std::setfill ('0') << std::setw (5) << count;
	ss << ".dcm";
	
	//OFString value = static_cast<OFString>(ss.str ().c_str ());
	OFString value = ss.str ().c_str ();
	return value;
}

void storeSCPCallback (
	/* callbackData */ void *,
	T_DIMSE_StoreProgress *progress,
	T_DIMSE_C_StoreRQ *request,
	/* imageFileName */ char *,
	DcmDataset **imageDataset,
	T_DIMSE_C_StoreRSP *response,
	DcmDataset **statusDetail
) {
	if (progress->state == DIMSE_StoreEnd) {
		if ((imageDataset) && (*imageDataset)) {
			OFString buff;
			ImageMap map;

			try {
				if (!map.buffToMemory (buff, *imageDataset)) {
					response->DimseStatus = STATUS_STORE_Error_CannotUnderstand; // MetaInfo incorrect
					return;
				}
			}
			catch (PacsdException &e) {
				DCMNET_ERROR ("Out of resources: " << e.what ());
				buff.clear ();
				response->DimseStatus = STATUS_STORE_Refused_OutOfResources;
				return;
			}

			if (response->DimseStatus == STATUS_Success) {
				/* Checking SOPs */
				DIC_UI sopClass, sopInstance;
				if (!DU_findSOPClassAndInstanceInDataSet (*imageDataset, sopClass, sopInstance, OFFalse)) {
					response->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
				} else if (strcmp (sopClass, request->AffectedSOPClassUID) != 0) {
					response->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
				} else {
					map.dset = *imageDataset;
					try {
						/* Checking for mandatory tags */
						map.is_MandatoryTags ();
				
						/* Storing file on filesystem */
						storeInstanceFile (map.dset);

						/* Storing data into database */
						//storeInstanceDb (map.dset);
					}
					catch (PacsdException &e) {
						DCMNET_ERROR ("Could not store instance: " << e.what ());
						delete imageDataset;
						*imageDataset = NULL;
						return;
					}
				}
			}

			/*
			delete imageDataset;
			*imageDataset = NULL;
			*/
		}

		/*
		delete statusDetail;
		*statusDetail = NULL;
		*/
	}
}

OFCondition StoreScp (T_ASC_Association *assoc, T_DIMSE_Message *msg, T_ASC_PresentationContextID presID) {
	OFCondition cond = EC_Normal;
	T_DIMSE_C_StoreRQ *request = &msg->msg.CStoreRQ;
	
	DcmFileFormat dfile;
	DcmDataset *dset = dfile.getDataset ();

	cond = DIMSE_storeProvider (assoc, presID, request, NULL, OFFalse, &dset, storeSCPCallback, NULL, DIMSE_BLOCKING, 0);
	return cond;
}
