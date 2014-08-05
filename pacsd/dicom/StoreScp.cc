#include "StoreScp.h"
#include "../internals/ImageMap.h"
#include "../core/exception.h"

void storeSCPCallback (
	/* callbackData */ void*,
	T_DIMSE_StoreProgress *progress,
	T_DIMSE_C_StoreRQ *req,
	/* imageFileName */ char *,
	DcmDataset **imageDataSet,
	T_DIMSE_C_StoreRSP *response,
	DcmDataset **statusDetail
) {
	if (progress->state == DIMSE_StoreEnd) {
		if ( (imageDataSet != NULL) && (*imageDataSet != NULL) ) {
			std::string buff;
			ImageMap map;
			
			// TODO: exception or just if/else?
			try {
				if (!map.buffToMemory (buff, *imageDataSet)) {
					response->DimseStatus = STATUS_STORE_Refused_OutOfResources;
				}
			}
			catch (...) {
				response->DimseStatus = STATUS_STORE_Refused_OutOfResources;
			}

			if (response->DimseStatus == STATUS_Success) {
				// checking SOPs
				DIC_UI sopClass, sopInstance;
				if (!DU_findSOPClassAndInstanceInDataSet (*imageDataSet, sopClass, sopInstance, OFFalse)) {
					response->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
				} else if ( strcmp (sopClass, req->AffectedSOPClassUID) != 0 ) {
					response->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
				} else {
					try {
						map.dset = *imageDataSet;
						if (!map.storeDicomImage ()) {
							response->DimseStatus == STATUS_STORE_Refused_OutOfResources;

							delete *imageDataSet;
							*imageDataSet = NULL;
						}
					}
					catch (PacsdException &e) {
						DCMNET_ERROR ("Cannot store dicom image: " << e.what());
					}
				}
			}
		}
	}

	delete *statusDetail;
	*statusDetail = NULL;
}

OFCondition StoreScp (T_ASC_Association *assoc, T_DIMSE_Message *msg, T_ASC_PresentationContextID presID) {
	OFCondition cond = EC_Normal;
	T_DIMSE_C_StoreRQ *req = &msg->msg.CStoreRQ;

	DcmFileFormat dfile;
	DcmDataset *dset = dfile.getDataset ();

	cond = DIMSE_storeProvider (
		assoc,
		presID,
		req,
		NULL,
		/* use metaheader */ OFFalse,
		&dset,
		storeSCPCallback,
		NULL,
		/* blocking mode */ DIMSE_BLOCKING,
		/*timeout */ 0
	);

	return cond;
}
