/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#include "ui_basic/button.h"

#include "graphic/font_handler1.h"
#include "graphic/image.h"
#include "graphic/rendertarget.h"
#include "graphic/text_constants.h"
#include "graphic/text_layout.h"
#include "ui_basic/mouse_constants.h"
#include "wlapplication.h"

namespace UI {

// Margin around image. The image will be scaled down to fit into this rectangle with preserving size.
constexpr int kButtonImageMargin = 2;

Button::Button //  for textual buttons. If h = 0, h will resize according to the font's height.
	(Panel * const parent,
	 const std::string & name,
	 int32_t const x, int32_t const y, uint32_t const w, uint32_t const h,
	 const Image* bg_pic,
	 const std::string & title_text,
	 const std::string & tooltip_text,
	 bool const init_enabled, bool const flat)
	:
	NamedPanel           (parent, name, x, y, w, h, tooltip_text),
	highlighted_   (false),
	pressed_       (false),
	permpressed_   (false),
	enabled_       (init_enabled),
	repeating_     (false),
	flat_          (flat),
	keep_image_size_(false),
	draw_flat_background_(false),
	time_nextact_  (0),
	title_         (title_text),
	pic_background_(bg_pic),
	pic_custom_    (nullptr),
	clr_down_      (229, 161, 2)
{
	// Automatically resize for font height and give it a margin.
	if (h < 1) {
		int new_height =
				UI::g_fh1->render(as_uifont(UI::g_fh1->fontset()->representative_character()))->height() + 4;
		set_desired_size(w, new_height);
		set_size(w, new_height);
	}
	set_thinks(false);
}

Button::Button //  for pictorial buttons
	(Panel * const parent,
	 const std::string & name,
	 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
	 const Image* bg_pic,
	 const Image* fg_pic,
	 const std::string & tooltip_text,
	 bool const init_enabled, bool const flat, const bool keep_image_size)
	:
	NamedPanel      (parent, name, x, y, w, h, tooltip_text),
	highlighted_   (false),
	pressed_       (false),
	permpressed_   (false),
	enabled_       (init_enabled),
	repeating_     (false),
	flat_          (flat),
	keep_image_size_(keep_image_size),
	draw_flat_background_(false),
	time_nextact_  (0),
	pic_background_(bg_pic),
	pic_custom_    (fg_pic),
	clr_down_      (229, 161, 2)
{
	set_thinks(false);
}


Button::~Button()
{
}


/**
 * Sets a new picture for the Button.
*/
void Button::set_pic(const Image* pic)
{
	title_.clear();

	if (pic_custom_ == pic)
		return;

	pic_custom_ = pic;
}


/**
 * Set a text title for the Button
*/
void Button::set_title(const std::string & title) {
	if (title_ == title)
		return;

	pic_custom_ = nullptr;
	title_      = title;
}


/**
 * Enable/Disable the button (disabled buttons can't be clicked).
 * Buttons are enabled by default
*/
void Button::set_enabled(bool const on)
{
	if (enabled_ == on)
		return;

	// disabled buttons should look different...
	if (on)
		enabled_ = true;
	else {
		if (pressed_) {
			pressed_ = false;
			set_thinks(false);
			grab_mouse(false);
		}
		enabled_ = false;
		highlighted_ = false;
	}
}


/**
 * Redraw the button
*/
void Button::draw(RenderTarget & dst)
{
	// Draw the background
	if (!flat_ || draw_flat_background_) {
		assert(pic_background_);
		dst.fill_rect(Rect(Point(0, 0), get_w(), get_h()), RGBAColor(0, 0, 0, 255));
		dst.tile(Rect(Point(0, 0), get_w(), get_h()), pic_background_, Point(get_x(), get_y()));
	}

	if (enabled_ && highlighted_ && !flat_)
		dst.brighten_rect
			(Rect(Point(0, 0), get_w(), get_h()), MOUSE_OVER_BRIGHT_FACTOR);

	//  If we've got a picture, draw it centered
	if (pic_custom_) {
		if (keep_image_size_) {
			if (enabled_) {
				//  ">> 1" is almost like "/ 2", but simpler for signed types (difference
				//  is that -1 >> 1 is -1 but -1 / 2 is 0).
				dst.blit(
							Point(
								(get_w() - static_cast<int32_t>(pic_custom_->width())) >> 1,
								(get_h() - static_cast<int32_t>(pic_custom_->height())) >> 1),
							pic_custom_);
			} else {
				//  ">> 1" is almost like "/ 2", but simpler for signed types (difference
				//  is that -1 >> 1 is -1 but -1 / 2 is 0).
				dst.blit_monochrome(
							Point(
								(get_w() - static_cast<int32_t>(pic_custom_->width())) >> 1,
								(get_h() - static_cast<int32_t>(pic_custom_->height())) >> 1),
							pic_custom_,
							RGBAColor(255, 255, 255, 127));
			}
		} else {
			const int max_image_w = get_w() - 2 * kButtonImageMargin;
			const int max_image_h = get_h() - 2 * kButtonImageMargin;
			double image_scale =
				std::min(1.,
							std::min(static_cast<double>(max_image_w) / pic_custom_->width(),
										static_cast<double>(max_image_h) / pic_custom_->height()));
			int blit_width = image_scale * pic_custom_->width();
			int blit_height = image_scale * pic_custom_->height();

			if (enabled_) {
				dst.blitrect_scale(
					Rect((get_w() - blit_width) / 2, (get_h() - blit_height) / 2, blit_width, blit_height),
					pic_custom_,
					Rect(0, 0, pic_custom_->width(), pic_custom_->height()),
					1.,
					BlendMode::UseAlpha);
			} else {
				dst.blitrect_scale_monochrome(
					Rect((get_w() - blit_width) / 2, (get_h() - blit_height) / 2, blit_width, blit_height),
					pic_custom_,
					Rect(0, 0, pic_custom_->width(), pic_custom_->height()),
					RGBAColor(255, 255, 255, 127));
			}
		}

	} else if (title_.length()) {
		//  Otherwise draw title string centered
		const Image* entry_text_im =
				autofit_ui_text(title_,
									 get_inner_w() - 2 * kButtonImageMargin,
									 enabled_ ? UI_FONT_CLR_FG : UI_FONT_CLR_DISABLED);

		dst.blit(Point((get_w() - entry_text_im->width()) / 2, (get_h() - entry_text_im->height()) / 2),
					entry_text_im);
	}

	//  draw border
	//  a pressed but not highlighted button occurs when the user has pressed
	//  the left mouse button and then left the area of the button or the button
	//  stays pressed when it is pressed once
	RGBAColor black(0, 0, 0, 255);

	// permpressed_ is true, we invert the behaviour on pressed_
	bool draw_pressed = permpressed_ ?    !(pressed_ && highlighted_)
	                                  :     (pressed_ && highlighted_);

	if (!flat_) {
		assert(2 <= get_w());
		assert(2 <= get_h());
		//  button is a normal one, not flat
		if (!draw_pressed) {
			//  top edge
			dst.brighten_rect
				(Rect(Point(0, 0), get_w(), 2), BUTTON_EDGE_BRIGHT_FACTOR);
			//  left edge
			dst.brighten_rect
				(Rect(Point(0, 2), 2, get_h() - 2), BUTTON_EDGE_BRIGHT_FACTOR);
			//  bottom edge
			dst.fill_rect(Rect(Point(2, get_h() - 2), get_w() - 2, 1), black);
			dst.fill_rect(Rect(Point(1, get_h() - 1), get_w() - 1, 1), black);
			//  right edge
			dst.fill_rect(Rect(Point(get_w() - 2, 2), 1, get_h() - 2), black);
			dst.fill_rect(Rect(Point(get_w() - 1, 1), 1, get_h() - 1), black);
		} else {
			//  bottom edge
			dst.brighten_rect
				(Rect(Point(0, get_h() - 2), get_w(), 2),
				 BUTTON_EDGE_BRIGHT_FACTOR);
			//  right edge
			dst.brighten_rect
				(Rect(Point(get_w() - 2, 0), 2, get_h() - 2),
				 BUTTON_EDGE_BRIGHT_FACTOR);
			//  top edge
			dst.fill_rect(Rect(Point(0, 0), get_w() - 1, 1), black);
			dst.fill_rect(Rect(Point(0, 1), get_w() - 2, 1), black);
			//  left edge
			dst.fill_rect(Rect(Point(0, 0), 1, get_h() - 1), black);
			dst.fill_rect(Rect(Point(1, 0), 1, get_h() - 2), black);
		}
	} else {
		//  Button is flat, do not draw borders, instead, if it is pressed, draw
		//  a box around it.
		if (enabled_ && highlighted_)
		{
			RGBAColor shade(100, 100, 100, 80);
			dst.fill_rect(Rect(Point(0, 0), get_w(), 2), shade);
			dst.fill_rect(Rect(Point(0, 2), 2, get_h() - 2), shade);
			dst.fill_rect(Rect(Point(0, get_h() - 2), get_w(), get_h()), shade);
			dst.fill_rect(Rect(Point(get_w() - 2, 0), get_w(), get_h()), shade);
		}
	}
}

void Button::think()
{
	assert(repeating_);
	assert(pressed_);
	Panel::think();

	if (highlighted_) {
		uint32_t const time = SDL_GetTicks();
		if (time_nextact_ <= time) {
			time_nextact_ += MOUSE_BUTTON_AUTOREPEAT_TICK; //  schedule next tick
			if (time_nextact_ < time)
				time_nextact_ = time;
			play_click();
			sigclicked();
			clicked();
			//  The button may not exist at this point (for example if the button
			//  closed the dialog that it is part of). So member variables may no
			//  longer be accessed.
		}
	}
}

/**
 * Update highlighted status
*/
void Button::handle_mousein(bool const inside)
{
	bool oldhl = highlighted_;

	highlighted_ = inside && enabled_;

	if (oldhl == highlighted_)
		return;

	if (highlighted_)
		sigmousein();
	else
		sigmouseout();
}


/**
 * Update the pressed status of the button
*/
bool Button::handle_mousepress(uint8_t const btn, int32_t, int32_t) {
	if (btn != SDL_BUTTON_LEFT)
		return false;

	if (enabled_) {
		grab_mouse(true);
		pressed_ = true;
		if (repeating_) {
			time_nextact_ =
				SDL_GetTicks() + MOUSE_BUTTON_AUTOREPEAT_DELAY;
			set_thinks(true);
		}
	}
	return true;
}

bool Button::handle_mouserelease(uint8_t const btn, int32_t, int32_t) {
	if (btn != SDL_BUTTON_LEFT)
		return false;

	if (pressed_) {
		pressed_ = false;
		set_thinks(false);
		grab_mouse(false);
		if (highlighted_ && enabled_) {
			play_click();
			sigclicked();
			clicked();
			//  The button may not exist at this point (for example if the button
			//  closed the dialog that it is part of). So member variables may no
			//  longer be accessed.
		}
	}
	return true;
}

bool Button::handle_mousemove(const uint8_t, int32_t, int32_t, int32_t, int32_t) {
	return true; // We handle this always by lighting up
}

void Button::set_perm_pressed(bool state) {
	if (state != permpressed_) {
		permpressed_ = state;
	}
}

void Button::set_flat(bool flat) {
	flat_ = flat;
}

void Button::set_draw_flat_background(bool set) {
	draw_flat_background_ = set;
}

}
