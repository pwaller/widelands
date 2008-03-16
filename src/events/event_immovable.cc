/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#include "event_immovable.h"

#include "game.h"
#include "profile.h"
#include "wexception.h"

#define EVENT_VERSION 1

namespace Widelands {

void Event_Immovable::Read(Section & s, Editor_Game_Base & egbase) {
	try {
		int32_t const packet_version = s.get_safe_int("version");
		if (packet_version == EVENT_VERSION) {
			Map const & map = egbase.map();
			m_location = s.get_safe_Coords  ("point",  map.extent());
			m_immovable_type =
				&s.get_safe_Immovable_Type("tribe", "name", egbase);
		} else
			throw wexception("unknown/unhandled version %i", packet_version);
	} catch (_wexception const & e) {
		throw wexception("(flag): %s", e.what());
	}
}

void Event_Immovable::Write(Section & s, Editor_Game_Base &) const {
	s.set_string        ("type",          "immovable");
	s.set_int           ("version",       EVENT_VERSION);
	s.set_Coords        ("point",         m_location);
	s.set_Immovable_Type("tribe", "name", *m_immovable_type);
}


Event::State Event_Immovable::run(Game * game) {
	Map const & map = game->map();
	if (BaseImmovable * const immovable = map[m_location].get_immovable())
		immovable->remove(game);
	m_immovable_type->create(*game, m_location);
	game->inform_players_about_immovable
		(Map::get_index(m_location, map.get_width()), m_immovable_type);
	return m_state = DONE;
}

};
