/*
 * Copyright (C) 2002-2004 by The Widelands Development Team
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
#ifndef __S__WIDELANDS_BATTLE__H__
#define __S__WIDELANDS_BATTLE__H__
 
#include "immovable.h"
#include "soldier.h"

class Widelands_Map_Battle_Data_Packet;

class Battle : public BaseImmovable
{
   friend class Widelands_Map_Battle_Data_Packet;
   public:
         Battle();
         ~Battle();
      virtual int get_type () { return BATTLE; }
      virtual int get_size () { return SMALL; }
      virtual bool get_passable () { return false; }
      virtual void draw (Editor_Game_Base* eg, RenderTarget* rt, FCoords f, Point p) { }
      
      void init (Editor_Game_Base*);
      void init (Editor_Game_Base*, Soldier*, Soldier*);
      void soldiers (Soldier*, Soldier*);
      virtual void cleanup (Editor_Game_Base*);
      virtual void act (Game*, uint);
      
   private:
      Soldier* m_first;
      Soldier* m_second;
      int      m_last_try;
      int      m_next_assault;
};



#endif
