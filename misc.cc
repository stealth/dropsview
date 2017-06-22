/*
 * This file is part of dropsview.
 *
 * (C) 2017 by Sebastian Krahmer,
 *             sebastian [dot] krahmer [at] gmail [dot] com
 *
 * dropsview is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * dropsview is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with dropsview.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <poll.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <map>
#include <vector>
#include <string>
#include <curses.h>
#include "misc.h"
#include "config.h"


namespace dropsview {

using namespace std;


map<string, string> gMessages{};


// only lowercase hex
bool is_hex_hash(const string &s)
{
	if (s.size() % 2 != 0 || s.size() < 2)
		return 0;

	for (string::size_type i = 0; i < s.size(); ++i) {
		if (!((s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'f')))
			return 0;
	}
	return 1;
}


map<string, string> list_personas()
{
	map<string, string> pers;

	string out = "", err = "", line = "";
	system({"opmsg", "-L", "--long", nullptr}, out, err);

	string::size_type idx1 = 0, idx2 = 0;
	for (;;) {
		idx2 = out.find("\n", idx1);
		if (idx2 == string::npos)
			break;
		line = out.substr(idx1, idx2 - idx1);
		idx1 = idx2 + 1;

		if (line.find("uid:u::::1:0:") == string::npos)
			continue;
		string id = line.substr(13);
		idx2 = id.find("::");
		if (idx2 == string::npos)
			continue;
		string name = id.substr(idx2 + 2);
		id.erase(idx2);
		idx2 = name.find(":");
		if (idx2 == string::npos)
			continue;
		name.erase(idx2);
		if (!is_hex_hash(id))
			continue;

		pers[id] = name;
	}

	if (pers.empty())
		pers["00"] = "no personas";

	return pers;
}


void refresh_inq()
{
	string dir = config::home;
	dir += "/.drops/";
	dir += config::tag;
	dir += "/inq/";

	struct dirent *de = nullptr;
	struct stat st;

	DIR *dp = opendir(dir.c_str());
	if (!dp) {
		if (gMessages.empty())
			gMessages["00 empty inq"] = "00 empty inq";
		return;
	}

	int fd = -1;
	ssize_t r = 0;
	while ((de = readdir(dp))) {
		string path = dir;
		if (gMessages.count(de->d_name) > 0)
			continue;

		if (!is_hex_hash(de->d_name))
			continue;

		path += de->d_name;
		if ((fd = open(path.c_str(), O_RDONLY)) < 0)
			continue;

		if (fstat(fd, &st) < 0) {
			close(fd);
			continue;
		}

		char *buf = new (nothrow) char[st.st_size];
		r = read(fd, buf, st.st_size);
		if (r > 0) {
			// strip off long signature line for viewing
			char *nl = strchr(buf, '\n');
			if (nl)
				gMessages[de->d_name] = string(nl, r - (nl - buf));
		}

		delete [] buf;
		close(fd);
	}

	closedir(dp);

	if (gMessages.empty())
		gMessages["00 empty inq"] = "00 empty inq";

}


int system(const vector<const char *> &v)
{
	pid_t pid = 0;

	if ((pid = fork()) == 0) {
		execvp(v[0], const_cast<char *const*>(&v[0]));
		exit(1);
	} else if (pid < 0)
		return -1;

	waitpid(pid, nullptr, 0);
	return 0;
}


// system() w/o sh -c
int system(const vector<const char *> &v, string &out, string &err)
{
	pid_t pid = 0;
	int op[2], ep[2];

	out = err = "";

	if (pipe(op) < 0 || pipe(ep) < 0)
		return -1;

	if ((pid = fork()) == 0) {
		close(op[0]); close(ep[0]);
		dup2(op[1], 1);
		dup2(ep[1], 2);
		close(op[1]); close(ep[1]);

		execvp(v[0], const_cast<char *const*>(&v[0]));
		exit(1);
	} else if (pid < 0)
		return -1;

	close(op[1]); close(ep[1]);

	char buf[4096] = {0};
	ssize_t r = 0;

	pollfd pfds[2];
	memset(pfds, 0, sizeof(pfds));

	pfds[0].fd = op[0];
	pfds[1].fd = ep[0];
	pfds[0].events = pfds[1].events = POLLIN;

	for (; pfds[0].events != 0 || pfds[1].events != 0;) {

		if (poll(pfds, 2, 0) <= 0)
			continue;

		for (int i = 0; i < 2; ++i) {
			if (pfds[i].revents) {
				r = read(pfds[i].fd, buf, sizeof(buf));
				if (r <= 0)
					pfds[i].events = 0;
				else {
					if (pfds[i].fd == op[0])
						out += string(buf, r);
					else
						err += string(buf, r);
				}
			}
		}

	}

	waitpid(pid, nullptr, 0);

	close(op[0]); close(ep[0]);

	return 0;
}



void mark_as_read(const string &id)
{
	config::message_status[id] = status::read;

	string path = config::home + "/.drops/view/messages.status";

	FILE *f = fopen(path.c_str(), "a");
	if (!f)
		return;

	fprintf(f, "%s:r,\n", id.c_str());
	fclose(f);

	return;
}


void load_message_status()
{
	string path = config::home + "/.drops/view/messages.status";
	FILE *f = fopen(path.c_str(), "r");
	if (!f)
		return;

	char buf[256] = {0}, *ptr = nullptr;
	string line = "", msgid = "";
	string::size_type idx = string::npos;

	for (;!feof(f);) {
		memset(buf, 0, sizeof(buf));
		ptr = fgets(buf, sizeof(buf) - 1, f);
		if (!ptr)
			break;
		string line = buf;
		if ((idx = line.find(":")) == string::npos)
			continue;
		msgid = line.substr(0, idx);
		config::message_status[msgid] = status::read;
	}

	fclose(f);
	return;
}


int box(WINDOW *w, chtype verch, chtype horch)
{
	if (config::drawbox)
		return ::box(w, verch, horch);

	return 0;
}


}

