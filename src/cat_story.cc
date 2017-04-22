/*
	cat_story
	mperron (2017)
*/

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <map>
#include <regex>

#include <sys/types.h>
#include <dirent.h>

#include <time.h>

using namespace std;

#define DIR_DB_S "db/s"
#define MAX_PAGE_COUNT 10

// Prefix HDR_: headers used in the preamble of a story file.
#define HDR_TITLE "title"

// Prefix QRY_: Query string parameter names.
#define QRY_START "a"
#define QRY_END "b"
#define QRY_COUNT "c"
#define QRY_SINGLE "s"

int error_code(int code, const char *msg, ...){
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

void write_story(string fname){
	static regex regex_key("^\\s*([^\\s=]*)\\s*=\\s*(.*)$");
	smatch sm;
	map<string, string> headers;

	string path = ((DIR_DB_S "/") + fname);
	ifstream in(path);
	string buffer;

	// Turn the Unix time stamp into a presentable post date.
	string time = format_time(fname);

	// Parse headers.
	while(in.good()){
		getline(in, buffer);

		// On an empty line, assume we've reached the end of the headers.
		if(buffer.length() == 0)
			break;

		if(regex_match(buffer, sm, regex_key))
			headers[sm[1]] = sm[2];
	}

	// Write out the story.
	cout << "<!-- " << fname << ": " << time << " -->" << endl;
	cout << "<div class=story>" << endl;
	cout << "<span class=dateline>" << time << "</span>" <<endl;

	try {
		cout << "<h2><a href=\"?s=" << fname << "\">" << headers.at(HDR_TITLE) << "</a></h2>" << endl;
	} catch(...){
		cout << "<!-- untitled -->" << endl;
	}

	while(in.good()){
		getline(in, buffer);
		cout << "\t" <<  buffer << endl;
	}
	cout << "</div>" << endl;
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

int main(int argc, char **argv){
	DIR *dbs = opendir(DIR_DB_S);
	string b;

	list<string> files;
	map<string, string> query;

	query_parse(query);

	// Single post lookup
	bool single_post = false;
	string post_id;
	try {
		post_id = query.at(QRY_SINGLE);
		single_post = true;
	} catch(...){
	}

	// Range of posts lookup
	bool bounded_posts = false;
	bool bounded_in = false;
	string post_id_start;
	string post_id_stop;
	int post_count;
	bool hardstop = false;
	try {
		post_id_start = query.at(QRY_START);

		try {
			post_id_stop = query.at(QRY_END);
			hardstop = true;
		} catch(...){
		}

		try {
			post_count = stol(query.at(QRY_COUNT));
		} catch(...){
			post_count = MAX_PAGE_COUNT;
		}

		bounded_posts = true;
	} catch(...){
	}

	if(!dbs)
		return error_code(1, "Could not open " DIR_DB_S "\n");

	int count = 0;
	struct dirent *entry;
	while(entry = readdir(dbs)){
		// Skip dot files
		if(entry->d_name[0] == '.')
			continue;

		files.push_back(entry->d_name);
	}
	closedir(dbs);

	files.sort();
	files.reverse();

	cout << endl;

	for(auto str : files){
		// If this is a single post lookup, skip files that don't match the name.
		if(single_post && (post_id != str))
			continue;

		// If this is a range post lookup, skip until we're in-bounds.
		if(bounded_posts && !bounded_in){
			if(post_id_start == str)
				bounded_in = true;
			else
				continue;
		}

		write_story(str);
		count++;

		// Hard limit to the number of posts on a page.
		if(count >= MAX_PAGE_COUNT)
			break;

		// Stop processing, we're at the boundary.
		if(bounded_posts && bounded_in){
			if(hardstop && (post_id_stop == str))
				break;

			if(count >= post_count)
				break;
		}
	}

	// No posts found!
	if(!count)
		cout << "<div><p><i>... dust ...</i></p></div>" << endl;

	cout << endl;

	return 0;
}
