/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include "widelands.h"
#include "ui.h"
#include "game.h"
#include "map.h"
#include "IntPlayer.h"
#include "player.h"
#include "transport.h"
#include "fieldaction.h"
#include "watchwindow.h"


class FieldActionWindow : public Window {
public:
	FieldActionWindow(Interactive_Player *plr, UniqueWindow *registry);
	~FieldActionWindow();

	void init();
	void add_buttons_auto();
	void add_buttons_road(bool flag);
	
	// Action handlers
	void act_watch();
	void act_buildflag();
	void act_ripflag();
	void act_buildroad();
	void act_abort_buildroad();
	void act_removeroad();

private:
	static AutoPic pic_buildroad;
   static AutoPic pic_remroad;
   static AutoPic pic_buildflag;
   static AutoPic pic_ripflag;
   static AutoPic pic_watchfield;
   static AutoPic pic_abort;
   
   void add_button(AutoPic* pic, void (FieldActionWindow::*fn)());
	void okdialog();
	
	Interactive_Player	*m_player;
	Map						*m_map;
	UniqueWindow			*m_registry;

	int		m_nbuttons;
	
	FCoords	m_field;
};

AutoPic FieldActionWindow::pic_buildroad("menu_build_way.bmp", 0, 0, 255, 34, 34);
AutoPic FieldActionWindow::pic_remroad("menu_rem_way.bmp", 0, 0, 255, 34, 34);
AutoPic FieldActionWindow::pic_buildflag("menu_build_flag.bmp", 0, 0, 255, 34, 34);
AutoPic FieldActionWindow::pic_ripflag("menu_rip_flag.bmp", 0, 0, 255, 34, 34);
AutoPic FieldActionWindow::pic_watchfield("menu_watch_field.bmp", 0, 0, 255, 34, 34);
AutoPic FieldActionWindow::pic_abort("menu_abort.bmp", 0, 0, 255, 34, 34);

/*
===============
FieldActionWindow::FieldActionWindow

Initialize a field action window, creating the appropriate buttons.
===============
*/
FieldActionWindow::FieldActionWindow(Interactive_Player *plr, UniqueWindow *registry)
	: Window(plr, plr->get_w()/2, plr->get_h()/2, 68, 34, "Action")
{
	m_player = plr;
	m_map = m_player->get_game()->get_map();
	m_registry = registry;
	if (registry->window)
		delete registry->window;
	registry->window = this;
	
	Field *f = m_map->get_field(m_player->get_fieldsel());
	m_field = FCoords(m_player->get_fieldsel(), f);

	m_player->set_fieldsel_freeze(true);

	m_nbuttons = 0;
}

/*
===============
FieldActionWindow::~FieldActionWindow

Free allocated resources, remove from registry.
===============
*/
FieldActionWindow::~FieldActionWindow()
{
	m_player->set_fieldsel_freeze(false);
	m_registry->window = 0;
}


/*
===============
FieldActionWindow::init

Initialize after buttons have been registered.
This mainly deals with mouse placement
===============
*/
void FieldActionWindow::init()
{
	set_inner_size(m_nbuttons*34, 34);

	// Move the window away from the current mouse position, i.e.
	// where the field is, to allow better view
	int mousex = get_mouse_x();
	int mousey = get_mouse_y();
	
	if (mousex >= 0 && mousex < get_w() &&
	    mousey >= 0 && mousey < get_h()) {
		if (mousey < get_h()/2)
			set_pos(get_x(), get_y()+get_h());
		else
			set_pos(get_x(), get_y()-get_h());
	}
	
	// Now force the mouse onto the first button
	// TODO: adapt this automatically as more buttons are added
	set_mouse_pos(17, 17);
}


/*
===============
FieldActionWindow::add_buttons_auto

Add the buttons you normally get when clicking on a field.
===============
*/
void FieldActionWindow::add_buttons_auto()
{
	// Add actions
	if (m_field.field->get_owned_by() == m_player->get_player_number())
	{
		BaseImmovable *imm = m_map->get_immovable(m_field);
	
		if (imm && imm->get_type() == Map_Object::FLAG)
		{
			// Add flag actions
			Flag *flag = (Flag*)imm;

			add_button(&pic_buildroad, &FieldActionWindow::act_buildroad);

			Building *building = flag->get_building();

			if (!building || strcasecmp(building->get_name(), "headquarters"))
				add_button(&pic_ripflag, &FieldActionWindow::act_ripflag);
		}
		else
		{
			// Add build actions
			int buildcaps = m_player->get_player()->get_buildcaps(m_field);

			if (buildcaps & BUILDCAPS_FLAG)
				add_button(&pic_buildflag, &FieldActionWindow::act_buildflag);
			
			if (imm && imm->get_type() == Map_Object::ROAD)
				add_button(&pic_remroad, &FieldActionWindow::act_removeroad);
		}
	}
	
	// Common to all fields
	add_button(&pic_watchfield, &FieldActionWindow::act_watch);
}

/*
===============
FieldActionWindow::add_buttons_road

Buttons used during road building: Set flag here and Abort
===============
*/
void FieldActionWindow::add_buttons_road(bool flag)
{
	if (flag)
		add_button(&pic_buildflag, &FieldActionWindow::act_buildflag);
	
	add_button(&pic_abort, &FieldActionWindow::act_abort_buildroad);
}


/*
===============
FieldActionWindow::add_button
===============
*/
void FieldActionWindow::add_button(AutoPic *pic, void (FieldActionWindow::*fn)())
{
	Button *b = new Button(this, m_nbuttons*34, 0, 34, 34, 2);
	b->clicked.set(this, fn);
	b->set_pic(pic);
	
	m_nbuttons++;
}

/*
===============
FieldActionWindow::okdialog

Call this from the button handlers.
It resets the mouse to its original position and closes the window
===============
*/
void FieldActionWindow::okdialog()
{
	m_player->warp_mouse_to_field(m_field);
	die();
}

/*
===============
FieldActionWindow::act_watch()

Open a watch window for the given field and delete self.
===============
*/
void FieldActionWindow::act_watch()
{
	show_watch_window(m_player, m_field.x, m_field.y);
	okdialog();
}

/*
===============
FieldActionWindow::act_buildflag

Build a flag at this field
===============
*/
void FieldActionWindow::act_buildflag()
{
	Game *g = m_player->get_game();
	
	g->send_player_command(m_player->get_player_number(), CMD_BUILD_FLAG, m_field.x, m_field.y);
	
	if (m_player->is_building_road())
		m_player->finish_build_road();
	
	okdialog();
}

/*
===============
FieldActionWindow::act_ripflag

Remove the flag at this field
===============
*/
void FieldActionWindow::act_ripflag()
{
	Game *g = m_player->get_game();
	
	g->send_player_command(m_player->get_player_number(), CMD_RIP_FLAG, m_field.x, m_field.y);

	okdialog();
}

/*
===============
FieldActionWindow::act_buildroad

Start road building.
===============
*/
void FieldActionWindow::act_buildroad()
{
	m_player->start_build_road(m_field);
	okdialog();
}

/*
===============
FieldActionWindow::act_abort_buildroad

Abort building a road.
===============
*/
void FieldActionWindow::act_abort_buildroad()
{
	m_player->abort_build_road();
	okdialog();
}

/*
===============
FieldActionWindow::act_removeroad

Remove the road at the given field
===============
*/
void FieldActionWindow::act_removeroad()
{
	Game *g = m_player->get_game();
	BaseImmovable *imm = g->get_map()->get_immovable(m_field);
	
	if (imm && imm->get_type() == Map_Object::ROAD)
		g->send_player_command(m_player->get_player_number(), CMD_REMOVE_ROAD, imm->get_serial());
	
	okdialog();
}

/*
===============
show_field_action

Perform a field action (other than building options).
Bring up a field action window or continue road building.
===============
*/
void show_field_action(Interactive_Player *parent, UniqueWindow *registry)
{
	FieldActionWindow *faw;

	if (!parent->is_building_road()) {
		faw = new FieldActionWindow(parent, registry);
		faw->add_buttons_auto();
		faw->init();
		return;
	}

	// we're building a road right now	
	Map *map = parent->get_game()->get_map();
	Coords target = parent->get_fieldsel();
	
	// if user clicked on the same field again, build a flag	
	if (target == parent->get_build_road_end()) {
		faw = new FieldActionWindow(parent, registry);
		
		bool flag = false;
		if (target != parent->get_build_road_start() && 
		    parent->get_player()->get_buildcaps(target) & BUILDCAPS_FLAG)
			flag = true;
		faw->add_buttons_road(flag);
		faw->init();
		return;
	}

	// append or take away from the road
	if (!parent->append_build_road(target)) {
		faw = new FieldActionWindow(parent, registry);
		faw->add_buttons_road(false);
		faw->init();
		return;
	}
	
	// did he click on a flag or a road where a flag can be built?
	BaseImmovable *imm = map->get_immovable(target);
	
	if (imm) {
		switch(imm->get_type()) {
		case Map_Object::ROAD:
			if (!(parent->get_player()->get_buildcaps(target) & BUILDCAPS_FLAG))
				break;
			parent->get_game()->send_player_command(parent->get_player_number(), CMD_BUILD_FLAG, target.x, target.y);
			// fall through, there is a flag now
				
		case Map_Object::FLAG:
			parent->finish_build_road();
			break;
		}
	}
}
