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

#include <cassert>
#include "ui_unique_window.h"

/*
==============================================================================

UniqueWindow IMPLEMENTATION

==============================================================================
*/


/**
In order to avoid dangling pointers, we need to kill our contained window here.
*/
UIUniqueWindowRegistry::~UIUniqueWindowRegistry()
{
	if (window)
		delete window;
}


/**
Register, position according to the registry information.
*/
UIUniqueWindow::UIUniqueWindow(UIPanel* parent, UIUniqueWindowRegistry* reg, int w, int h, std::string title)
	: UIWindow(parent, 0, 0, w, h, title.c_str())
{
	m_registry = reg;
	m_usedefaultpos = true;

	if (m_registry)
	{
		if (m_registry->window)
			delete m_registry->window;

		m_registry->window = this;
		if (m_registry->x >= 0) {
			set_pos(m_registry->x, m_registry->y);
			m_usedefaultpos = false;
		}
	}
}


/**
Unregister, save latest position.
*/
UIUniqueWindow::~UIUniqueWindow()
{
	if (m_registry) {
		assert(m_registry->window == this);

		m_registry->window = 0;
		m_registry->x = get_x();
		m_registry->y = get_y();
	}
}
