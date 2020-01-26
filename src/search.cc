/*
	search
	mperron (2018)
*/

#include <iostream>
#include <string>
#include <map>
#include <list>
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include "common.h"

using namespace std;

char dir_db[256] = DIR_DB_BASE DIR_DB_DEFAULT;

void check_match(string file, string query, map<string, int> &matches){
	static regex regex_query(REG_QUERY_PRE + query + REG_QUERY_POST, std::regex_constants::icase);
	static regex regex_key(REG_KEY);

	map<string, string> headers;
	smatch sm, qm;

	string path = dir_db + ("/" + file);

	// Determine whether this is a valid story file.
	{
		struct stat sb;

		// File doesn't exist or we don't have access to it.
		if(stat(path.c_str(), &sb))
			return;

		// Don't read directories as story files.
		if(S_ISDIR(sb.st_mode))
			return;
	}

	// Check line-by-line for matches
	ifstream in(path);
	string buffer;

	// Parse headers.
	while(in.good()){
		getline(in, buffer);

		// On an empty line, assume we've reached the end of the headers.
		if(buffer.length() == 0)
			break;

		if(regex_match(buffer, sm, regex_key)){
			string header = sm[2];
			headers[sm[1]] = header;

			if(regex_match(header, qm, regex_query)){
				// Double priority to header matches.
				matches[file] += 2;
			}
		}
	}

	while(in.good()){
		getline(in, buffer);

		if(regex_match(buffer, qm, regex_query))
			matches[file]++;
	}
}

void print_preview(string file){
	static regex regex_key(REG_KEY);

	string path = dir_db + ("/" + file);
	map<string, string> headers;
	smatch sm;

	// Check line-by-line for matches
	ifstream in(path);
	string buffer;

	// Turn the Unix time stamp into a presentable post date.
	string time = format_time(file);

	// Parse headers.
	while(in.good()){
		getline(in, buffer);

		// On an empty line, assume we've reached the end of the headers.
		if(buffer.length() == 0)
			break;

		if(regex_match(buffer, sm, regex_key)){
			string header = sm[2];
			headers[sm[1]] = header;
		}
	}

	cout << "<!-- " << file << ": " << time << " -->" << endl;
	cout << "<article class=story>" << endl;

	{
		string title = "untitled";

		try {
			title = headers.at(HDR_TITLE);
		} catch(...){
		}

		cout << "<header><h2><a href=\"?s=" << file << "\">" << title << "</a></h2></header>" << endl;
	}

	while(in.good()){
		getline(in, buffer);

		// 
	}

	cout << "<footer><span class=dateline><time>" << time << "</time></span></footer>" <<endl;
	cout << "</article>" << endl;
}

int main(int argc, char **argv){
	map<string, string> query_map;
	string query = "";

	// Find the database directory for this news site.
	if(argc >= 2){
		strcpy(dir_db, DIR_DB_BASE);
		strcat(dir_db, argv[1]);
	}
	string dbdir = (dir_db);

	// Parse the QUERY_STRING splitting on ampersands and obtaining key/value pairs.
	query_parse(query_map);
	try {
		query = query_map.at(QRY_SEARCH);
	} catch(...){
		cerr << "No query provided. Expecting no results." << endl;
	}

	map<string, int> matches;

	// Search for the specified string.
	if(query.length() > 0){
		DIR *dbs = opendir(dbdir.c_str());
		if(!dbs)
			return errorcode(1, "Could not open " DIR_DB_BASE "\n");

		list<string> files;
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

		// Check every file for a match.
		for(string file : files)
			check_match(file, query, matches);

		// Summarize results
		if(matches.size() > 0){
			cout << "Found " << matches.size() << " matches." << endl;

			for(auto pair : matches){
				cout << pair.first << ": " << format_time(pair.first) << " - " << pair.second << " match weight." << endl;
			}
		}


		// Print each preview
		for(auto pair : matches){
			// TODO build ordered structure based on match strength
			print_preview(pair.first);
		}
	}

	return 0;
}
