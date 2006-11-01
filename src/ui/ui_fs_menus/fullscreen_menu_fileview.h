/*
 * Copyright (C) 2002, 2006 by the Widelands Development Team
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

#ifndef __S__FILE_VIEW_SCREEN_H
#define __S__FILE_VIEW_SCREEN_H

#include "fullscreen_menu_base.h"

#include "ui_unique_window.h"

#include <string>

namespace UI {
struct Multiline_Textarea;
struct Panel;
};

void fileview_window(UI::Panel* parent, UI::UniqueWindow::Registry* reg, std::string filename);

/**
 * Shows a Text in a Fullscreen Menu. Waits for the button Ok to be clicked.
*/
struct Fullscreen_Menu_TextView : public Fullscreen_Menu_Base {
	Fullscreen_Menu_TextView(std::string filename);

protected:
   void set_text(std::string text);

private:
   UI::Multiline_Textarea* mlta;
};

/**
 * Shows an ASCII-File in a Fullscreen Menu. Waits for the button Ok to be clicked.
 */
class Fullscreen_Menu_FileView : public Fullscreen_Menu_TextView {
   public:
      Fullscreen_Menu_FileView(std::string filename );
};



#endif // __S__FILE_VIEW_SCREEN_H
