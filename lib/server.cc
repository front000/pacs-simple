#include "server.h"
#include "core/exception.h"
//#include "dicom/request.h"
#include "dicom/PacsdRequest.h"
#include <vector>

Server::~Server () {
	cond = ASC_dropNetwork (&net);

	if (cond.bad ())
		DCMNET_ERROR ("Network: could no be dropped");
	DCMNET_INFO ("Network: dropped");
}

T_ASC_Network *Server::Init () {
	cond = ASC_initializeNetwork (
		NET_ACCEPTOR,
		sc.port,
		sc.timeout,
		&net
	);

	if (cond.bad ()) {
		DCMNET_ERROR ("Network: could not initialize");
		throw PacsdException ("Could not init network");
	}
	DCMNET_INFO ("Network: initialized");

	return net;
}

OFCondition Association::Cleanup (T_ASC_Association *assoc) {
	if (!assoc)
		throw PacsdException ("Association: could not cleanup, cause not defined");
		
	cond = ASC_dropSCPAssociation (assoc, sc.timeout);
	if (cond.bad ()) {
		DCMNET_ERROR ("Association: could not drop");
		return cond;
	}
	//DCMNET_INFO ("Association: dropped");

	cond = ASC_destroyAssociation (&assoc);
	if (cond.bad ())
		DCMNET_ERROR ("Association: could not destroy");
	//DCMNET_INFO ("Association: destroyed");

	return cond;
}

//T_ASC_Association *Association::Accept (T_ASC_Network *net) {
void *Association::Connect (T_ASC_Network *net) {
	// init assoc
	cond = ASC_receiveAssociation (net, &assoc, ASC_DEFAULTMAXPDU, NULL, NULL, OFFalse, DUL_NOBLOCK, sc.timeout);
	
	// timeout, nothing to connect thread
	if (cond == DUL_NOASSOCIATIONREQUEST) {
		Cleanup (assoc);
		return NULL; // thread finished
	}

	// could not create association
	if (cond.bad ()) {
		DCMNET_ERROR ("Association: could not create association after init: " << cond.text ());
		return NULL;
	}
	DCMNET_INFO ("Association: receiving enabled");

	/* 
		Allowed syntaxes and presentation contexts
	*/
	// non-compressed TSs
	std::vector<const char*> transferSyntaxes;
	transferSyntaxes.push_back (UID_LittleEndianExplicitTransferSyntax);
	transferSyntaxes.push_back (UID_BigEndianExplicitTransferSyntax);
	transferSyntaxes.push_back (UID_LittleEndianImplicitTransferSyntax);

	std::vector<const char*> presentationContexts;
	
	// C-STORE
	presentationContexts.push_back (UID_VerificationSOPClass);

	// C-FIND
	presentationContexts.push_back (UID_FINDPatientRootQueryRetrieveInformationModel);	// Patient level
	presentationContexts.push_back (UID_FINDStudyRootQueryRetrieveInformationModel);		// Study level
	//presentationContexts.push_back (UID_FINDModalityWorklistInformationModel);				// Series level
	//TODO: image level, I don't know UID_

	// C-MOVE
	presentationContexts.push_back (UID_MOVEStudyRootQueryRetrieveInformationModel);

	/*
		Connection checks
	*/
	cond = ASC_acceptContextsWithPreferredTransferSyntaxes (assoc->params, &presentationContexts[0], presentationContexts.size(), &transferSyntaxes[0], transferSyntaxes.size());
	if (cond.bad ()) {
		DCMNET_ERROR ("Association: presentation context or transfer syntaxes wrong");
		Cleanup (assoc);
		return NULL;
	}

	// the array of Storage SOP Class UIDs comes from dcuid.h
	cond = ASC_acceptContextsWithPreferredTransferSyntaxes (assoc->params, dcmAllStorageSOPClassUIDs, numberOfAllDcmStorageSOPClassUIDs, &transferSyntaxes[0], transferSyntaxes.size());
	if (cond.bad ()) {
		DCMNET_ERROR ("Association: preferred transfer syntaxes wrong (from dcuid.h)");
		Cleanup (assoc);
		return NULL;
	}

	/*
		Generating assoc with client
	*/
	ASC_setAPTitles (assoc->params, NULL, NULL, sc.aetitle.c_str());

	// check for received app context name
	cond = ASC_getApplicationContextName (assoc->params, buff);
	if (cond.bad () || (strcmp (buff, UID_StandardApplicationContext) != 0)) {
		T_ASC_RejectParameters reject = {
			ASC_RESULT_REJECTEDPERMANENT,
			ASC_SOURCE_SERVICEUSER,
			ASC_REASON_SU_APPCONTEXTNAMENOTSUPPORTED
		};

		DCMNET_ERROR ("Association: rejected, bad context name: " << buff);
		cond = ASC_rejectAssociation (assoc, &reject); // TODO check for reject?
		Cleanup (assoc);
		return NULL;
	}

	// check the AETs
	{
		DIC_AE
			hostIP_C, hostTitle_C,
			userIP_C, userTitle_C;

		if (
			ASC_getAPTitles (assoc->params, hostTitle_C, userTitle_C, NULL).bad () ||
			ASC_getPresentationAddresses (assoc->params, hostIP_C, userIP_C).bad ()
		) {
			T_ASC_RejectParameters reject = {
				ASC_RESULT_REJECTEDPERMANENT,
				ASC_SOURCE_SERVICEUSER,
				ASC_REASON_SU_NOREASON
			};
			
			DCMNET_ERROR ("Association: rejected, checks for userIP/Title failed");
			cond = ASC_rejectAssociation (assoc, &reject); // TODO check for reject?
			Cleanup (assoc);
			return NULL;
		}

		hostIP = std::string (hostIP_C);
		hostTitle = std::string (hostTitle_C);
		userTitle = std::string (userTitle_C);
		//DCMNET_INFO ("hostIP is " << hostIP << ", hostTitle is " << hostTitle << ", userTitle is " << userTitle); // TODO remove this?

		// TODO: make or ignore this: server.IsMyAETitle, server.HasApplicationEntityFilte
	}

	OFBool opt_rejectWithoutImplementationUID = OFFalse; // TODO hardcode!!!
	if (opt_rejectWithoutImplementationUID && strlen (assoc->params->theirImplementationClassUID) == 0) {
		T_ASC_RejectParameters reject = {
			ASC_RESULT_REJECTEDPERMANENT,
			ASC_SOURCE_SERVICEUSER,
			ASC_REASON_SU_NOREASON
		};

		cond = ASC_rejectAssociation (assoc, &reject); //TODO check for reject?
		Cleanup (assoc);
		return NULL;
	}

	{
		cond = ASC_acknowledgeAssociation (assoc);
		if (cond.bad ()) {
			DCMNET_ERROR ("Association: failed, is not aknowledge: " << cond.text ());
			Cleanup (assoc);
			return NULL;
		}

		DCMNET_INFO ("Association: is aknowledged, Max Send PDV = " << assoc->sendPDVLength);
		if (ASC_countAcceptedPresentationContexts (assoc->params) == 0)
			DCMNET_INFO ("Association: but valid presentation contexts was not found");

		DCMNET_INFO ("Association: negotiated");
	}

	// receiving requests
	
	bool connected = true; // TODO!!!! check current connection for exists
	while (cond.good ()) {
		if (assoc == NULL)
			break;

		PacsdRequest req;
		cond = req.getRequest (assoc);
	}
	ASC_acknowledgeRelease (assoc); // FFF
	DCMNET_INFO ("Connection closed");
	Cleanup (assoc);
	// exiting thread and clothing connection
}
