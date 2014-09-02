#ifndef _MOVESCP_H_
#define _MOVESCP_H_

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmnet/diutil.h"

OFCondition MoveScp (
	T_ASC_Association *assoc,
	T_DIMSE_Message *msg,
	T_ASC_PresentationContextID presID
);

#endif
