#include "request.h"
#include "../core/exception.h"
#include "../core/subroutine.h"

#include "FindScp.h"
#include "StoreScp.h"

OFCondition PacsdRequest::getRequest (T_ASC_Association *assoc) {
	DcmDataset *sd = NULL;
	
	cond = EC_Normal;
	T_ASC_PresentationContextID presID = 0;
	T_DIMSE_Message msg;

	// ABOUT: http://dcmtk.sourcearchive.com/documentation/3.6.0-9/classDcmSCP_a1e212118e46f2ac1ca1b1465d3798260.html
	cond = DIMSE_receiveCommand (assoc, DIMSE_NONBLOCKING, 1, &presID, &msg, &sd);

	if (cond == DUL_PEERREQUESTEDRELEASE) {
		DCMNET_INFO ("Received release request");
		ASC_acknowledgeRelease (assoc);
		ASC_dropSCPAssociation (assoc);
		return cond;
	}

	if (cond == DUL_PEERABORTEDASSOCIATION) {
		DCMNET_INFO ("Received abort request");
	}

	if (cond != EC_Normal) {
		DCMNET_ERROR ("PacsdRequest: errors was found: " << cond.text());
		return cond;
	}

	DicomRequestType request; // Echo, Find, Move, Store, Get
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
			DCMNET_ERROR ("PacsdRequest: unsupported command: " << HRRequest (request));
			std::string e_msg = "Usupported command" + HRRequest (request);
			return NULL;
	}

	DCMNET_INFO ("Received " << HRRequest (request) << " request");

	// extracted from prev switch , cause filtering ops mb increased -> supported gets false
	switch (request) {
		case DicomRequestType_Echo:
		{
			cond = EchoScp (assoc, &msg, presID);
			if (cond.bad ())
				DCMNET_ERROR ("C-ECHO failed: " << cond.text());
			else
				DCMNET_INFO ("C-ECHO success");

			break;
		}

		case DicomRequestType_Store:
		{
			cond = StoreScp (assoc, &msg, presID);
			if (cond.bad ())
				DCMNET_ERROR ("C-STORE failed: " << cond.text());
			else
				DCMNET_INFO ("C-STORE success");

			break;
		}

		case DicomRequestType_Move:
		{
			cond = MoveScp (assoc, &msg, presID);
			if (cond.bad ())
				DCMNET_ERROR ("C-MOVE failed: " << cond.text());
			else
				DCMNET_INFO ("C-MOVE success");

			break;
		}

		case DicomRequestType_Find:
		{
			cond = FindScp (assoc, &msg, presID);
			if (cond.bad ())
				DCMNET_ERROR ("C-FIND failed: " << cond.text());
			else
				DCMNET_INFO ("C-FIND success");

			break;
		}

		case DicomRequestType_Get:
		{
			//TODO
			break;
		}

		default:
			break;
	}

	return cond;
}

OFCondition PacsdRequest::EchoScp (T_ASC_Association *assoc, T_DIMSE_Message *msg, T_ASC_PresentationContextID presID) {
	return DIMSE_sendEchoResponse (assoc, presID, &msg->msg.CEchoRQ, STATUS_Success, NULL);
}
