#include "PacsdRequest.h"

#include "FindScp.h"
#include "StoreScp.h"
#include "MoveScp.h"

OFCondition PacsdRequest::getRequest (T_ASC_Association *assoc) {
	DcmDataset *statusDetail = NULL;
	OFCondition cond = EC_Normal;
	T_ASC_PresentationContextID presID = 0;
	T_DIMSE_Message msg;

	/* Receiving request */
	cond = DIMSE_receiveCommand (assoc, DIMSE_NONBLOCKING, 1, &presID, &msg, &statusDetail);

	if (cond == DUL_PEERREQUESTEDRELEASE) {
		DCMNET_INFO ("Request released from client");
		ASC_acknowledgeRelease (assoc);
		ASC_dropSCPAssociation (assoc);
		return cond;
	}

	if (cond == DUL_PEERABORTEDASSOCIATION) {
		DCMNET_INFO ("Connection aborted from client");
		ASC_dropSCPAssociation (assoc);
		return cond;
	}

	// other problems
	if (cond != EC_Normal) {
		DCMNET_ERROR ("Request errors found: " << cond.text ());
		ASC_dropSCPAssociation (assoc);
		return cond;
	}

	/* Selecting request type */
	DicomRequestType request;
	switch (msg.CommandField) {
		case DIMSE_C_ECHO_RQ:
			request = DicomRequestType_Echo;
			break;

		case DIMSE_C_STORE_RQ:
			request = DicomRequestType_Store;
			break;

		case DIMSE_C_MOVE_RQ:
			request = DicomRequestType_Move;
			break;

		case DIMSE_C_FIND_RQ:
			request = DicomRequestType_Find;
			break;

		case DIMSE_C_GET_RQ:
			request = DicomRequestType_Get;
			break;

		default:
			DCMNET_ERROR ("Unsupported request type");
			return EC_IllegalParameter;
	}

	DCMNET_INFO ("Received request: " << HRRequest (request));

	/* Executing requested command */
	switch (request) {
		case DicomRequestType_Echo:
			cond = EchoScp (assoc, &msg, presID);
			break;

		case DicomRequestType_Store:
			cond = StoreScp (assoc, &msg, presID);
			break;

		case DicomRequestType_Move:
			cond = MoveScp (assoc, &msg, presID);
			break;

		case DicomRequestType_Find:
			cond = FindScp (assoc, &msg, presID);
			break;

		/*
		case DicomRequestType_Get:
			cond = CGetScp (assoc, &msg, presID);
		*/

		default:
			break;
	}

	if (cond.bad ()) {
		DCMNET_ERROR (HRRequest (request) << " failed: " << cond.text ());
	} else {
		DCMNET_INFO (HRRequest (request) << " success");
	}

	return cond;
}

OFCondition PacsdRequest::EchoScp (T_ASC_Association *assoc, T_DIMSE_Message *msg, T_ASC_PresentationContextID presID) {
	OFCondition cond = DIMSE_sendEchoResponse (assoc, presID, &msg->msg.CEchoRQ, STATUS_Success, NULL);
	return cond;
}

OFString PacsdRequest::HRRequest (DicomRequestType request) {
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
			return "Unknown request type";
	}
}
