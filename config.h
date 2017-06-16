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

#ifndef dropsview_config_h
#define dropsview_config_h

#include <string>
#include <map>
#include <cstdint>
#include "misc.h"

namespace dropsview {

namespace config {

extern std::string home;
extern std::string editor, tag;

extern std::map<std::string, uint64_t> message_status;

}



int init_config();

}


#endif

