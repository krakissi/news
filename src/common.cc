/*
	common
	mperron (2018)
*/

#include "common.h"

int errorcode(int code, const char *msg, ...){
	cerr << msg;

	return code;
}

string format_time(string fname){
	string result = "";

	size_t n = 256;
	char *s = (char*) calloc(n, sizeof(char));

	long t;
	try {
		t = stol(fname);
	} catch(...){
		return fname;
	}

	struct tm *tm = localtime(&t);

	strftime(s, n, "%x %T", tm);

	result.append(s);
	free(s);
	return result;
}

void query_parse(map<string, string> &query){
	static regex regex_query("([^&]+)");
	static regex regex_splitter("^([^=]*)=(.*)$");

	smatch sm;
	char *str = getenv("QUERY_STRING");
	if(!str)
		return;
	string str_search(str);

	while(regex_search(str_search, sm, regex_query)){
		string str_inner = sm[1];
		smatch sm_inner;

		if(regex_match(str_inner, sm_inner, regex_splitter))
			query[sm_inner[1]] = sm_inner[2];
		else
			query[str_inner] = "";

		str_search = sm.suffix();
	}
}
