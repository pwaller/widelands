/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include "widelands.h"
#include "graphic.h"
#include "ui_basic.h"
#include "fullscreen_menu_base.h"
#include "game.h"
#include "fullscreen_menu_mapselect.h"
#include "fullscreen_menu_launchgame.h"
#include "map.h"
#include "player.h"
#include "playerdescrgroup.h"

// hard-coded playercolors
uchar g_playercolors[MAX_PLAYERS][12] = {
	{ // blue
		  0,   0, 165,
		  0,  55, 190,
		  0, 120, 215,
		  0, 210, 245
	},
	{ // red
		119,  19,   0,
		166,  27,   0,
		209,  34,   0,
		255,  41,   0
	},
	{ // yellow
		112, 103,   0,
		164, 150,   0,
		209, 191,   0,
		255, 232,   0
	},
	{ // green
		 26,  99,   1,
		 37, 143,   2,
		 48, 183,   3,
		 59, 223,   3
	},
	{ // black/dark gray
		  0,   0,   0,
		 19,  19,  19,
		 35,  35,  35,
		 57,  57,  57
	},
	{ // orange
		119,  80,   0,
		162, 109,   0,
		209, 141,   0,
		255, 172,   0,
	},
	{ // purple
		 91,   0,  93,
		139,   0, 141,
		176,   0, 179,
		215,   0, 218,
	},
	{ // white
		119, 119, 119,
		166, 166, 166,
		210, 210, 210,
		255, 255, 255
	}
};


PlayerDescriptionGroup::PlayerDescriptionGroup(UIPanel* parent, int x, int y, Game* game, int plnum)
	: UIPanel(parent, x, y, 300, 20)
{
	m_game = game;
	m_plnum = plnum;
	
	m_enabled = false;
	set_visible(false);

	// create sub-panels
	m_btnEnablePlayer = new UICheckbox(this, 0, 0);
	m_btnEnablePlayer->set_state(true);
	m_btnEnablePlayer->changedto.set(this, &PlayerDescriptionGroup::enable_player);
	
	m_btnPlayerType = new UIButton(this, 28, 0, 174, 20, 1);
	m_btnPlayerType->clicked.set(this, &PlayerDescriptionGroup::toggle_playertype);
	if (plnum==1)
		m_playertype = Player::playerLocal;
	else
		m_playertype = Player::playerAI;
}

/** PlayerDescriptionGroup::set_enabled(bool enable)
 *
 + The group is enabled if the map has got a starting position for this player.
 * We need to update the Game class accordingly.
 */
void PlayerDescriptionGroup::set_enabled(bool enable)
{
	if (enable == m_enabled)
		return;
	
	m_enabled = enable;
	
	if (!m_enabled)
	{
		if (m_btnEnablePlayer->get_state())
			m_game->remove_player(m_plnum);
		set_visible(false);
	}
	else
	{
		if (m_btnEnablePlayer->get_state())
			m_game->add_player(m_plnum, m_playertype, "romans", g_playercolors[m_plnum-1]);
			
		const char* string = 0;
		switch(m_playertype) {
		case Player::playerLocal: string = "Human"; break;
		case Player::playerAI: string = "Computer"; break;
		}
		m_btnPlayerType->set_title(string);
		m_btnPlayerType->set_visible(m_btnEnablePlayer->get_state());
		
		set_visible(true);
	}
	
	changed.call();
}

/** PlayerDescriptionGroup::enable_player(bool on)
 *
 * Update the Game when the checkbox is changed.
 */
void PlayerDescriptionGroup::enable_player(bool on)
{
	if (on) {
		m_game->add_player(m_plnum, m_playertype, "romans", g_playercolors[m_plnum-1]);
	} else {
		m_game->remove_player(m_plnum);
	}

	m_btnPlayerType->set_visible(on);
	changed.call();
}

void PlayerDescriptionGroup::toggle_playertype()
{
	// NOOP: toggling the player type is currently not possible
}

