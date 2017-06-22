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

#include <string>
#include <iostream>
#include <locale.h>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "gui.h"
#include "config.h"
#include "misc.h"


using namespace std;
using namespace dropsview;


void usage(const char *p)
{
	cerr<<"\nUsage: "<<p<<" -T [tag] [-b]\n\n";
	exit(-1);
}


int main(int argc, char *argv[])
{
	int c;

	while ((c = getopt(argc, argv, "T:b")) != -1) {
		switch (c) {
		case 'T':
			config::tag = optarg;
			break;
		case 'b':
			config::drawbox = 0;
			break;
		default:
			usage(argv[0]);
		}
	}

	if (config::tag != "global" && !is_hex_hash(config::tag)) {
		cerr<<"Error: Invalid tag specified.\n";
		return -1;
	}

	init_config();
	load_message_status();

	umask(077);

	setlocale(LC_ALL, "");
	gui = new (nothrow) GUI;

	if (gui->init() < 0) {
		cerr<<"Error: "<<gui->why()<<endl;
		delete gui;
		return -1;
	} else
		gui->perform_menus();

	cerr<<"Error: "<<gui->why()<<endl;
	delete gui;
	return -1;
}

