/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#include "widelands_map_heights_data_packet.h"
#include "filesystem.h"
#include "editor_game_base.h"
#include "map.h"
#include "world.h"
#include "widelands_map_data_packet_ids.h"
#include "error.h"

/*
 * Destructor
 */
Widelands_Map_Heights_Data_Packet::~Widelands_Map_Heights_Data_Packet(void) {
}

/*
 * Read Function
 */
void Widelands_Map_Heights_Data_Packet::Read(FileRead* fr, Editor_Game_Base* egbase) throw(wexception) {
   // Read all the heights 
   Map* map=egbase->get_map();
   
   for(ushort y=0; y<map->get_height(); y++) {
      for(ushort x=0; x<map->get_width(); x++) {
         uchar h=fr->Unsigned8();
//         log("[Map Loader] Setting height of field (%i,%i) to %i\n", x, y, h);
         map->get_field(Coords(x,y))->set_height(h);
      }
   }
}


/*
 * Write Function
 */
void Widelands_Map_Heights_Data_Packet::Write(FileWrite* fw, Editor_Game_Base* egbase) throw(wexception) {
   // first of all the magic bytes
   fw->Unsigned16(PACKET_HEIGHTS);

   // Now, all heights as unsigned chars in order
   Map* map=egbase->get_map();
   for(ushort y=0; y<map->get_height(); y++) {
      for(ushort x=0; x<map->get_width(); x++) {
         fw->Unsigned8(map->get_field(Coords(x,y))->get_height());
      }
   }
}
