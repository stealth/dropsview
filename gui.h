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

#ifndef dropsview_menu_h
#define dropsview_menu_h

#include <map>
#include <vector>
#include <string>
#include <curses.h>
#include <menu.h>


namespace dropsview {


typedef enum {
	eBanner = -1, ePersonas, eMessages, eDrops, eMAX
} MenuNo;

#define okMenuNo(n) (((n) > eBanner) && ((n) < eMAX))

#define MENU_Y	1


class MenuData {

	std::map<std::string, std::string> content;

public:
	char *title{nullptr};

	void (*func)(int){nullptr};
	unsigned int mask{0};
	MenuNo menu;

	MenuData()
	{
	}

	MenuData(char *s1, void (*f)(int), unsigned int m, MenuNo mn)
		: title(s1), func(f), mask(m), menu(mn)
	{
	}

	virtual ~MenuData()
	{
		// this is a strdup() thats stored via new_item() and our responsibility to free()
		free(title);
	}

	std::string get_content(const std::string &key)
	{
		// for RVO
		std::string s = "";

		auto it = content.find(key);
		if (it == content.end())
			return s;
		s = it->second;
		return s;
	}

	void set_content(const std::string &key, const std::string &value)
	{
		content[key] = value;
	}

};



class GUI {

private:

	std::string d_err{""};

	MENU *d_mBanner{nullptr}, *d_mDrops{nullptr}, *d_mPersonas{nullptr}, *d_mMessages{nullptr};
	WINDOW *d_status{nullptr}, *d_content{nullptr}, *d_contentbox{nullptr};


	int menu_getc(MENU *m)
	{
		return wgetch(menu_win(m));
	}

	int menu_offset(MenuNo number);

	MENU *menu_create(ITEM **items, int count, int ncols, MenuNo number);

	void menu_destroy(MENU *);

	MENU *current_menu(void);

	MENU *build_persona_menu(const std::map<std::string, std::string> &, void (*func)(int), MenuNo mn);

	MENU *build_message_menu(const std::map<std::string, std::string> &, void (*func)(int), MenuNo mn);

	MENU *build_vector_menu(const std::vector<std::string> &v, void (*f)(int), MenuNo mn);

	void handle_command(int ch, MENU *menu);

	void menu_display(MENU *m)
	{
		touchwin(menu_win(m));
		wrefresh(menu_win(m));
	}

	int perform_drops_menu(int cmd)
	{
		return menu_driver(d_mDrops, cmd);
	}

	int perform_persona_menu(int cmd)
	{
		return menu_driver(d_mPersonas, cmd);
	}


	int perform_inbox_menu(int cmd)
	{
		return menu_driver(d_mMessages, cmd);
	}

	int menu_number(void)
	{
		return item_index(current_item(d_mBanner)) - (eBanner + 1);
	}

	template<class T>
	T build_error(const std::string &msg, T r)
	{
		d_err = "GUI::";
		d_err += msg;
		if (errno) {
			d_err += ":";
			d_err += strerror(errno);
		}
		errno = 0;
		return r;
	}


public:

	GUI()
	{
	}

	virtual ~GUI()
	{
		menu_destroy(d_mDrops);
		menu_destroy(d_mPersonas);
		menu_destroy(d_mMessages);
		menu_destroy(d_mBanner);

		delwin(d_status);
		delwin(d_content);
		delwin(d_contentbox);

		endwin();
	}

	GUI(const GUI &) = delete;

	GUI &operator=(const GUI &) = delete;

	const char *why()
	{
		return d_err.c_str();
	}

	int init();

	void perform_menus(void);

	WINDOW *window(const std::string &name)
	{
		if (name == "content")
			return d_content;
		else if (name == "contentbox")
			return d_contentbox;
		else if (name == "status")
			return d_status;
		return nullptr;
	}

};


extern GUI *gui;


}

#endif

