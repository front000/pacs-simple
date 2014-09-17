#include "db_mysql.h"
#include <iostream>
#include <boost/regex.hpp>
#include <sstream>

Database::Database () {
	//connection = connect ();
}

Database::~Database() {
	disconnect ();
}

MYSQL *Database::connect () {
	mysql_init (&mysql);

	connection = mysql_real_connect (
		/* handle */ &mysql,
		/* host */ "green-swamp.ru",
		/* user */ "pacs1",
		/* pass */ "JfzpjuFRva",
		/* name */ "db_pacs1",
		/* port */ 3306,
		0, 0
	);

	if (connection == NULL)
		std::cerr << "Could not connect to MySQL: " << mysql_error (&mysql) << std::endl;
	
	return connection;
}

void Database::disconnect () {
	if (connection) mysql_close (connection);
}

int Database::set (OFString query) {
	int state = mysql_query (connection, query.c_str ());
	if (state)
		std::cerr << "Could not execute query '" << query << "': " << mysql_error (connection) << std::endl;

	return state;
}

void Database::get (std::vector<std::vector<OFString> > &data, OFString query) {
	if (set (query)) {
		data.clear ();
	} else {
		result = mysql_store_result (connection);
		std::size_t num_fields = mysql_num_fields (result);

		std::vector<OFString> temp;
		temp.reserve (num_fields);
		while ((row = mysql_fetch_row (result)) != NULL) {
			std::size_t i;
			for (i = 0; i < num_fields; i++) {
				temp.push_back (row[i]);
			}
			data.push_back (temp);
			temp.clear ();
		}

		mysql_free_result (result);
		// don't closing connection after set/get
	}
}

OFString Database::CFindSQLQuery (DcmDataset *requestIdentifiers, OFString QueryLevel, std::vector<OFString> &keys) {
	OFString sql_select = "SELECT patient.SpecificCharacterSet";
	OFString sql_from = "FROM patient, study, series, object";
	OFString sql_where = "WHERE patient.id = study.patient_id AND study.id = series.study_id AND series.id = object.series_id";
	std::vector<OFString> q; // User defined SQL WHERE

	// TODO: change sql_select via QueryLevel
	DcmStack stack;
	DcmObject *object = NULL;
	OFBool next_ = OFTrue;
	
	boost::smatch what;
	OFString tagValue, tbl; // tbl - SQL table
	
	std::vector<OFString> CFindKeys;
	getCFindKeys (CFindKeys);

	stack.clear ();
	while (requestIdentifiers->nextObject (stack, next_).good ()) {
		object = stack.top ();
		DcmTag tag = object->getTag ();
		OFString tagName = tag.getTagName ();

		std::stringstream ss;
		ss << tagName;
		std::string tn = ss.str ();

		/* Unsupported C-FIND key */
		if (!isCFindKey (CFindKeys, tagName))
			continue;

		/* Patient level */
		std::string pattern = "^(Patient|SpecificCharacterSet)";
		if (boost::regex_search (tn, what, boost::regex (pattern)))
			tbl = "patient";

		/* Study level */
		pattern = "^(Study|AccessionNumber)";
		if (boost::regex_search (tn, what, boost::regex (pattern)))
			tbl = "study";

		/* Series level */
		pattern = "^(Series|Modality)";
		if (boost::regex_search (tn, what, boost::regex (pattern)))
			tbl = "series";

		/* Image level */
		pattern = "^(SOP|Content|InstanceNumber)";
		if (boost::regex_search (tn, what, boost::regex (pattern)))
			tbl = "object";

		sql_select += ", " + tbl + "." + tagName;
		// OFString (DcmTagKey)
		OFString SpecificCharacterSet ("SpecificCharacterSet");
		keys.push_back (SpecificCharacterSet);
		if (!(std::find (keys.begin (), keys.end (), tagName) != keys.end ()))
			keys.push_back (tagName);

		if (!object->isEmpty ()) {
			// TODO: sql_select increment if QueryRootLevel == tbl
			sql_select += ", " + tbl + "." + tagName; // select this too

			requestIdentifiers->findAndGetOFString (tag.getXTag (), tagValue);
			OFString qv;
			if (tagName == "PatientName") {
				qv = tbl + "." + tagName + " LIKE \"" + tagValue + "%\""; // <-- Fucking API! I hate u screening!
			} else {
				qv = tbl + "." + tagName + " = \"" + tagValue + "\""; // <-- Fucking API! I hate u screening!
			}
			q.push_back (qv);
			qv.clear ();
		}
	}

	if (q.size () > 0) {
		std::vector<OFString>::const_iterator it = q.begin ();
		for (it; it < q.end (); ++it)
			sql_where += " AND " + *it;
	}

	return sql_select + " " + sql_from + " " + sql_where;
}
