/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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

#include "graphic/graphic.h"

#include <cstring>
#include <iostream>
#include <memory>

#include <SDL_image.h>

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "build_info.h"
#include "config.h"
#include "graphic/animation.h"
#include "graphic/diranimations.h"
#include "graphic/font_handler.h"
#include "graphic/gl/system_headers.h"
#include "graphic/image.h"
#include "graphic/image_io.h"
#include "graphic/image_transformations.h"
#include "graphic/rendertarget.h"
#include "graphic/screen.h"
#include "graphic/terrain_texture.h"
#include "graphic/texture.h"
#include "graphic/texture_cache.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/streamwrite.h"
#include "logic/roadtype.h"
#include "notifications/notifications.h"
#include "ui_basic/progresswindow.h"

using namespace std;

Graphic * g_gr;

namespace  {

/// The size of the transient (i.e. temporary) surfaces in the cache in bytes.
/// These are all surfaces that are not loaded from disk.
const uint32_t TRANSIENT_TEXTURE_CACHE_SIZE = 160 << 20;   // shifting converts to MB

// Sets the icon for the application.
void set_icon(SDL_Window* sdl_window) {
#ifndef _WIN32
	const std::string icon_name = g_gr->image_catalog().filepath(ImageCatalog::Keys::kLogoWidelands128);
#else
	const std::string icon_name = g_gr->image_catalog().filepath(ImageCatalog::Keys::kLogoWidelands32);
#endif
	SDL_Surface* s = load_image_as_sdl_surface(icon_name, g_fs);
	SDL_SetWindowIcon(sdl_window, s);
	SDL_FreeSurface(s);
}

}  // namespace

/**
 * Initialize the SDL video mode.
*/
Graphic::Graphic(int window_mode_w, int window_mode_h, bool fullscreen)
   : m_window_mode_width(window_mode_w),
     m_window_mode_height(window_mode_h),
     m_update(true),
     texture_cache_(create_texture_cache(TRANSIENT_TEXTURE_CACHE_SIZE)),
     image_cache_(new ImageCache(texture_cache_.get())),
	  image_catalog_(new ImageCatalog()),
     animation_manager_(new AnimationManager())
{
	ImageTransformations::initialize();

	// Request an OpenGL 2 context with double buffering.
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	log("Graphics: Try to set Videomode %ux%u\n", m_window_mode_width, m_window_mode_height);
	m_sdl_window = SDL_CreateWindow("Widelands Window",
	                                SDL_WINDOWPOS_UNDEFINED,
	                                SDL_WINDOWPOS_UNDEFINED,
	                                m_window_mode_width,
	                                m_window_mode_height,
	                                SDL_WINDOW_OPENGL);
	resolution_changed();
	set_fullscreen(fullscreen);

	SDL_SetWindowTitle(m_sdl_window, ("Widelands " + build_id() + '(' + build_type() + ')').c_str());
	set_icon(m_sdl_window);

	m_glcontext = SDL_GL_CreateContext(m_sdl_window);
	SDL_GL_MakeCurrent(m_sdl_window, m_glcontext);

	// See graphic/gl/system_headers.h for an explanation of the
	// next line.
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		log("glewInit returns %i\nYour OpenGL installation must be __very__ broken. %s\n",
			 err, glewGetErrorString(err));
		throw wexception("glewInit returns %i: Broken OpenGL installation.", err);
	}

	log("Graphics: OpenGL: Version \"%s\"\n",
		 reinterpret_cast<const char*>(glGetString(GL_VERSION)));

	GLboolean glBool;
	glGetBooleanv(GL_DOUBLEBUFFER, &glBool);
	log("Graphics: OpenGL: Double buffering %s\n", (glBool == GL_TRUE) ? "enabled" : "disabled");

	GLint glInt;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glInt);
	log("Graphics: OpenGL: Max texture size: %u\n", glInt);

	SDL_GL_SetSwapInterval(1);

	glDrawBuffer(GL_BACK);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClear(GL_COLOR_BUFFER_BIT);

	SDL_GL_SwapWindow(m_sdl_window);

	/* Information about the video capabilities. */
	{
		SDL_DisplayMode disp_mode;
		SDL_GetWindowDisplayMode(m_sdl_window, &disp_mode);
		log("**** GRAPHICS REPORT ****\n"
		    " VIDEO DRIVER %s\n"
		    " pixel fmt %u\n"
		    " size %d %d\n"
		    "**** END GRAPHICS REPORT ****\n",
		    SDL_GetCurrentVideoDriver(),
		    disp_mode.format,
		    disp_mode.w,
		    disp_mode.h);
		assert(SDL_BYTESPERPIXEL(disp_mode.format) == 4);
	}
}

Graphic::~Graphic()
{
	m_maptextures.clear();
	texture_cache_->flush();
	// TODO(unknown): this should really not be needed, but currently is :(
	if (UI::g_fh)
		UI::g_fh->flush();

	if (m_sdl_window) {
		SDL_DestroyWindow(m_sdl_window);
		m_sdl_window = nullptr;
	}
	if (m_glcontext) {
		SDL_GL_DeleteContext(m_glcontext);
		m_glcontext = nullptr;
	}
}

/**
 * Return the screen x resolution
*/
int Graphic::get_xres()
{
	return screen_->width();
}

/**
 * Return the screen x resolution
*/
int Graphic::get_yres()
{
	return screen_->height();
}

void Graphic::change_resolution(int w, int h) {
	m_window_mode_width = w;
	m_window_mode_height = h;

	if (!fullscreen()) {
		SDL_SetWindowSize(m_sdl_window, w, h);
		resolution_changed();
	}
}

void Graphic::resolution_changed() {
	int new_w, new_h;
	SDL_GetWindowSize(m_sdl_window, &new_w, &new_h);

	screen_.reset(new Screen(new_w, new_h));
	m_rendertarget.reset(new RenderTarget(screen_.get()));

	Notifications::publish(GraphicResolutionChanged{new_w, new_h});

	update();
}

/**
 * Return a pointer to the RenderTarget representing the screen
*/
RenderTarget * Graphic::get_render_target()
{
	m_rendertarget->reset();
	return m_rendertarget.get();
}

bool Graphic::fullscreen()
{
	uint32_t flags = SDL_GetWindowFlags(m_sdl_window);
	return (flags & SDL_WINDOW_FULLSCREEN) || (flags & SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void Graphic::set_fullscreen(const bool value)
{
	if (value == fullscreen()) {
		return;
	}

	// Widelands is not resolution agnostic, so when we set fullscreen, we want
	// it at the full resolution of the desktop and we want to know about the
	// true resolution (SDL supports hiding the true resolution from the
	// application). Since SDL ignores requests to change the size of the window
	// whet fullscreen, we do it when in windowed mode.
	if (value) {
		SDL_DisplayMode display_mode;
		SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(m_sdl_window), &display_mode);
		SDL_SetWindowSize(m_sdl_window, display_mode.w, display_mode.h);

		SDL_SetWindowFullscreen(m_sdl_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	} else {
		SDL_SetWindowFullscreen(m_sdl_window, 0);

		// Next line does not work. See comment in refresh().
		SDL_SetWindowSize(m_sdl_window, m_window_mode_width, m_window_mode_height);
	}
	resolution_changed();
}


void Graphic::update() {
	m_update = true;
}

/**
 * Returns true if parts of the screen have been marked for refreshing.
*/
bool Graphic::need_update() const
{
	return  m_update;
}

/**
 * Bring the screen uptodate.
 *
 * \param force update whole screen
*/
void Graphic::refresh()
{
	// Setting the window size immediately after going out of fullscreen does
	// not work properly. We work around this issue by resizing the window in
	// refresh() when in window mode.
	if (!fullscreen()) {
		int true_width, true_height;
		SDL_GetWindowSize(m_sdl_window, &true_width, &true_height);
		if (true_width != m_window_mode_width || true_height != m_window_mode_height) {
			SDL_SetWindowSize(m_sdl_window, m_window_mode_width, m_window_mode_height);
		}
	}

	SDL_GL_SwapWindow(m_sdl_window);
	m_update = false;
}
const Image* Graphic::cataloged_image(ImageCatalog::Keys key) {
	return images().get(image_catalog_.get()->filepath(key));
}


/**
 * Saves a pixel region to a png. This can be a file or part of a stream.
 *
 * @param surf The Surface to save
 * @param sw a StreamWrite where the png is written to
 */
void Graphic::save_png(const Image* image, StreamWrite * sw) const {
	save_surface_to_png(image->texture(), sw, COLOR_TYPE::RGBA);
}

uint32_t Graphic::new_maptexture(const std::vector<std::string>& texture_files, const uint32_t frametime)
{
	m_maptextures.emplace_back(new TerrainTexture(texture_files, frametime));
	return m_maptextures.size(); // ID 1 is at m_maptextures[0]
}

/**
 * Advance frames for animated textures
*/
void Graphic::animate_maptextures(uint32_t time)
{
	for (uint32_t i = 0; i < m_maptextures.size(); ++i) {
		m_maptextures[i]->animate(time);
	}
}

/**
 * Save a screenshot to the given file.
*/
void Graphic::screenshot(const string& fname) const
{
	log("Save screenshot to %s\n", fname.c_str());
	StreamWrite * sw = g_fs->open_stream_write(fname);
	save_surface_to_png(screen_.get(), sw, COLOR_TYPE::RGB);
	delete sw;
}

/**
 * Retrieve the map texture with the given number
 * \return the actual texture data associated with the given ID.
 */
TerrainTexture * Graphic::get_maptexture_data(uint32_t id)
{
	--id; // ID 1 is at m_maptextures[0]

	assert(id < m_maptextures.size());
	return m_maptextures[id].get();
}
