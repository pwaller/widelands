/*
 * Copyright (C) 2002, 2006-2013 by the Widelands Development Team
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

#include "graphic/animation.h"

#include <cassert>
#include <cstdio>
#include <limits>
#include <memory>

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "graphic/diranimations.h"
#include "graphic/graphic.h"
#include "graphic/image.h"
#include "graphic/image_cache.h"
#include "graphic/surface.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/bob.h"
#include "logic/instances.h"
#include "profile/profile.h"
#include "scripting/lua_table.h"
#include "sound/sound_handler.h"


using namespace std;


namespace  {

/// A class that makes iteration over filename_?.png templates easy.
class NumberGlob {
public:
	NumberGlob(const std::string& pictmp);

	/// If there is a next filename, puts it in 's' and returns true.
	bool next(std::string* s);

private:
	std::string templ_;
	std::string fmtstr_;
	std::string replstr_;
	uint32_t cur_;
	uint32_t max_;

	DISALLOW_COPY_AND_ASSIGN(NumberGlob);
};

/**
 * Implemantation for NumberGlob.
 */
NumberGlob::NumberGlob(const string& pictmp) : templ_(pictmp), cur_(0) {
	int nchars = count(pictmp.begin(), pictmp.end(), '?');
	fmtstr_ = "%0" + boost::lexical_cast<string>(nchars) + "i";

	max_ = 1;
	for (int i = 0; i < nchars; ++i) {
		max_ *= 10;
		replstr_ += "?";
	}
	max_ -= 1;
}

bool NumberGlob::next(string* s) {
	if (cur_ > max_)
		return false;

	if (max_) {
		*s = boost::replace_last_copy(templ_, replstr_, (boost::format(fmtstr_) % cur_).str());
	} else {
		*s = templ_;
	}
	++cur_;
	return true;
}


// Parses an array { 12, 23 } into a point.
void get_point(const LuaTable& table, Point* p) {
	std::vector<int> pts = table.array_entries<int>();
	if (pts.size() != 2) {
		throw wexception("Expected 2 entries, but got %" PRIuS ".", pts.size());
	}
	p->x = pts[0];
	p->y = pts[1];
}

/**
 * Implements the Animation interface for an animation that is unpacked on disk, that
 * is every frame and every pc color frame is an singular file on disk.
 */
class NonPackedAnimation : public Animation {
public:
	virtual ~NonPackedAnimation() {}
	NonPackedAnimation(const string& directory, Section & section);
	NonPackedAnimation(const LuaTable& table);

	// Implements Animation.
	uint16_t width() const override;
	uint16_t height() const override;
	uint16_t nr_frames() const override;
	uint32_t frametime() const override;
	const Point& hotspot() const override;
	const Image& representative_image_from_disk() const override;
	const std::string& representative_image_from_disk_filename() const override;
	virtual void blit(uint32_t time, const Point&, const Rect& srcrc, const RGBColor* clr, Surface*)
	   const override;
	void trigger_soundfx(uint32_t framenumber, uint32_t stereo_position) const override;


private:
	// Loads the graphics if they are not yet loaded.
	void ensure_graphics_are_loaded() const;

	// Load the needed graphics from disk.
	void load_graphics();

	uint32_t frametime_;
	Point hotspot_;
	bool hasplrclrs_;
	std::vector<std::string> image_files_;
	std::vector<std::string> pc_mask_image_files_;

	vector<const Image*> frames_;
	vector<const Image*> pcmasks_;

	// name of sound effect that will be played at frame 0.
	// TODO(sirver): this should be done using playFX in a program instead of
	// binding it to the animation.
	string sound_effect_;
};

NonPackedAnimation::NonPackedAnimation(const string& directory, Section& section)
		: frametime_(FRAME_LENGTH),
		  hasplrclrs_(false) {
	// If this animation has a sound effect associated, try to load it now.
	const std::string sfx = section.get_string("sfx", "");
	if (!sfx.empty()) {
			sound_effect_ = string(directory) + "/" + sfx;
			g_sound_handler.load_fx_if_needed(directory, sfx, sound_effect_);
	}

	int32_t const fps = section.get_int("fps");
	if (fps < 0)
		throw wexception("fps is %i, must be non-negative", fps);
	if (fps > 0)
		frametime_ = 1000 / fps;

	hotspot_ = section.get_point("hotspot");

	//  In the filename template, the last sequence of '?' characters (if any)
	//  is replaced with a number, for example the template "idle_??" is
	//  replaced with "idle_00". Then the code looks if there is a PNG with that
	//  name, increments the number and continues . on until it can not find any
	//  file. Then it is assumed that there are no more frames in the animation.
	string picnametempl;
	if (char const * const pics = section.get_string("pics")) {
		picnametempl = directory + pics;
	} else {
		picnametempl = directory + section.get_name();
	}
	// Strip the .png extension if it has one.
	boost::replace_all(picnametempl, ".png", "");

	NumberGlob glob(picnametempl);
	string filename_wo_ext;
	while (glob.next(&filename_wo_ext)) {
		const string filename = filename_wo_ext + ".png";
		if (!g_fs->file_exists(filename))
			break;
		image_files_.push_back(filename);

		const string pc_filename = filename_wo_ext + "_pc.png";
		if (g_fs->file_exists(pc_filename)) {
			hasplrclrs_ = true;
			pc_mask_image_files_.push_back(pc_filename);
		}
	}
}

NonPackedAnimation::NonPackedAnimation(const LuaTable& table)
		: frametime_(FRAME_LENGTH),
		  hasplrclrs_(false) {
	// TODO(sirver): the LuaTable constructor has no support for player_colors right now.
	get_point(*table.get_table("hotspot"), &hotspot_);

	if (table.has_key("sound_effect")) {
		std::unique_ptr<LuaTable> sound_effects = table.get_table("sound_effect");

		const std::string name = sound_effects->get_string("name");
		const std::string directory = sound_effects->get_string("directory");
		sound_effect_ = directory + "/" + name;
		g_sound_handler.load_fx_if_needed(directory, name, sound_effect_);
	}

	image_files_ = table.get_table("pictures")->array_entries<std::string>();
	if (image_files_.empty()) {
		throw wexception("Animation without pictures.");
	} else if (image_files_.size() == 1) {
		if (table.has_key("fps")) {
			throw wexception("Animation with one picture must not have 'fps'.");
		}
	} else {
		frametime_ = 1000 / get_positive_int(table, "fps");
	}
}

void NonPackedAnimation::ensure_graphics_are_loaded() const {
	if (frames_.empty()) {
		const_cast<NonPackedAnimation*>(this)->load_graphics();
	}
}

void NonPackedAnimation::load_graphics() {
	if (image_files_.empty())
		throw wexception("animation without pictures.");

	if (pc_mask_image_files_.size() && pc_mask_image_files_.size() != image_files_.size())
		throw wexception
			("animation has %" PRIuS " frames but playercolor mask has %" PRIuS " frames",
			 image_files_.size(), pc_mask_image_files_.size());

	for (const std::string& filename : image_files_) {
		const Image* image = g_gr->images().get(filename);
		if (frames_.size() &&
		    (frames_[0]->width() != image->width() || frames_[0]->height() != image->height())) {
			throw wexception("wrong size: (%u, %u), should be (%u, %u) like the first frame",
			                 image->width(),
			                 image->height(),
			                 frames_[0]->width(),
			                 frames_[0]->height());
		}
		frames_.push_back(image);
	}

	for (const std::string& filename : pc_mask_image_files_) {
		// TODO(unknown): Do not load playercolor mask as opengl texture or use it as
		//     opengl texture.
		const Image* pc_image = g_gr->images().get(filename);
		if (frames_[0]->width() != pc_image->width() || frames_[0]->height() != pc_image->height()) {
			// TODO(unknown): see bug #1324642
			throw wexception("playercolor mask has wrong size: (%u, %u), should "
			                 "be (%u, %u) like the animation frame",
			                 pc_image->width(),
			                 pc_image->height(),
			                 frames_[0]->width(),
			                 frames_[0]->height());
		}
		pcmasks_.push_back(pc_image);
	}
}

uint16_t NonPackedAnimation::width() const {
	ensure_graphics_are_loaded();
	return frames_[0]->width();
}

uint16_t NonPackedAnimation::height() const {
	ensure_graphics_are_loaded();
	return frames_[0]->height();
}

uint16_t NonPackedAnimation::nr_frames() const {
	ensure_graphics_are_loaded();
	return frames_.size();
}

uint32_t NonPackedAnimation::frametime() const {
	return frametime_;
}

const Point& NonPackedAnimation::hotspot() const {
	return hotspot_;
}

const Image& NonPackedAnimation::representative_image_from_disk() const {
	ensure_graphics_are_loaded();
	return *frames_[0];
}

const std::string& NonPackedAnimation::representative_image_from_disk_filename() const {
	return image_files_[0];
}

void NonPackedAnimation::trigger_soundfx(uint32_t time, uint32_t stereo_position) const {
	if (sound_effect_.empty()) {
		return;
	}
	const uint32_t framenumber = time / frametime_ % nr_frames();
	if (framenumber == 0) {
		g_sound_handler.play_fx(sound_effect_, stereo_position, 1);
	}
}

void NonPackedAnimation::blit
	(uint32_t time, const Point& dst, const Rect& srcrc, const RGBColor* clr, Surface* target) const
{
	assert(target);

	const int idx = time / frametime_ % nr_frames();
	assert(idx < nr_frames());

	if (!hasplrclrs_ || clr == nullptr) {
		target->blit(
		   Rect(dst.x, dst.y, srcrc.w, srcrc.h), *frames_.at(idx), srcrc, 1., BlendMode::UseAlpha);
	} else {
		target->blit_blended(
		   Rect(dst.x, dst.y, srcrc.w, srcrc.h), *frames_.at(idx), *pcmasks_.at(idx), srcrc, *clr);
	}
}

}  // namespace


/*
==============================================================================

DirAnimations IMPLEMENTAION

==============================================================================
*/

DirAnimations::DirAnimations
	(uint32_t dir1,
	 uint32_t dir2,
	 uint32_t dir3,
	 uint32_t dir4,
	 uint32_t dir5,
	 uint32_t dir6)
{
	m_animations[0] = dir1;
	m_animations[1] = dir2;
	m_animations[2] = dir3;
	m_animations[3] = dir4;
	m_animations[4] = dir5;
	m_animations[5] = dir6;
}


/**
 * Load direction animations of the given name.
 *
 * If a section of the given name exists, it is expected to contain a 'dirpics'
 * key and assorted information of the old direction animation format.
 *
 * Otherwise, sections with the names 'name_??', with ?? replaced
 * by nw, ne, e, se, sw, and w are expected to exist and describe
 * the corresponding animations.
 *
 * @param optional No error if animations do not exist
 */
void DirAnimations::parse
	(Widelands::MapObjectDescr & b,
	 const string & directory,
	 Profile & prof,
	 const string & name,
	 bool optional,
	 const string & default_dirpics)
{
	if (Section * section = prof.get_section(name)) {
		// NOTE: deprecate this format eventually
		char dirpictempl[256];
		char * repl;

		snprintf
			(dirpictempl, sizeof(dirpictempl), "%s",
			 section->get_string("dirpics", default_dirpics.c_str()));
		repl = strstr(dirpictempl, "!!");
		if (!repl)
			throw wexception
				("DirAnimations dirpics name templates %s does not contain !!",
				 dirpictempl);
		strncpy(repl, "%s", 2);

		for (int32_t dir = 0; dir < 6; ++dir) {
			static char const * const dirstrings[6] =
				{"ne", "e", "se", "sw", "w", "nw"};

			// Fake the section name here, so that the animation loading code is
			// using the correct glob pattern to load the images from.
			char pictempl[256];
			snprintf(pictempl, sizeof(pictempl), dirpictempl, dirstrings[dir]);
			section->set_name(pictempl);
			m_animations[dir] = g_gr->animations().load(directory, *section);

			char animname[256];
			snprintf(animname, sizeof(animname), "%s_%s", name.c_str(), dirstrings[dir]);
			b.add_animation(animname, m_animations[dir]);
		}
	} else {
		for (int32_t dir = 0; dir < 6; ++dir) {
			static char const * const dirstrings[6] =
				{"ne", "e", "se", "sw", "w", "nw"};

			char animname[256];
			snprintf(animname, sizeof(animname), "%s_%s", name.c_str(), dirstrings[dir]);
			Section * dirsection = prof.get_section(animname);
			if (dirsection) {
				m_animations[dir] = g_gr->animations().load(directory, *dirsection);
				b.add_animation(animname, m_animations[dir]);
			} else {
				if (!optional)
					throw wexception("DirAnimations: did not find section %s", animname);
			}
		}
	}
}


/*
==============================================================================

AnimationManager IMPLEMENTATION

==============================================================================
*/

uint32_t AnimationManager::load(const string& directory, Section & s) {
	m_animations.push_back(new NonPackedAnimation(directory, s));
	return m_animations.size();
}

uint32_t AnimationManager::load(const LuaTable& table) {
	m_animations.push_back(new NonPackedAnimation(table));
	return m_animations.size();
}

const Animation& AnimationManager::get_animation(uint32_t id) const
{
	if (!id || id > m_animations.size())
		throw wexception("Requested unknown animation with id: %i", id);

	return *m_animations[id - 1];
}
