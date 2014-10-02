#ifndef _STORESCP_H_
#define _STORESCP_H_

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmnet/diutil.h"

OFBool storeInstanceFile (DcmDataset *dset);
OFString genSavePath (DcmDataset *dset);
OFString genFilename (OFString &savepath);

OFCondition StoreScp (
	T_ASC_Association *assoc,
	T_DIMSE_Message *msg,
	T_ASC_PresentationContextID presID
);

#endif
