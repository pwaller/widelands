/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#ifndef __S__EDITOR_TOOL_CHANGE_HEIGHT_OPTIONS_MENU
#define __S__EDITOR_TOOL_CHANGE_HEIGHT_OPTIONS_MENU

#include "editor_tool_options_menu.h"

class Editor_Interactive;
class Editor_Increase_Height_Tool;
class Editor_Decrease_Height_Tool;
class Editor_Set_Height_Tool;
namespace UI {struct Textarea;};

struct Editor_Tool_Change_Height_Options_Menu : public Editor_Tool_Options_Menu {
      Editor_Tool_Change_Height_Options_Menu(Editor_Interactive*, int, Editor_Increase_Height_Tool*,
            UI::UniqueWindow::Registry*);
      ~Editor_Tool_Change_Height_Options_Menu() { }

   private:
      void clicked(int);
      void update(void);
      UI::Textarea* m_increase, *m_set;
      Editor_Increase_Height_Tool* m_iht;
      Editor_Decrease_Height_Tool* m_dht;
      Editor_Set_Height_Tool* m_sht;
};

#endif
