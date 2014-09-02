#include "MoveScp.h"

struct MoveScpData {
	std::string target;
	DcmDataset *lastRequest;
};

void MoveScpCallback (
	/* in */
	void *callbackData,
	OFBool cancelled,
	T_DIMSE_C_MoveRQ *request,
	DcmDataset *requestIdentifiers,
	int responseCount,
	/* out */
	T_DIMSE_C_MoveRSP *response,
	DcmDataset **responseIdentifiers,
	DcmDataset **statusDetail
) {
	bzero (response, sizeof (T_DIMSE_C_MoveRSP));
	*statusDetail = NULL;
	*responseIdentifiers = NULL;
}

OFCondition MoveScp (T_ASC_Association *assoc, T_DIMSE_Message *msg, T_ASC_PresentationContextID presID) {
	OFCondition cond = EC_Normal;
	T_DIMSE_C_MoveRQ *req = &msg->msg.CMoveRQ;

	MoveScpData data;
	data.target = std::string (msg->msg.CMoveRQ.MoveDestination);
	data.lastRequest = NULL;
	
	cond = DIMSE_moveProvider (assoc, presID, req, MoveScpCallback, &data, DIMSE_BLOCKING, 0);

	return cond;
}
