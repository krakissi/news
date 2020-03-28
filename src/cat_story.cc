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

bool write_story(string fname, bool single_post){
	static regex regex_key(REG_KEY);
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

	// If this is not a "single post" and the story is set to hidden, skip it.
	if(!single_post){
		try {
			if(headers.at(HDR_HIDDEN) == "true")
				return false;
		} catch(...){
		}
	}

	// Write out the story.
	cout << "<!-- " << fname << ": " << time << " -->" << endl;
	cout << "<article class=\"story story_id_" << fname << "\">" << endl;

	try {
		cout << "<header><h2><a href=\"?s=" << fname << "\">" << headers.at(HDR_TITLE) << "</a></h2></header>" << endl;
	} catch(...){
		cout << "<!-- untitled -->" << endl;
	}

	while(in.good()){
		getline(in, buffer);

		// Story break. If we're in a single post, stop here and link to the
		// story. Otherwise continue.
		if(buffer == "!!!!"){
			if(single_post)
				continue;

			cout << "<span class=pagebreak_link><a href=\"?s=" << fname << "\">Read More...</a></span>" << endl;
			break;
		}

		cout << "\t" <<  buffer << endl;
	}

	cout << "<footer><span class=dateline><time>" << time << "</time></span></footer>" <<endl;
	cout << "</article>" << endl;

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
		try {
			post_count = stol(query.at(QRY_COUNT));
		} catch(...){
			post_count = MAX_PAGE_COUNT;
		}

		post_id_start = query.at(QRY_START);

		try {
			post_id_stop = query.at(QRY_END);
			hardstop = true;
		} catch(...){
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
		if(!write_story(str, single_post))
			continue;

		count++;

		// Hard limit to the number of posts on a page.
		if(count >= post_count)
			break;

		// Stop processing, we're at the boundary.
		if(bounded_posts && bounded_in){
			if(hardstop && (post_id_stop == str))
				break;
		}
	}

	// No posts found!
	if(!count)
		cout << "<div><p><i>... dust ...</i></p></div>" << endl;

	cout << endl;

	return 0;
}
