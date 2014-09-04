#ifndef _DB_MYSQL_H_
#define _DB_MYSQL_H_

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmnet/diutil.h"
#include "../dicom/FindScp.h"
#include <mysql/mysql.h>
#include <vector>
#include <string>

class Database {
private:
	MYSQL *connection, mysql;
	MYSQL_RES *result;
	MYSQL_ROW row;

	// TODO: load config to connect
	std::vector<std::string> CFindKeys;

public:
	Database ();
	~Database();

	MYSQL *connect (); 
	void disconnect ();
	int set (OFString query);
	void get (std::vector<std::vector<OFString> > &data, OFString query);

	/* Making list of available dicom tags */
	void initDicomCFindKeys ();

	/* Constructs sql query */
	OFString CFindSQLQuery (DcmDataset *requestIdentifiers, OFString QueryLevel, std::vector<OFString> &keys);
};

#endif
