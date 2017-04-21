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

int error_code(int code, const char *msg, ...){
	cerr << msg;

	return code;
}

string format_time(string fname){
	string result = "";

	size_t n = 256;
	char *s = (char*) calloc(n, sizeof(char));
	long t = stol(fname);
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

	for(auto x : headers){
		// FIXME debug
		cout << "<!-- " << x.first << ": " << x.second << " -->" << endl;
	}

	// Write out the story.
	cout << "<!-- " << fname << ": " << time << " -->" << endl;
	cout << "<div class=story>" << endl;
	cout << "<span class=dateline>" << time << "</span>" <<endl;

	try {
		cout << "<h2>" << headers.at("title") << "</h2>" << endl;
	} catch(...){
		cout << "<!-- untitled -->" << endl;
	}

	while(in.good()){
		getline(in, buffer);
		cout << "\t" <<  buffer << endl;
	}
	cout << "</div>" << endl;
}

int main(int argc, char **argv){
	DIR *dbs = opendir(DIR_DB_S);
	string b;

	list<string> files;

	if(!dbs)
		return error_code(1, "Could not open " DIR_DB_S "\n");

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
	for(auto str : files)
		write_story(str);
	cout << endl;

	return 0;
}
