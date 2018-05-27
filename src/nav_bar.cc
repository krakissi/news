/*
	nav_bar
	mperron (2018)
*/

#include <list>
#include <vector>

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
		bounded_in = true;
	}

	cout << endl;

	if(hardstop){
		// Showing a specific date range. Nothing to do in this mode.
	} else {
		bool found_single = false;
		vector<string> posts;
		int index_first = 0, index_last = -1, c = 0;

		for(auto str : files){
			// Skip files that aren't stories.
			if(!is_story(str))
				continue;

			if(single_post){
				// Find the index of the story we're displaying.
				if(post_id == str)
					index_first = c;
			} else if(bounded_posts){
				// Find the index of the first story on the page.
				if(post_id_start == str){
					bounded_in = true;
					index_first = c;
				}
			}

			posts.push_back(str);
			c++;

			if(bounded_in){
				// Find the index of the last story on the page.
				if((c - index_first) == post_count){
					bounded_in = false;
					index_last = (c - 1);
				}
			}
		}

		cout << "<div class=page_nav>" << endl;

		if(single_post){
			string id_newest = "", id_newer = "", id_older = "", id_oldest = "";

			// Newest link
			if(index_first > 0){
				id_newest = posts[0];

				cout << "\t<span class=page_newest><a href=\"?s=" << id_newest << "\">newest</a></span>" << endl;
			}

			// Newer link
			if(index_first > 1){
				id_newer = posts[(index_first - 1)];

				cout << "\t<span class=page_newer><a href=\"?s=" << id_newer << "\">newer</a></span>" << endl;
			}

			// Older link
			if(index_first < (c - 2)){
				id_older = posts[(index_first + 1)];

				cout <<"\t<span class=page_older><a href=\"?s=" << id_older << "\">older</a></span>" << endl;
			}

			// Oldest link
			if(index_first < (c - 1)){
				id_oldest = posts[(c - 1)];

				cout <<"\t<span class=page_oldest><a href=\"?s=" << id_oldest << "\">oldest</a></span>" << endl;
			}
		} else {
			string id_newer = "", id_older = "";

			// Newer link
			if(index_first > 0){
				id_newer = posts[((index_first >= post_count) ? (index_first - post_count) : 0)];

				// Write out the link
				cout << "\t<span class=page_newer><a href=\"?a=" << id_newer;
				if(post_count != MAX_PAGE_COUNT)
					cout << "&c=" << post_count;
				cout << "\">newer</a></span>" << endl;
			}

			// Older link
			if((index_last > 0) && (index_last < (c - 1))){
				id_older = posts[(index_last + 1)];

				// Write out the link.
				cout << "\t<span class=page_older><a href=\"?a=" << id_older;
				if(post_count != MAX_PAGE_COUNT)
					cout << "&c=" << post_count;
				cout << "\">older</a></span>" << endl;
			}
		}

		cout << "</div>" << endl;
	}

	return 0;
}
