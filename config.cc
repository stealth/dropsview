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

#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdint>
#include "misc.h"


namespace dropsview {

using namespace std;


namespace config {

string home{""}, editor{"vi"}, tag{"global"};

map<string, uint64_t> message_status;

bool drawbox = 1;

}


int init_config()
{
	config::home = getenv("HOME");

	char *ed = getenv("EDITOR");
	if (ed)
		config::editor = ed;

	if (!config::home.size())
		return -1;

	string dv = config::home + "/.drops";
	mkdir(dv.c_str(), 0700);
	dv += "/view";
	mkdir(dv.c_str(), 0700);

	return 0;
}


}

