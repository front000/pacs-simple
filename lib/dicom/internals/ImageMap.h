#ifndef _IMAGEMAP_H_
#define _IMAGEMAP_H_

#include "dcmtk/config/osconfig.h"                                                                                                                                                                                 
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmnet/dimse.h"                                                                                                                                                                                    
#include "dcmtk/dcmnet/diutil.h"
#include <sstream>

const std::size_t DTSize = 24;
static DcmTagKey DicomTags[ DTSize ] = {
	/* patient modules */
	DCM_PatientID,
	DCM_PatientName,
	DCM_PatientSex,
	DCM_PatientBirthDate,
	/* if patient is animal */ DCM_PatientSpeciesDescription,

	/* general study */
	DCM_StudyID,
	DCM_StudyInstanceUID,
	DCM_AccessionNumber,
	DCM_StudyDate,
	DCM_StudyTime,
	DCM_StudyDescription,

	/* general series */
	DCM_SeriesInstanceUID,
	DCM_SeriesDescription,
	DCM_SeriesDate,
	DCM_SeriesTime,
	DCM_SeriesNumber,
	DCM_Modality,

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

const std::size_t MTSize = 5;
static DcmTagKey MandatoryTags[ MTSize ] = {
	DCM_StudyInstanceUID,
	DCM_Modality,
	DCM_SeriesInstanceUID,
	DCM_SOPClassUID,
	DCM_SOPInstanceUID
};

class ImageMap {
private:
	OFCondition setEmptyTag (std::size_t tagID);
	std::stringstream ss;

public:
	ImageMap () {};
	~ImageMap() {};
	/*
	~ImageMap() {
		delete *dset;
		*dset = NULL;
	};
	*/
	OFBool buffToMemory (OFString &buff, DcmDataset *dset);
	void is_MandatoryTags ();

	DcmDataset *dset;
};

#endif
