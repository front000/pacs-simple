#ifndef _REQUEST_H_
#define _REQUEST_H_

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/ofstd/ofcond.h"

#include "FindScp.h"
#include "MoveScp.h"
#include "StoreScp.h"

enum DicomRequestType {
	DicomRequestType_Echo,
	DicomRequestType_Find,
	DicomRequestType_Get,
	DicomRequestType_Move,
	DicomRequestType_Store
};

class PacsdRequest {
private:
	OFCondition cond;
	OFCondition EchoScp (T_ASC_Association *assoc, T_DIMSE_Message *msg, T_ASC_PresentationContextID presID);

public:
	PacsdRequest () {};
	~PacsdRequest() {};
	OFCondition getRequest (T_ASC_Association *assoc);
};

#endif
