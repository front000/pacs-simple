#ifndef _SUBROUTINE_H_
#define _SUBROUTINE_H_

std::string HRRequest (DicomRequestType request) {
	switch (request) {
		case DicomRequestType_Echo:
			return "C-ECHO";

		case DicomRequestType_Store:
			return "C-STORE";

		case DicomRequestType_Move:
			return "C-MOVE";

		case DicomRequestType_Find:
			return "C-FIND";

		case DicomRequestType_Get:
			return "C-GET";

		default:
			return "unknown command";
	}
}

#endif
