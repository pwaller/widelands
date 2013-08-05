/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#include "game_io/game_player_economies_data_packet.h"

#include "economy/economy_data_packet.h"
#include "economy/flag.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "upcast.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 3


void Game_Player_Economies_Data_Packet::Read
	(FileSystem & fs, Game & game, Map_Map_Object_Loader *)
{
	try {
		const Map   &       map        = game.map();
		Map_Index     const max_index  = map.max_index();
		Player_Number const nr_players = map.get_nrplayers();

		FileRead fr;
		fr.Open(fs, "binary/player_economies");
		uint16_t const packet_version = fr.Unsigned16();
		if (3 <= packet_version and packet_version <= CURRENT_PACKET_VERSION) {
			iterate_players_existing(p, nr_players, game, player)
				try {
					Player::Economies & economies = player->m_economies;
					uint16_t const nr_economies = economies.size();
					Player::Economies ecos(nr_economies);
					container_iterate(Player::Economies, ecos, i)
						if
							(upcast
							 	(Flag const,
							 	 flag,
							 	 map[fr.Map_Index32(max_index)].get_immovable()))
						{
							*i.current = flag->get_economy();
							EconomyDataPacket d(flag->get_economy());
							d.Read(fr);
						} else
							throw game_data_error
								(_("there is no flag at the specified location"));
				} catch (const _wexception & e) {
					throw game_data_error(_("player %u: %s"), p, e.what());
				}
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), packet_version);
	} catch (const _wexception & e) {
		throw game_data_error(_("economies: %s"), e.what());
	}
}

/*
 * Write Function
 */
void Game_Player_Economies_Data_Packet::Write
	(FileSystem & fs, Game & game, Map_Map_Object_Saver * const)
{
	FileWrite fw;

	fw.Unsigned16(CURRENT_PACKET_VERSION);

	const Map & map = game.map();
	const Field & field_0 = map[0];
	Player_Number const nr_players = map.get_nrplayers();
	iterate_players_existing_const(p, nr_players, game, player) {
		const Player::Economies & economies = player->m_economies;
		container_iterate_const(Player::Economies, economies, i) {
			// Walk the map so that we find a representant.
			for (Field const * field = &field_0;; ++field) {
				assert(field < &map[map.max_index()]); //  should never reach end
				if (upcast(Flag const, flag, field->get_immovable())) {
					if (flag->get_economy() == *i.current) {
						fw.Map_Index32(field - &field_0);

						EconomyDataPacket d(flag->get_economy());
						d.Write(fw);
						break;
					}
				}
			}
		}
	}

	fw.Write(fs, "binary/player_economies");
}

}
