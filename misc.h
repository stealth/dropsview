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

#ifndef dropsview_misc_h
#define dropsview_misc_h

#include <map>
#include <string>
#include <vector>
#include <curses.h>


namespace dropsview {


namespace status {

enum {
	read = 1
};

}


extern std::map<std::string, std::string> gMessages;

bool is_hex_hash(const std::string &);

std::map<std::string, std::string> list_personas();

void mark_as_read(const std::string &);

void load_message_status();

void refresh_inq(void);

int system(const std::vector<const char *> &);

int system(const std::vector<const char *> &, std::string &, std::string &);

int box(WINDOW *win, chtype verch, chtype horch);


}

#endif
