/*
	cat_story
	mperron (2017)
*/

#include <fstream>
#include <list>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include <time.h>

#include "common.h"

char dir_db[256] = DIR_DB_BASE DIR_DB_DEFAULT;

bool write_story(string fname){
	static regex regex_key("^\\s*([^\\s=]*)\\s*=\\s*(.*)$");
	smatch sm;
	map<string, string> headers;

	string path = dir_db + ("/" + fname);

	// Determine whether this is a valid story file.
	{
		struct stat sb;

		// File doesn't exist or we don't have access to it.
		if(stat(path.c_str(), &sb))
			return false;

		// Don't read directories as story files.
		if(S_ISDIR(sb.st_mode))
			return false;
	}

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

	return true;
}

int main(int argc, char **argv){
	// First argument is an optional alternative DB directory.
	if(argc >= 2){
		strcpy(dir_db, DIR_DB_BASE);
		strcat(dir_db, argv[1]);
	}

	string dbdir = (dir_db);
	DIR *dbs = opendir(dbdir.c_str());
	if(!dbs)
		return errorcode(1, "Could not open " DIR_DB_BASE "\n");

	list<string> files;
	map<string, string> query;

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

		// Write out the story. If this returns false, the file wasn't a story.
		if(!write_story(str))
			continue;

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
