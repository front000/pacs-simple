#include "ImageMap.h"
#include "../../core/PacsdException.h"

#include "dcmtk/dcmdata/dcistrmf.h"
#include "dcmtk/dcmsr/dsrtypes.h"
#include "dcmtk/dcmdata/dcostrmb.h"

/* Setup empty tag */
OFCondition ImageMap::setEmptyTag (std::size_t tagID) {
	OFString value;

	if (DicomTags[tagID] == DCM_StudyDate) {
		value = "00000000";
	} else if (DicomTags[tagID] == DCM_StudyTime) {
		value = "00";
	} else if (DicomTags[tagID] == DCM_StudyID) {
		value = "0";
	} else if (DicomTags[tagID] == DCM_PatientName) {
		value = "John Doe";
	} else if (DicomTags[tagID] == DCM_PatientID) {
		value = "JohnDoe";
	} else {
		value = "";
	}

	OFCondition cond = dset->putAndInsertString (DicomTags[tagID], value.c_str ());
	return cond;
}

/* Checks for mandatory/DICOM needed tags */
void ImageMap::is_MandatoryTags () {
	std::size_t i;

	/* Mandatory tags */
	for (i = 0; i < MTSize; i++) {
		OFString value;
		OFCondition cond = dset->findAndGetOFString (MandatoryTags[i], value);
		if (cond.bad () || value.empty ()) {
			ss << "Mandatory tag was not found, ID is " << i << cond.text ();
			throw PacsdException (ss.str ());
		}
	}

	/* Other needed */
	for (i = 0; i < DTSize; i++) {
		OFString value;
		if (dset->findAndGetOFString (DicomTags[i], value).bad ()) {
			OFCondition cond = setEmptyTag (i);
			if (cond.bad ()) {
				throw PacsdException (cond.text ());
			}
		}
		
		// Patch for dotted StudyTime: xxxx.xx.xx => xxxxxxxx
		if (DicomTags[i] == DCM_StudyDate && value.size () == 10) {
			OFString st = value.substr (0, 4) + value.substr (5, 2) + value.substr (8, 2);
			dset->remove (DCM_StudyDate);
			dset->putAndInsertString (DCM_StudyDate, st.c_str ());
		}
	}
}

/* Load dataset into memory */
OFBool ImageMap::buffToMemory (OFString &buff, DcmDataset *dset) {
	E_TransferSyntax xfer = dset->getOriginalXfer ();
	if (xfer = EXS_Unknown)
		xfer = EXS_LittleEndianExplicit;

	DcmFileFormat ff (dset);
	OFCondition cond = ff.validateMetaInfo (xfer);
	if (cond.bad ()) {
		DCMNET_ERROR ("Could not validate Metainfo for storing image: " << cond.text ());
		return OFFalse;
	}

	E_EncodingType encodingType = EET_ExplicitLength;
	uint32_t elemSize = ff.calcElementLength (xfer, encodingType);

	buff.resize (elemSize);
	DcmOutputBufferStream ob (&buff[0], elemSize);

	ff.transferInit ();
	cond  = ff.write (ob, xfer, encodingType, NULL, EGL_recalcGL, EPD_withoutPadding);
	ff.transferEnd ();

	if (cond.bad ()) {
		ss << "Could not store image into memory: " << cond.text ();
		throw PacsdException (ss.str ());
	}

	buff.clear ();
	return OFTrue;
}
