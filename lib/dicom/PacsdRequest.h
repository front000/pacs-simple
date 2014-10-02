#ifndef _PACSD_REQUEST_H_
#define _PACSD_REQUEST_H_

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmnet/diutil.h"

enum DicomRequestType {
	DicomRequestType_Echo,
	DicomRequestType_Store,
	DicomRequestType_Move,
	DicomRequestType_Find,
	DicomRequestType_Get
};

class PacsdRequest {
private:
	OFCondition EchoScp (T_ASC_Association *assoc, T_DIMSE_Message *msg, T_ASC_PresentationContextID presID);
	OFString HRRequest (DicomRequestType request);

public:
	PacsdRequest () {};
	~PacsdRequest() {};
	OFCondition getRequest (T_ASC_Association *assoc);
};

#endif
