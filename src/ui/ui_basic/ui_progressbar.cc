/*
 * Copyright (C) 2004, 2006-2007 by the Widelands Development Team
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

#include <cstdio>
#include "font_handler.h"
#include "rendertarget.h"
#include "rgbcolor.h"
#include "types.h"
#include "ui_progressbar.h"
#include "constants.h"

namespace UI {
/**
Initialize the progress bar.
*/
Progress_Bar::Progress_Bar(Panel* parent, int x, int y, int w, int h, uint orientation)
	: Panel(parent, x, y, w, h)
{
	m_orientation = orientation;

	m_state = 0;
	m_total = 100;
}


/**
Set the current state of progress.
*/
void Progress_Bar::set_state(uint state)
{
	m_state = state;

	update(0, 0, get_w(), get_h());
}


/**
Set the maximum state
*/
void Progress_Bar::set_total(uint total)
{
	assert(total);
	m_total = total;

	update(0, 0, get_w(), get_h());
}


/**
Draw the progressbar.
*/
void Progress_Bar::draw(RenderTarget* dst)
{
	assert(0 < get_w());
	assert(0 < get_h());
	assert(m_total);
	const float fraction =
		m_state < m_total ? static_cast<const float>(m_state) / m_total : 1.0;
	assert(0 <= fraction);
	assert     (fraction <= 1);

	const RGBColor color = fraction <= 0.15 ?
		RGBColor(255, 0, 0)
		:
		fraction <= 0.5 ? RGBColor(255, 255, 0) : RGBColor(0, 255, 0);

	// Draw the actual bar
	if (m_orientation == Horizontal)
	{
		const uint w = static_cast<const uint>(get_w() * fraction);
		assert(w <= static_cast<const uint>(get_w()));

		dst->fill_rect(Rect(Point(0, 0), w, get_h()), color);
		dst->fill_rect
			(Rect(Point(w, 0), get_w() - w, get_h()), RGBColor(0, 0, 0));
	}
	else
	{
		const uint h = static_cast<const uint>(get_h() * (1.0 - fraction));

		dst->fill_rect(Rect(Point(0, 0), get_w(), h), RGBColor(0, 0, 0));
		dst->fill_rect(Rect(Point(0, h), get_w(), get_h() - h), color);
	}

	// Print the state in percent
	char buf[30];

	snprintf(buf, sizeof(buf), "%u%%", static_cast<const uint>(fraction * 100));

	g_fh->draw_string
		(*dst,
		 UI_FONT_SMALL, UI_FONT_SMALL_CLR,
		 Point(get_w() / 2, get_h() / 2),
		 buf,
		 Align_Center);
}
};
