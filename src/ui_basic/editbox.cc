/*
 * Copyright (C) 2003-2016 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "ui_basic/editbox.h"

#include <limits>

#include <SDL_keycode.h>
#include <boost/format.hpp>

#include "graphic/font_handler1.h"
#include "graphic/rendertarget.h"
#include "graphic/text/bidi.h"
#include "graphic/text/font_set.h"
#include "graphic/text_constants.h"
#include "ui_basic/mouse_constants.h"

// TODO(GunChleoc): Arabic: Fix positioning for Arabic

namespace {

constexpr int kMargin = 4;

} // namespace


namespace UI {

struct EditBoxImpl {
	/**
	 * Font used for rendering text.
	 */
	/*@{*/
	std::string fontname;
	uint32_t    fontsize;
	/*@}*/

	/// Background tile style.
	const Image* background;

	/// Maximum number of characters in the input
	uint32_t maxLength;

	/// Current text in the box.
	std::string text;

	/// Position of the caret.
	uint32_t caret;

	/// Current scrolling offset to the text anchor position, in pixels
	int32_t scrolloffset;

	/// Alignment of the text. Vertical alignment is always centered.
	Align align;
};

EditBox::EditBox
	(Panel * const parent,
	 const int32_t x, const int32_t y, const uint32_t w,
	 const Image* background,
	 int font_size)
	:
	Panel(parent, x, y, w, UI::g_fh1->render(as_uifont(UI::g_fh1->fontset()->representative_character()),
														  font_size)->height() + 2),
	m_(new EditBoxImpl),
	history_active_(false),
	history_position_(-1)
{
	set_thinks(false);

	m_->background = background;
	m_->fontname = UI::g_fh1->fontset()->sans();
	m_->fontsize = font_size;

	// Set alignment to the UI language's principal writing direction
	m_->align = UI::g_fh1->fontset()->is_rtl() ? UI::Align::kCenterRight : UI::Align::kCenterLeft;
	m_->caret = 0;
	m_->scrolloffset = 0;
	// yes, use *signed* max as maximum length; just a small safe-guard.
	m_->maxLength = std::numeric_limits<int32_t>::max();

	set_handle_mouse(true);
	set_can_focus(true);
	set_handle_textinput();

	// Initialize history as empty string
	for (uint8_t i = 0; i < CHAT_HISTORY_SIZE; ++i)
		history_[i] = "";
}

EditBox::~EditBox()
{
	// place a destructor where the compiler can find the EditBoxImpl destructor
}

/**
 * \return the current text entered in the edit box
 */
const std::string & EditBox::text() const
{
	return m_->text;
}

/**
 * Set the current text in the edit box.
 *
 * The text is truncated if it is longer than the maximum length set by
 * \ref setMaxLength().
 */
void EditBox::set_text(const std::string & t)
{
	if (t == m_->text)
		return;

	bool caretatend = m_->caret == m_->text.size();

	m_->text = t;
	if (m_->text.size() > m_->maxLength)
		m_->text.erase(m_->text.begin() + m_->maxLength, m_->text.end());
	if (caretatend || m_->caret > m_->text.size())
		m_->caret = m_->text.size();
}


/**
 * \return the maximum length of the input string
 */
uint32_t EditBox::max_length() const
{
	return m_->maxLength;
}


/**
 * Set the maximum length of the input string.
 *
 * If the current string is longer than the new maximum length,
 * its end is cut off to fit into the maximum length.
 */
void EditBox::set_max_length(uint32_t const n)
{
	m_->maxLength = n;

	if (m_->text.size() > m_->maxLength) {
		m_->text.erase(m_->text.begin() + m_->maxLength, m_->text.end());
		if (m_->caret > m_->text.size())
			m_->caret = m_->text.size();

		check_caret();
	}
}


/**
 * The mouse was clicked on this editbox
*/
bool EditBox::handle_mousepress(const uint8_t btn, int32_t, int32_t)
{
	if (btn == SDL_BUTTON_LEFT && get_can_focus()) {
		focus();
		return true;
	}

	return false;
}
bool EditBox::handle_mouserelease(const uint8_t btn, int32_t, int32_t)
{
	return btn == SDL_BUTTON_LEFT && get_can_focus();
}

/**
 * Handle keypress/release events
 */
// TODO(unknown): Text input works only because code.unicode happens to map to ASCII for
// ASCII characters (--> // HERE). Instead, all user editable strings should be
// real unicode.
bool EditBox::handle_key(bool const down, SDL_Keysym const code)
{
	if (down) {
		switch (code.sym) {
		case SDLK_ESCAPE:
			cancel();
			return true;

		case SDLK_TAB:
			// Let the panel handle the tab key
			return false;

		case SDLK_KP_ENTER:
		case SDLK_RETURN:
			// Save history if active and text is not empty
			if (history_active_) {
				if (!m_->text.empty()) {
					for (uint8_t i = CHAT_HISTORY_SIZE - 1; i > 0; --i)
						history_[i] = history_[i - 1];
					history_[0] = m_->text;
					history_position_ = -1;
				}
			}
			ok();
			return true;

		case SDLK_KP_PERIOD:
			if (code.mod & KMOD_NUM) {
				break;
			}
			/* no break */
		case SDLK_DELETE:
			if (m_->caret < m_->text.size()) {
				while ((m_->text[++m_->caret] & 0xc0) == 0x80) {};
				// now handle it like Backspace
			} else
				return true;
			/* no break */
		case SDLK_BACKSPACE:
			if (m_->caret > 0) {
				while ((m_->text[--m_->caret] & 0xc0) == 0x80)
					m_->text.erase(m_->text.begin() + m_->caret);
				m_->text.erase(m_->text.begin() + m_->caret);
				check_caret();
				changed();
			}
			return true;

		case SDLK_KP_4:
			if (code.mod & KMOD_NUM) {
				break;
			}
			/* no break */
		case SDLK_LEFT:
			if (m_->caret > 0) {
				while ((m_->text[--m_->caret] & 0xc0) == 0x80) {};
				if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
					for (uint32_t new_caret = m_->caret;; m_->caret = new_caret)
						if (0 == new_caret || isspace(m_->text[--new_caret]))
							break;

				check_caret();
			}
			return true;

		case SDLK_KP_6:
			if (code.mod & KMOD_NUM) {
				break;
			}
			/* no break */
		case SDLK_RIGHT:
			if (m_->caret < m_->text.size()) {
				while ((m_->text[++m_->caret] & 0xc0) == 0x80) {};
				if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
					for (uint32_t new_caret = m_->caret;; ++new_caret)
						if
							(new_caret == m_->text.size()
							 ||
							 isspace(m_->text[new_caret - 1]))
						{
							m_->caret = new_caret;
							break;
						}

				check_caret();
			}
			return true;

		case SDLK_KP_7:
			if (code.mod & KMOD_NUM) {
				break;
			}
			/* no break */
		case SDLK_HOME:
			if (m_->caret != 0) {
				m_->caret = 0;

				check_caret();
			}
			return true;

		case SDLK_KP_1:
			if (code.mod & KMOD_NUM) {
				break;
			}
			/* no break */
		case SDLK_END:
			if (m_->caret != m_->text.size()) {
				m_->caret = m_->text.size();
				check_caret();
			}
			return true;

		case SDLK_KP_8:
			if (code.mod & KMOD_NUM) {
				break;
			}
			/* no break */
		case SDLK_UP:
			// Load entry from history if active and text is not empty
			if (history_active_) {
				if (history_position_ > CHAT_HISTORY_SIZE - 2)
					history_position_ = CHAT_HISTORY_SIZE - 2;
				if (history_[++history_position_].size() > 0) {
					m_->text = history_[history_position_];
					m_->caret = m_->text.size();
					check_caret();
				}
			}
			return true;

		case SDLK_KP_2:
			if (code.mod & KMOD_NUM) {
				break;
			}
			/* no break */
		case SDLK_DOWN:
			// Load entry from history if active and text is not equivalent to the current one
			if (history_active_) {
				if (history_position_ < 1)
					history_position_ = 1;
				if (history_[--history_position_] != m_->text) {
					m_->text = history_[history_position_];
					m_->caret = m_->text.size();
					check_caret();
				}
			}
			return true;

		default:
			break;
		}
	}

	return false;
}

bool EditBox::handle_textinput(const std::string& input_text) {
	if ((m_->text.size() +  input_text.length()) < m_->maxLength) {
		m_->text.insert(m_->caret, input_text);
		m_->caret += input_text.length();
		check_caret();
		changed();
	}
	return true;
}

void EditBox::draw(RenderTarget & odst)
{
	RenderTarget & dst = odst;

	// Draw the background
	dst.tile
		(Rect(Point(0, 0), get_w(), get_h()),
		 m_->background,
		 Point(get_x(), get_y()));

	// Draw border.
	if (get_w() >= kMargin && get_h() >= kMargin) {
		static const RGBColor black(0, 0, 0);

		// bottom edge
		dst.brighten_rect
			(Rect(Point(0, get_h() - 2), get_w(), 2),
			 BUTTON_EDGE_BRIGHT_FACTOR);
		// right edge
		dst.brighten_rect
			(Rect(Point(get_w() - 2, 0), 2, get_h() - 2),
			 BUTTON_EDGE_BRIGHT_FACTOR);
		// top edge
		dst.fill_rect(Rect(Point(0, 0), get_w() - 1, 1), black);
		dst.fill_rect(Rect(Point(0, 1), get_w() - 2, 1), black);
		// left edge
		dst.fill_rect(Rect(Point(0, 0), 1, get_h() - 1), black);
		dst.fill_rect(Rect(Point(1, 0), 1, get_h() - 2), black);
	}

	if (has_focus()) {
		dst.brighten_rect
			(Rect(Point(0, 0), get_w(), get_h()), MOUSE_OVER_BRIGHT_FACTOR);
	}

	const int max_width = get_w() - 2 * kMargin;

	const Image* entry_text_im = UI::g_fh1->render(as_editorfont(m_->text, m_->fontsize));

	const int linewidth = entry_text_im->width();
	const int lineheight = m_->text.empty() ?
							  UI::g_fh1->render(as_editorfont(UI::g_fh1->fontset()->representative_character(),
																		 m_->fontsize))->height() :
							  entry_text_im->height();

	Point point(kMargin, get_h() / 2);

	if (static_cast<int>(m_->align & UI::Align::kRight)) {
		point.x += max_width;
	}

	UI::correct_for_align(m_->align, linewidth, lineheight, &point);

	// Crop to max_width while blitting
	if (max_width < linewidth) {
		// Fix positioning for BiDi languages.
		if (UI::g_fh1->fontset()->is_rtl()) {
			point.x = 0;
		}
		// We want this always on, e.g. for mixed language savegame filenames
		if (i18n::has_rtl_character(m_->text.c_str(), 100)) { // Restrict check for efficiency
			// TODO(GunChleoc): Arabic: Fix scrolloffset
			dst.blitrect(point,
							 entry_text_im,
							 Rect(linewidth - max_width, 0, linewidth, lineheight));
		}
		else {
			if (static_cast<int>(m_->align & UI::Align::kRight)) {
				// TODO(GunChleoc): Arabic: Fix scrolloffset
				dst.blitrect(point,
								 entry_text_im,
								 Rect(point.x + m_->scrolloffset + kMargin, point.y, max_width, lineheight));
			} else {
				dst.blitrect(point,
								 entry_text_im,
								 Rect(point.x - m_->scrolloffset - kMargin, point.y, max_width, lineheight));
			}
		}
	} else {
		dst.blitrect(point, entry_text_im, Rect(0, 0, max_width, lineheight));
	}

	if (has_focus()) {
		// Draw the caret
		std::string line_to_caret = m_->text.substr(0, m_->caret);
		// TODO(GunChleoc): Arabic: Fix cursor position for BIDI text.
		int caret_x = text_width(line_to_caret, m_->fontsize);

		const uint16_t fontheight = text_height(m_->text, m_->fontsize);

		const Image* caret_image = g_gr->images().get("images/ui_basic/caret.png");
		Point caretpt;
		caretpt.x = point.x + m_->scrolloffset + caret_x - caret_image->width() + LINE_MARGIN;
		caretpt.y = point.y + (fontheight - caret_image->height()) / 2;
		dst.blit(caretpt, caret_image);
	}
}

/**
 * Check the caret's position and scroll it into view if necessary.
 */
void EditBox::check_caret()
{
	std::string leftstr(m_->text, 0, m_->caret);
	std::string rightstr(m_->text, m_->caret, std::string::npos);
	int32_t leftw = text_width(leftstr, m_->fontsize);
	int32_t rightw = text_width(rightstr, m_->fontsize);

	int32_t caretpos;

	switch (m_->align & UI::Align::kHorizontal) {
	case UI::Align::kRight:
		caretpos = get_w() - kMargin + m_->scrolloffset - rightw;
		break;
	default:
		caretpos = kMargin + m_->scrolloffset + leftw;
	}

	if (caretpos < kMargin)
		m_->scrolloffset += kMargin - caretpos + get_w() / 5;
	else if (caretpos > get_w() - kMargin)
		m_->scrolloffset -= caretpos - get_w() + kMargin + get_w() / 5;

	if ((m_->align & UI::Align::kHorizontal) == UI::Align::kLeft) {
		if (m_->scrolloffset > 0)
			m_->scrolloffset = 0;
	} else if ((m_->align & UI::Align::kHorizontal) == UI::Align::kRight) {
		if (m_->scrolloffset < 0)
			m_->scrolloffset = 0;
	}
}

}
