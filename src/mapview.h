/*
 * Copyright (C) 2002 by Holger Rapp 
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

#ifndef __S__MAPVIEW_H
#define __S__MAPVIEW_H

#include "map.h"
#include "graphic.h"

/* class Map_View 
 *
 * this implements a view of a card. it's used
 * to render a valid card on the screen
 *
 * Depends: class Map
 * 			g_gr
 */
#include <iostream>
class Map_View {
		  Map_View(const Map_View&);
		  Map_View& operator=(const Map_View&);

		  public:
					 Map_View(Map*);
					 ~Map_View(void);

					 // Function to set the viewpoint
					 void set_viewpoint(uint,  uint); 
					
											 
					 void set_rel_viewpoint(int x, int y) { set_viewpoint(vpx+x,  vpy+y); }

					 void draw(void);

		  private:
					 struct __starts {
								float border1;
								float border2;
					 } g_starts[500], g_stops[500];

					 Map* map;
					 int vpx, vpy;
					 void draw_field(Field*);
					 void draw_polygon(Field*, Field*, Field*, Pic*);
					 void get_starts(const Field*, const Field*, const Field*, int, int);
					 void scanconv(const Field*, const Field*, __starts*, int ystart);
};


#endif /* __S__MAPVIEW_H */
