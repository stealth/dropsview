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

/* Some functions (menu_virtualize, menu_offset, menu_create and
 * perform_menus) were taken (and some modified) from the ncurses demo samples,
 * demo_menus.c, which has the following (C) notice:
 *
 * $Id: demo_menus.c,v 1.54 2014/09/05 08:34:06 tom Exp $
 *
 * Demonstrate a variety of functions from the menu library.
 * Thomas Dickey - 2005/4/9
 */
/*
 * Copyright (c) 1998-2014,2015 Free Software Foundation, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, distribute with modifications, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization.
 */


#include <string>
#include <vector>
#include <memory>
#include <map>
#include <locale.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <curses.h>
#include <menu.h>
#include "gui.h"
#include "config.h"
#include "misc.h"


namespace dropsview {


GUI *gui{nullptr};

using namespace std;


void call_drops(int);

static char empty[1];


static int menu_virtualize(int c)
{
	int result;

	if (c == '\n' || c == KEY_EXIT)
		result = (MAX_COMMAND + 1);
	else if (c == 'u')
		result = (REQ_SCR_ULINE);
	else if (c == 'd')
		result = (REQ_SCR_DLINE);
	else if (c == 'b' || c == KEY_NPAGE)
		result = (REQ_SCR_UPAGE);
	else if (c == 'f' || c == KEY_PPAGE)
		result = (REQ_SCR_DPAGE);
	else if (c == 'l' || c == KEY_LEFT || c == KEY_BTAB)
		result = (REQ_LEFT_ITEM);
	else if (c == 'n' || c == KEY_DOWN)
		result = (REQ_NEXT_ITEM);
	else if (c == 'p' || c == KEY_UP)
		result = (REQ_PREV_ITEM);
	else if (c == 'r' || c == KEY_RIGHT || c == '\t')
		result = (REQ_RIGHT_ITEM);
	else if (c == ' ')
		result = (REQ_TOGGLE_ITEM);
	else {
		if (c != KEY_MOUSE)
			beep();
		result = (c);
	}
	return result;
}


void call_menus(int code)
{
}


void call_persona(int code)
{
}


int GUI::init()
{
	initscr();
	noraw();
	cbreak();
	noecho();
	curs_set(0);

	d_status = newwin(3, COLS, LINES - 3, 0);
	d_contentbox = newwin(LINES - 4, COLS - 40, 1, 40);
	d_content = derwin(d_contentbox, LINES - 6, COLS - 42, 1, 1);

	if (!d_status || !d_content || !d_contentbox)
		return build_error("Failed to create ncurses menus.", -1);

	d_mBanner = build_vector_menu({"personas", config::tag, "drops"}, call_menus, eBanner);
	auto personas = list_personas();
	d_mPersonas = build_persona_menu(personas, call_persona, ePersonas);
	refresh_inq();
	d_mMessages = build_message_menu(gMessages, nullptr, eMessages);
	d_mDrops = build_vector_menu({"exit"}, call_drops, eDrops);
	if (!d_mBanner || !d_mPersonas || !d_mMessages || !d_mDrops)
		return build_error("Failed to create ncurses menus.", -1);

	dropsview::box(d_contentbox, 0, 0);
	wrefresh(d_contentbox);

	return 0;
}


int GUI::menu_offset(MenuNo number)
{
	int result = 0;

	if (okMenuNo(number)) {
		int spc_desc, spc_rows, spc_cols;

#ifdef NCURSES_VERSION
		menu_spacing(d_mBanner, &spc_desc, &spc_rows, &spc_cols);
#else
		spc_rows = 0;
#endif

#define menu_itemwidth(menu) (menu)->itemlen
		/* FIXME: MENU.itemlen seems the only way to get actual width of items */
		result =
		    (number - (eBanner + 1)) * (menu_itemwidth(d_mBanner) +
						spc_rows);
	}
	return result;
}



void dropsview_item_init(MENU *menu)
{
	WINDOW *content = gui->window("content");
	WINDOW *contentbox = gui->window("contentbox");
	WINDOW *status = gui->window("status");

	ITEM *item = current_item(menu);
	MenuData *md = (MenuData *)item_userptr(item);

	werase(content);

	if (md->menu == ePersonas) {
		string s = md->get_content("pub_pem") + "\n";
		s += "keytype: " + md->get_content("keytype") + "\n";
		s += "has private part: " + md->get_content("private") + "\n";

		mvwprintw(status, 1, 1, "%s %s", md->get_content("id").c_str(), md->get_content("name").c_str());
		mvwprintw(content, 0, 0, "%s", s.c_str());
	} else if (md->menu == eMessages) {
		mvwprintw(status, 1, 1, "%s", md->get_content("id").c_str());
		mvwprintw(content, 0, 0, "%s", md->get_content("message").c_str());
	} else if (md->menu == eBanner)
		mvwprintw(status, 1, 1, "");

	touchwin(contentbox);
	wclrtoeol(status);
	dropsview::box(status, 0, 0);
	wrefresh(status);
	wrefresh(content);
}



MENU *GUI::menu_create(ITEM **items, int count, int ncols, MenuNo number)
{
	MENU *result;
	WINDOW *menuwin;
	int mrows, mcols;
	int y = okMenuNo(number) ? MENU_Y : 0;
	int x = menu_offset(number);

	// center menus to the left, so that content window
	// doesnt clash with menu window
	if (number != eBanner)
		x = 1;

	int margin = (y == MENU_Y) ? 1 : 0;
	int maxcol = (ncols + x) < COLS ? ncols : (COLS - x - 1);
	int maxrow = (count + 1) / ncols;

	if ((maxrow + y) >= (LINES - 5))
		maxrow = LINES - 5 - y;

	result = new_menu(items);

	set_menu_format(result, maxrow, maxcol);
	scale_menu(result, &mrows, &mcols);

	if (mcols + (2 * margin + x) >= COLS)
		mcols = COLS - (2 * margin + x);

//	menuwin = newwin(mrows + (2 * margin), mcols + (2 * margin), y, x);
	if (number != eBanner)
		menuwin = newwin(LINES - 4, 40, 1, 0);
	else
		menuwin = newwin(mrows + (2 * margin), mcols + (2 * margin), 0, 0);

	set_menu_win(result, menuwin);
	keypad(menuwin, TRUE);
	if (margin)
		dropsview::box(menuwin, 0, 0);

	set_menu_sub(result, derwin(menuwin, mrows, mcols, margin, margin));
	post_menu(result);

	//set_menu_init(result, dropsview_item_init);
	//set_menu_term(result, my_menu_term);
	set_item_init(result, dropsview_item_init);
	//set_item_term(result, my_item_term);
	return result;
}


void GUI::menu_destroy(MENU *m)
{
	if (m) {
		ITEM **items = menu_items(m);
		free_menu(m);
		unpost_menu(m);

		for (auto ip = items; *ip; ++ip) {
			delete static_cast<MenuData *>(item_userptr(*ip));
			delete *ip;
		}

		delete [] items;
	}
}



MENU *GUI::current_menu(void)
{
	MENU *result = nullptr;

	switch (menu_number()) {
	case eDrops:
		result = d_mDrops;
		break;
	case ePersonas:
		result = d_mPersonas;
		break;
	case eMessages:
		result = d_mMessages;
		break;
	}

	return result;
}


MENU *GUI::build_persona_menu(const map<string, string> &m, void (*func)(int), MenuNo mn)
{
	char pub_pem[65000] = {0};

	string dir = config::home;
	dir += "/.opmsg/";

	int fd = -1;
	struct stat st;

	ITEM **items = new (nothrow) ITEM*[m.size() + 1], **ip = items;
	unsigned int i = 0;

	for (auto it = m.begin(); it != m.end(); ++it) {
		string s = it->first.substr(0, 16);
		s += " ";
		s += it->second.substr(0, 20);

		// must be a strdup()
		MenuData *md = new (nothrow) MenuData(strdup(s.c_str()), func, i, mn);
		md->set_content("id", it->first);
		md->set_content("name", it->second);

		*ip = new_item(md->title, empty);
		set_item_userptr(*ip, md);
		++ip;
		++i;

		string path = dir + it->first + "/rsa.pub.pem", privpath = dir + it->first;
		fd = open(path.c_str(), O_RDONLY);
		if (fd < 0) {
			path = dir + it->first + "/ec.pub.pem";
			if ((fd = open(path.c_str(), O_RDONLY)) < 0)
				continue;

			md->set_content("keytype", "EC");
			privpath += "/ec.priv.pem";
		} else {
			md->set_content("keytype", "RSA");
			privpath += "/rsa.priv.pem";
		}

		memset(&st, 0, sizeof(st));
		if (fstat(fd, &st) < 0 || st.st_size >= (off_t)sizeof(pub_pem)) {
			close(fd);
			continue;
		}

		memset(pub_pem, 0, sizeof(pub_pem));
		if (read(fd, pub_pem, st.st_size) != st.st_size) {
			close(fd);
			continue;
		}

		close(fd);
		md->set_content("pub_pem", string(pub_pem, st.st_size));

		if (stat(privpath.c_str(), &st) == 0)
			md->set_content("private", "true");
		else
			md->set_content("private", "false");
	}

	*ip = nullptr;

	if (d_mPersonas) {
		menu_destroy(d_mPersonas);
		d_mPersonas = nullptr;
	}

	auto *menu = menu_create(items, m.size(), 1, mn);
	set_menu_mark(menu, ">");
	return menu;
}


MENU *GUI::build_message_menu(const map<string, string> &m, void (*func)(int), MenuNo mn)
{
	ITEM **items = new (nothrow) ITEM*[m.size() + 1], **ip = items;
	unsigned int i = 0;

	for (auto it = m.begin(); it != m.end(); ++it) {
		string s = "   " + it->first;
		char *title = strdup(s.c_str());
		MenuData *md = new (nothrow) MenuData(title, func, i, mn);

		if (config::message_status[it->first] & status::read) {
			title[0] = 'R';
			md->set_content("status", "r");
		}

		md->set_content("message", it->second);
		md->set_content("id", it->first);

		*ip = new_item(title, empty);
		set_item_userptr(*ip, md);
		++ip;
		++i;
	}

	*ip = nullptr;

	if (d_mMessages) {
		menu_destroy(d_mMessages);
		d_mMessages = nullptr;
	}

	auto *menu = menu_create(items, m.size(), 1, mn);
	set_menu_mark(menu, ">");
	return menu;
}


MENU *GUI::build_vector_menu(const vector<string> &v, void (*f)(int), MenuNo mn)
{
	ITEM **items = new (nothrow) ITEM*[v.size() + 1], **ip = items;
	unsigned int n = 0;

	for (auto it = v.begin(); it != v.end(); ++it) {
		char *title = strdup(it->c_str());

		MenuData *md = new (nothrow) MenuData(title, f, n, mn);
		++n;
		*ip = new_item(title, empty);
		set_item_userptr(*ip, md);
		++ip;
	}
	*ip = nullptr;

	auto *m = menu_create(items, v.size(), mn == eBanner ? v.size() : 1, mn);
	set_menu_mark(m, ">");
	return m;
}


void GUI::handle_command(int ch, MENU *menu)
{
	const char *c = keyname(ch);
	ITEM *item = current_item(menu);
	MenuData *md = (MenuData *)item_userptr(item);

	if (menu_number() == ePersonas) {
		if (*c == 'q' || *c == 'v') {
			char tmpl[256] = {0};
			snprintf(tmpl, sizeof(tmpl) - 1, "%s/.drops/view/tmp_drops.XXXXXX", config::home.c_str());
			int tmp_fd = mkstemp(tmpl);
			if (tmp_fd < 0)
				return;
			string cmd = "";
			const string &pub_pem = md->get_content("pub_pem");

			if (*c == 'q') {
				cmd = "qrencode -t UTF8 -o ";
				cmd += tmpl;

				FILE *f = nullptr;
				if (!(f = popen(cmd.c_str(), "w"))) {
					close(tmp_fd);
					return;
				}

				fwrite(pub_pem.c_str(), pub_pem.size(), 1, f);
				fclose(f);
			} else {
				write(tmp_fd, pub_pem.c_str(), pub_pem.size());
			}

			close(tmp_fd);

			endwin();
			system({config::editor.c_str(), tmpl, nullptr});
			refresh();

			unlink(tmpl);

		} else if (*c == 's') {
			char tmpl[256] = {0};
			snprintf(tmpl, sizeof(tmpl) - 1, "%s/.drops/view/tmp_drops.XXXXXX", config::home.c_str());
			int tmp_fd = mkstemp(tmpl);
			if (tmp_fd < 0)
				return;

			const char *slash = strrchr(tmpl, '/');
			if (!slash)
				return;
			++slash;

			char outfile[256] = {0};
			snprintf(outfile, sizeof(outfile) - 1, "%s/.drops/%s/outq/%s.opmsg", config::home.c_str(), config::tag.c_str(), slash);

			string out, err;

			endwin();
			system({config::editor.c_str(), tmpl, nullptr});
			system({"opmsg", "-E", md->get_content("id").c_str(), "-i", tmpl, "-o", outfile, nullptr}, out, err);
			refresh();

			unlink(tmpl);
			close(tmp_fd);

			werase(d_content);
			mvwprintw(d_content, 0, 0, "%s", err.c_str());
			wclrtoeol(d_content);
			touchwin(d_contentbox);
			wrefresh(d_content);
		// reFresh
		} else if (*c == 'f') {
			auto personas = list_personas();
			d_mPersonas = build_persona_menu(personas, call_persona, ePersonas);
			wrefresh(menu_win(d_mPersonas));
		}
	} else if (menu_number() == eMessages) {
		if (*c == 'v' || *c == 'b') {
			bool burn = (*c == 'b');

			char tmpl[256] = {0};
			snprintf(tmpl, sizeof(tmpl) - 1, "%s/.drops/view/tmp_drops.XXXXXX", config::home.c_str());
			int tmp_fd = mkstemp(tmpl);
			if (tmp_fd < 0)
				return;

			string infile = config::home;
			infile += "/.drops/";
			infile += config::tag;
			infile += "/inq/";
			infile += md->get_content("id");

			// mark as read, if not already
			if (md->get_content("status").size() == 0) {
				md->title[0] = 'R';
				mark_as_read(md->get_content("id"));
				md->set_content("status", "r");
			}

			string out, err;
			if (burn)
				system({"opmsg", "-D", "-i", infile.c_str(), "--burn", nullptr}, out, err);
			else
				system({"opmsg", "-D", "-i", infile.c_str(), nullptr}, out, err);

			if (out.size() > 0) {
				write(tmp_fd, out.c_str(), out.size());

				endwin();
				system({config::editor.c_str(), tmpl, nullptr});
				refresh();
			}

			close(tmp_fd);
			unlink(tmpl);

			if (burn)
				unlink(infile.c_str());

			werase(d_content);
			mvwprintw(d_content, 0, 0, "%s", err.c_str());
			wclrtoeol(d_content);
			touchwin(d_contentbox);
			wrefresh(d_content);
		// reFresh
		} else if (*c == 'f') {
			refresh_inq();
			d_mMessages = build_message_menu(gMessages, nullptr, eMessages);
			wrefresh(menu_win(d_mMessages));
		}
	}
}


void GUI::perform_menus(void)
{
	MENU *this_menu;
	MENU *last_menu = d_mPersonas;
	int code = E_UNKNOWN_COMMAND;
	int cmd;
	int ch = ERR;

	menu_display(last_menu);
	touchwin(d_contentbox);
	refresh();

	for (;;) {

		dropsview::box(d_status, 0, 0);
		wrefresh(d_status);
		wrefresh(d_contentbox);
		menu_display(current_menu());
		refresh();

		if (ch != ERR)
			handle_command(ch, last_menu);

		ch = menu_getc(d_mBanner);
		cmd = menu_virtualize(ch);

		switch (cmd) {
			/*
			 * The banner menu acts solely to select one of the other menus.
			 * Move between its items, wrapping at the left/right limits.
			 */
		case REQ_LEFT_ITEM:
		case REQ_RIGHT_ITEM:
			code = menu_driver(d_mBanner, cmd);
			if (code == E_REQUEST_DENIED) {
				if (menu_number() > 0)
					code =
					    menu_driver(d_mBanner,
							REQ_FIRST_ITEM);
				else
					code =
					    menu_driver(d_mBanner,
							REQ_LAST_ITEM);
			}
			break;
		default:
			switch (menu_number()) {
			case eDrops:
				code = perform_drops_menu(cmd);
				break;
			case ePersonas:
				code = perform_persona_menu(cmd);
				break;
			case eMessages:
				code = perform_inbox_menu(cmd);
				break;
			}

			if ((code == E_REQUEST_DENIED) && (cmd == KEY_MOUSE)) {
				code = menu_driver(d_mBanner, cmd);
			}

			break;
		}

		if (code == E_OK) {
			this_menu = current_menu();
			if (this_menu != last_menu) {
				move(1, 0);
				clrtobot();
				dropsview::box(menu_win(this_menu), 0, 0);
				refresh();

				/* force the current menu to appear */
				menu_display(this_menu);

				last_menu = this_menu;
			}
		}
		wrefresh(menu_win(last_menu));
		if (code == E_UNKNOWN_COMMAND || code == E_NOT_POSTED) {
			ITEM *item = current_item(last_menu);
			MenuData *md = (MenuData *)item_userptr(item);
			if (md->func)
				md->func((int)md->mask);
		}
		if (code == E_REQUEST_DENIED)
			beep();
		continue;
	}
}


void call_drops(int code)
{
	switch (code) {
	case 0:
		delete gui;
		exit(0);
	}
}


}

