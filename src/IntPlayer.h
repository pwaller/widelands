/*
 * Copyright (C) 2002, 2003 by The Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __S__INTPLAYER_H
#define __S__INTPLAYER_H

#include "game.h"
#include "ui_basic.h"
#include "interactive_base.h"

class Player;
class MiniMap;
class Map_View;
class UIWindow;
class CoordPath;

/** class Interactive_Player
 *
 * This is the interactive player. this one is
 * responsible to show the correct map
 * to the player and draws the user interface,
 * cares for input and so on.
 */
class Interactive_Player : public Interactive_Base {
	public:
		Interactive_Player(Game *g, uchar pln);
		~Interactive_Player(void);

		virtual void think();

		void start();

		void exit_game_btn();
		void main_menu_btn();
		void toggle_buildhelp();

		void field_action();

      void recalc_overlay(FCoords fc) ;

		bool handle_key(bool down, int code, char c);

		inline Game *get_game() { return m_game; }
		inline uchar get_player_number(void) { return m_player_number; }
		inline Player *get_player() { assert(m_game); return m_game->get_player(m_player_number) ; }

		// Road building
		inline bool is_building_road() const { return m_buildroad; }
		inline CoordPath *get_build_road() { return m_buildroad; }
		void start_build_road(Coords start);
		void abort_build_road();
		void finish_build_road();
		bool append_build_road(Coords field);
		const Coords &get_build_road_start();
		const Coords &get_build_road_end();
		int get_build_road_end_dir();

	private:
		void roadb_add_overlay();
		void roadb_remove_overlay();

		Game*		m_game;
		uchar		m_player_number;

		UITextarea*	m_label_speed;

		UIUniqueWindowRegistry	m_mainmenu;
		UIUniqueWindowRegistry	m_fieldaction;

		CoordPath		*m_buildroad; // path for the new road
};


#endif // __S__INTPLAYER_H
