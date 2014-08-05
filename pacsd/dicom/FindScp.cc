#include "FindScp.h"
#include "dcmtk/dcmqrdb/dcmqrcbf.h"

static OFLogger echoscuLogger = OFLog::getLogger ("PACS-SIMPLE");

void findCallback (
	// in
	void *callbackData,
	OFBool cancelled,
	T_DIMSE_C_FindRQ *request,
	DcmDataset *requestIdentifiers,
	int responseCount,
	// out
	T_DIMSE_C_FindRSP *response,
	DcmDataset **responseIdentifiers,
	DcmDataset **statusDetail
) {
	/*bzero((char*)&response, sizeof (T_DIMSE_C_FindRSP));
	*statusDetail = NULL;*/

	//DcmQueryRetrieveFindContext *context = OFstatic_cast(DcmQueryRetrieveFindContext *, callbackData);
	//context->callbackHandler(cancelled, request, requestIdentifiers, responseCount, response, responseIdentifiers, statusDetail);
}

OFCondition FindScp (T_ASC_Association *assoc, T_DIMSE_Message *msg, T_ASC_PresentationContextID presID) {
	OFLog::configure (OFLogger::DEBUG_LOG_LEVEL);

	OFCondition cond = EC_Normal;
	T_DIMSE_C_FindRQ *req = &msg->msg.CFindRQ;

	T_DIMSE_C_FindRSP response;
	response.DimseStatus = STATUS_Success;
	response.MessageIDBeingRespondedTo = req->MessageID;
	response.DataSetType = DIMSE_DATASET_NULL;
	strcpy (response.AffectedSOPClassUID, req->AffectedSOPClassUID);
	response.opts = O_FIND_AFFECTEDSOPCLASSUID;

	DCMNET_INFO ("C-FIND: received request with id " << req->MessageID);
	if (req->DataSetType == DIMSE_DATASET_NULL) {
		DCMNET_INFO ("C-FIND: request received but no dataset announced, aborting");
		return DIMSE_BADMESSAGE;
	}

	// findSCP in:
	// dcmtk.dev/dcmtk-3.6.0/dcmqrdb/libsrc/dcmqrsrv.cc
	// and orthanc.dev/OrthancServer/Internals/FindScp.cpp

	cond = DIMSE_findProvider (
		assoc,
		presID,
		req,
		findCallback,	// callback void function
		&response,
		DIMSE_BLOCKING,	// blocking mode
		0						// dimse timeout, 0 for blocking mode
	);

	return cond;
}

