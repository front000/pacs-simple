#ifndef _FINDSCP_
#define _FINDSCP_

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmnet/diutil.h"
#include <vector>

static struct CFindAnswers {
	std::size_t last_;
	std::size_t itNum; // Current iteration (response) number 0 -> data.size ()
	std::vector<std::vector<OFString> > data;
	std::vector<OFString> keys;
} CFindData;

OFCondition FindScp (
	T_ASC_Association *assoc,
	T_DIMSE_Message *msg,
	T_ASC_PresentationContextID presID
);

/* Loading QueryRetrieveLevel tag */
OFString CFindQueryLevel (DcmDataset *requestIdentifiers);

/* Generates available C-FIND keys */
void getCFindKeys (std::vector<OFString> &CFindKeys);

/* Checks requested keys in available list */
OFBool isCFindKey (std::vector<OFString> &CFindKeys, OFString tagName);

#endif
