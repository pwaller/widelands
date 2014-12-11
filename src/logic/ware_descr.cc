/*
 * Copyright (C) 2002-2004, 2006-2008, 2011-2012 by the Widelands Development Team
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

#include "logic/ware_descr.h"

#include "base/i18n.h"
#include "graphic/animation.h"
#include "graphic/graphic.h"
#include "logic/tribe.h"
#include "profile/profile.h"

namespace Widelands {

WareDescr::WareDescr
	(const TribeDescr & gtribe, char const * const _name,
	 char const * const _descname,
	 const std::string & directory, Profile & prof, Section & global_s)
	:
	MapObjectDescr(MapObjectType::WARE, _name, _descname),
	m_tribe         (gtribe),
	m_helptext      (global_s.get_string("help", "")),
	m_icon_fname    (directory + "/menu.png"),
	m_icon(g_gr->images().get("pics/but0.png"))
{
	m_default_target_quantity =
		global_s.get_positive("default_target_quantity", std::numeric_limits<uint32_t>::max());

	add_animation("idle", g_gr->animations().load(directory, prof.get_safe_section("idle")));

	m_preciousness =
		static_cast<uint8_t>(global_s.get_natural("preciousness", 0));
}

WareDescr::WareDescr(const LuaTable& table) :
	MapObjectDescr(MapObjectType::WARE, table.get_string("name"), table.get_string("descname")),
	m_helptext(table.get_string("help")),
	m_icon_fname(table.get_string("menu_picture")),
	m_icon(g_gr->images().get("pics/but0.png"))
{
// NOCOM(#sirver): do we want default arguments (i.e. preciousness).
// NOCOM(#sirver): should not know about its tribe anymore.
// NOCOM(#sirver): fill in the rest.
}
/**
 * Load all static graphics
 */
void WareDescr::load_graphics()
{
	m_icon = g_gr->images().get(m_icon_fname);
}

}
