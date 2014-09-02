#include "FindScp.h"
#include "../db/db_mysql.h"
#include "dcmtk/dcmdata/dcobject.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include <algorithm>

static OFLogger echoscuLogger = OFLog::getLogger ("PACS-SIMPLE");

void FindScpCallback (
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
	/*
	bzero((char*)&response, sizeof (T_DIMSE_C_FindRSP));
	*statusDetail = NULL;
	*/

	Database dbh;

	if (cancelled) {
		strcpy(response->AffectedSOPClassUID, request->AffectedSOPClassUID);
		response->MessageIDBeingRespondedTo = request->MessageID;
		response->DimseStatus = STATUS_FIND_Cancel_MatchingTerminatedDueToCancelRequest;
		response->DataSetType = DIMSE_DATASET_NULL;
		return;
	}

	if (responseCount == 1) {
		//dbh.connect (); // initializing

		OFString QueryRootLevel = CFindQueryLevel (requestIdentifiers);
		// If QueryRetrieveLevel was not received -> rejecting request
		if (QueryRootLevel.empty ()) {
			strcpy(response->AffectedSOPClassUID, request->AffectedSOPClassUID);
			response->MessageIDBeingRespondedTo = request->MessageID;
			response->DimseStatus = STATUS_FIND_Failed_UnableToProcess; // TODO: find key to make exception "Invalid column name 'NONELevel'"
			response->DataSetType = DIMSE_DATASET_NULL;
			return;
		}

		OFString query = dbh.CFindSQLQuery (requestIdentifiers, QueryRootLevel);
		DCMNET_INFO ("query: " << query);
		// proceed next request
		response->DimseStatus = STATUS_Pending;

		/*
		DcmStack stack;
		DcmObject *object = NULL;
		OFBool next_ = OFTrue;

		stack.clear ();
		while (requestIdentifiers->nextObject (stack, next_).good ()) {
			object = stack.top ();
			DcmTag tag = object->getTag ();
			OFString t = tag.getTagName ();
			DCMNET_INFO ("C-FIND handled key: " << object->getTag() << ", tag name is " << tag.getTagName());
		}
		*/
	}

	// finalizing -> Success
	strcpy(response->AffectedSOPClassUID, request->AffectedSOPClassUID);
	response->MessageIDBeingRespondedTo = request->MessageID;
	response->DimseStatus = STATUS_Success;
	response->DataSetType = DIMSE_DATASET_NULL;
	return;

	requestIdentifiers->print (std::cout);
	DCMNET_INFO ("ResponseCount is " << responseCount);
}

OFCondition FindScp (T_ASC_Association *assoc, T_DIMSE_Message *msg, T_ASC_PresentationContextID presID) {
	OFCondition cond = EC_Normal;
	T_DIMSE_C_FindRQ *req = &msg->msg.CFindRQ;

	T_DIMSE_C_FindRSP response;

	/* construct default struct for response */
	response.opts = O_FIND_AFFECTEDSOPCLASSUID;

	cond = DIMSE_findProvider (assoc, presID, req, FindScpCallback, &response, DIMSE_BLOCKING, 0);

	return cond;
}

OFString CFindQueryLevel (DcmDataset *requestIdentifiers) {
	OFString value;
	if (requestIdentifiers->findAndGetOFString (DCM_QueryRetrieveLevel, value).bad ())
		return value;

	value = OFStandard::toLower (value);
	if ((value != "patient") && (value != "study") && (value != "series") && (value != "image"))
		value.clear ();

	return value;
}

void getCFindKeys (std::vector<OFString> &CFindKeys) {
	/* Patient */
	CFindKeys.push_back ("PatientID");
	CFindKeys.push_back ("PatientName");
	CFindKeys.push_back ("PatientSex");
	CFindKeys.push_back ("PatientBirthDate");
	CFindKeys.push_back ("PatientSpeciesDescription");
	CFindKeys.push_back ("SpecificCharacterSet");

	/* Study */
	CFindKeys.push_back ("StudyID");
	CFindKeys.push_back ("StudyInstanceUID");
	CFindKeys.push_back ("AccessionNumber");
	CFindKeys.push_back ("StudyDate");
	CFindKeys.push_back ("StudyTime");
	CFindKeys.push_back ("ReferringPhysicianName");
	CFindKeys.push_back ("StudyDescription");

	/* Series */
	CFindKeys.push_back ("SeriesInstanceUID");
	CFindKeys.push_back ("SeriesDescription");
	CFindKeys.push_back ("SeriesDate");
	CFindKeys.push_back ("SeriesTime");
	CFindKeys.push_back ("SeriesNumber");
	CFindKeys.push_back ("Modality");

	/* Image */
	CFindKeys.push_back ("SOPClassUID");
	CFindKeys.push_back ("SOPInstanceUID");
	CFindKeys.push_back ("ContentDate");
	CFindKeys.push_back ("ContentTime");
	CFindKeys.push_back ("InstanceNumber");
}

OFBool isCFindKey (std::vector<OFString> &CFindKeys, OFString tagName) {
	if (std::find (CFindKeys.begin(), CFindKeys.end(), tagName) != CFindKeys.end())
		return OFTrue;
	return OFFalse;
}
