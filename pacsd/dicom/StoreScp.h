#ifndef _STORESCP_H_
#define _STORESCP_H_

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmnet/diutil.h"

OFCondition StoreScp (
	T_ASC_Association *assoc,
	T_DIMSE_Message *msg,
	T_ASC_PresentationContextID presID
);

#endif
