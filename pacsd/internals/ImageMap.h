#ifndef _IMAGEMAP_H_
#define _IMAGEMAP_H_

#include "dcmtk/config/osconfig.h"                                                                                                                                                                                 
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmnet/dimse.h"                                                                                                                                                                                    
#include "dcmtk/dcmnet/diutil.h"
//#include "dcmtk/dcmdata/dcistrmf.h"
//#include "dcmtk/dcmsr/dsrtypes.h"

const int MandatoryTagsSize = 5;
const int PacsTagsSize = 22;

static DcmTagKey DicomTags[ PacsTagsSize ] = {
	/* patient modules */
	DCM_PatientName,
	DCM_PatientID,
	DCM_PatientBirthDate,
	DCM_PatientSex,
	/* if patient is animal */ DCM_PatientSpeciesDescription,

	/* general study */
	DCM_StudyInstanceUID,
	DCM_StudyDate,
	DCM_StudyTime,
	DCM_ReferringPhysicianName,
	DCM_StudyID,
	DCM_AccessionNumber,

	/* general series */
	DCM_Modality,
	DCM_SeriesInstanceUID,
	DCM_SeriesNumber,
	DCM_Laterality,

	/* sc equipment */
	DCM_InstanceNumber,
	DCM_PatientOrientation,
	DCM_ContentDate,
	DCM_ContentTime,

	/* SOP common */
	DCM_SOPClassUID,
	DCM_SOPInstanceUID,
	DCM_SpecificCharacterSet
};

static DcmTagKey DicomMandatoryTags[ MandatoryTagsSize ] = {
	DCM_StudyInstanceUID,
	DCM_Modality,
	DCM_SeriesInstanceUID,
	DCM_SOPClassUID,
	DCM_SOPInstanceUID
};

struct DicomSkel {
	/* study */
	std::string	s_year;
	std::string	s_month;	
	std::string s_day;
	std::string s_hour;
	std::string s_id;
	/* patient */
	std::string p_first_letter;
	std::string p_id;
};
static struct DicomSkel skel;

class ImageMap {
private:
	OFCondition addEmptyTag (int tagId);
	OFBool loadTags ();
	OFBool checkMandatoryTags ();
	std::string genSavePath (DicomSkel ds);
	std::string genFilename (std::string &dpath);
	bool mkSavePath (std::string &dpath);

	OFCondition cond;

public:
	ImageMap () {};
	/*
	~ImageMap() {
		delete *dset;
		*dset = NULL;
	};
	*/
	~ImageMap() {};

	OFBool buffToMemory (std::string &buff, DcmDataset *dset);
	//void showTags ();
	OFBool storeDicomImage ();

	DcmDataset *dset;
};

#endif
