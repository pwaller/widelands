/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#ifndef __S__EDITOR_MAKE_INFRASTRUCTURE_TOOL_H
#define __S__EDITOR_MAKE_INFRASTRUCTURE_TOOL_H

#include "editor_tool.h"
#include "ui_unique_window.h"

/*
=============================
class Editor_Make_Infrastructure_Tool

this places immovables on the map
=============================
*/
struct Editor_Make_Infrastructure_Tool : public Editor_Tool {
	Editor_Make_Infrastructure_Tool() : Editor_Tool(*this, *this), m_player(1) {}

	void          set_player(const Player_Number n) throw () {m_player = n;}
	Player_Number get_player() const                throw () {return m_player;}

	int handle_click_impl(Map &, const Node_and_Triangle, Editor_Interactive &);
	const char * get_sel_impl() const throw ()
	{return "pics/fsel.png";} //  Standard sel icon, most complex tool of all

private:
	Player_Number m_player;
      UI::UniqueWindow::Registry m_registry;
};

int Editor_Make_Infrastructure_Tool_Callback(const TCoords, void *, int);

#endif
