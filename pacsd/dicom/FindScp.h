#ifndef _FINDSCP_
#define _FINDSCP_

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmnet/diutil.h"

OFCondition FindScp (
	T_ASC_Association *assoc,
	T_DIMSE_Message *msg,
	T_ASC_PresentationContextID presID
);

#endif
