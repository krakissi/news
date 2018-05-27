/*
	nav_bar
	mperron (2018)
*/

#include <list>

#include <sys/stat.h>
#include <dirent.h>

#include "common.h"

char dir_db[256] = DIR_DB_BASE DIR_DB_DEFAULT;

bool is_story(string fname){
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

	return true;
}

int main(int argc, char **argv){
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

	if(hardstop){
		// Showing a specific date range. Nothing to do in this mode.
	} else {
		string id_newest = "", id_newer = "", id_older = "", id_oldest = "";
		bool found_single = false;

		for(auto str : files){
			// Skip files that aren't stories.
			if(!is_story(str))
				continue;

			// Set the newest file to the first one that we see.
			if(id_newest == "")
				id_newest = str;

			// If this is a single post lookup, skip files that don't match the name.
			if(single_post){
				if(post_id == str)
					found_single = true;
				else {
					if(!found_single)
						id_newer = str;
					else if(id_older == "")
						id_older = str;

					id_oldest = str;
				}
			} else {
				// TODO
				/*
				// If this is a range post lookup, skip until we're in-bounds.
				if(bounded_posts && !bounded_in){
					if(post_id_start == str)
						bounded_in = true;
					else
						continue;
				}

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
				*/
			}
		}

		if(single_post){
			// Write out story newest/newer/older/oldest controls
			// TODO
		} else {
			// Write out page change controls
			// TODO
		}
	}

	return 0;
}
