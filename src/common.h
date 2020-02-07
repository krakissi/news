/*
	common
	mperron (2018)

	Functions and base variables that are shared between tools in this module.
*/

#include <string>
#include <map>
#include <iostream>
#include <regex>

#define DIR_DB_BASE "db/"
#define MAX_PAGE_COUNT 10
#define DIR_DB_DEFAULT "s"

// Prefix HDR_: headers used in the preamble of a story file.
#define HDR_TITLE "title"
#define HDR_HIDDEN "hidden"

// Prefix REG_: regular expressions
#define REG_KEY "^\\s*([^\\s=]*)\\s*=\\s*(.*)$"
#define REG_QUERY_PRE "^.*"
#define REG_QUERY_POST ".*$"

// Prefix QRY_: Query string parameter names.
#define QRY_START "a"
#define QRY_END "b"
#define QRY_COUNT "c"
#define QRY_SINGLE "s"
#define QRY_SEARCH "q"

using namespace std;

int errorcode(int code, const char *msg, ...);

string format_time(string fname);

void query_parse(map<string, string> &query);
