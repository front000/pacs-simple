#include "FindScp.h"
#include "../db/db_mysql.h"
#include "dcmtk/dcmdata/dcobject.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmdata/dcelem.h"
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
	DCMNET_INFO ("ResponseCount is " << responseCount);
	//bzero((char*)&response, sizeof (T_DIMSE_C_FindRSP));
	//*statusDetail = NULL;

	if (cancelled) {
		memset (&CFindData, 0, sizeof (CFindData));

		strcpy(response->AffectedSOPClassUID, request->AffectedSOPClassUID);
		response->MessageIDBeingRespondedTo = request->MessageID;
		response->DimseStatus = STATUS_FIND_Cancel_MatchingTerminatedDueToCancelRequest;
		response->DataSetType = DIMSE_DATASET_NULL;
		return;
	}

	if (responseCount == 1) {
		memset (&CFindData, 0, sizeof (CFindData));

		OFString QueryRootLevel = CFindQueryLevel (requestIdentifiers);
		/* Rejecting C-Find request if QueryRetrieveLevel was not defined */
		if (QueryRootLevel.empty ()) {
			strcpy(response->AffectedSOPClassUID, request->AffectedSOPClassUID);
			response->MessageIDBeingRespondedTo = request->MessageID;
			response->DimseStatus = STATUS_FIND_Failed_UnableToProcess; // TODO: find key to make exception "Invalid column name 'NONELevel'"
			response->DataSetType = DIMSE_DATASET_NULL;
			return;
		}

		Database dbh;
		dbh.connect ();

		/* Generating query */
		OFString query = dbh.CFindSQLQuery (requestIdentifiers, QueryRootLevel, CFindData.keys);
		//DCMNET_INFO ("SQL Query: " << query);

		/* Loading results */
		dbh.get (CFindData.data, query);
		// DCMNET_INFO ("Found " << CFindData.data.size () << " items");

		/* Get last item id */
		CFindData.last_ = CFindData.data.size ();
		CFindData.itNum = 0;

		// proceed next request
		response->DimseStatus = STATUS_Pending;
		//return;
	}

	while (CFindData.itNum < CFindData.last_) {
		*responseIdentifiers = new DcmDataset;

		std::vector<OFString>::const_iterator it_data = CFindData.data[CFindData.itNum].begin ();
		std::vector<OFString>::const_iterator it_key = CFindData.keys.begin ();
		for (it_data, it_key; it_key < CFindData.keys.end (); ++it_key, ++it_data) {
			DcmTag tag;
			DcmElement *dce;

			OFCondition cond = DcmTag::findTagFromName ((*it_key).c_str (), tag);
			if (cond.bad ())
				continue;

			dce = newDicomElement (tag);
			dce->putString ((*it_data).c_str ());
			(*responseIdentifiers)->insert (dce, OFTrue);
		}

		(*responseIdentifiers)->print (std::cout);
		response->DimseStatus = STATUS_Pending;
		CFindData.itNum++;

		return;
	}

	/* Finalize  */
	memset (&CFindData, 0, sizeof (CFindData));

	strcpy(response->AffectedSOPClassUID, request->AffectedSOPClassUID);
	response->MessageIDBeingRespondedTo = request->MessageID;
	response->DataSetType = DIMSE_DATASET_NULL;
	response->DimseStatus = STATUS_Success;
	return;
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
