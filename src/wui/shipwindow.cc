/*
 * Copyright (C) 2011, 2013 by the Widelands Development Team
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

#include "logic/ship.h"

#include "ui_basic/box.h"

#include "economy/portdock.h"
#include "economy/ware_instance.h"
#include "graphic/graphic.h"
#include "interactive_gamebase.h"
#include "itemwaresdisplay.h"
#include "logic/warehouse.h"
#include "logic/worker.h"

static const char pic_goto[] = "pics/menu_ship_goto.png";
static const char pic_destination[] = "pics/menu_ship_destination.png";
static const char pic_explore_cw[]  = "pics/ship_explore_island_cw.png";
static const char pic_explore_ccw[] = "pics/ship_explore_island_ccw.png";
static const char pic_scout_nw[] = "pics/ship_scout_nw.png";
static const char pic_scout_ne[] = "pics/ship_scout_ne.png";
static const char pic_scout_w[]  = "pics/ship_scout_w.png";
static const char pic_scout_e[]  = "pics/ship_scout_e.png";
static const char pic_scout_sw[] = "pics/ship_scout_sw.png";
static const char pic_scout_se[] = "pics/ship_scout_se.png";
static const char pic_construct_port[] = "pics/fsel_editor_set_port_space.png";

namespace Widelands {

/**
 * Display information about a ship.
 */
struct ShipWindow : UI::Window {
	ShipWindow(Interactive_GameBase & igb, Ship & ship);
	virtual ~ShipWindow();

	virtual void think();

	UI::Button * make_button
		(UI::Panel * parent,
		 const std::string & name,
		 const std::string & title,
		 const std::string & picname,
		 boost::function<void()> callback);

	void act_goto();
	void act_destination();
	void act_scout_towards(uint8_t);
	void act_construct_port();
	void act_explore_island(uint8_t);

private:
	Interactive_GameBase & m_igbase;
	Ship & m_ship;

	UI::Button * m_btn_goto;
	UI::Button * m_btn_destination;
	UI::Button * m_btn_explore_island_cw;
	UI::Button * m_btn_explore_island_ccw;
	UI::Button * m_btn_scout_nw;
	UI::Button * m_btn_scout_ne;
	UI::Button * m_btn_scout_w;
	UI::Button * m_btn_scout_e;
	UI::Button * m_btn_scout_sw;
	UI::Button * m_btn_scout_se;
	UI::Button * m_btn_construct_port;
	ItemWaresDisplay * m_display;

	enum {
		dir_w,
		dir_nw,
		dir_ne,
		dir_e,
		dir_se,
		dir_sw,
		exp_cw,
		exp_ccw
	};
};

ShipWindow::ShipWindow(Interactive_GameBase & igb, Ship & ship) :
	Window(&igb, "shipwindow", 0, 0, 0, 0, _("Ship")),
	m_igbase(igb),
	m_ship(ship)
{
	assert(!m_ship.m_window);
	assert(m_ship.get_owner());
	m_ship.m_window = this;

	UI::Box * vbox = new UI::Box(this, 0, 0, UI::Box::Vertical);

	m_display = new ItemWaresDisplay(vbox, *ship.get_owner());
	m_display->set_capacity(ship.get_capacity());
	vbox->add(m_display, UI::Box::AlignCenter, false);

	// Expedition buttons
	if (m_ship.get_ship_state() != Ship::TRANSPORT) {
		UI::Box * exp_top = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
		vbox->add(exp_top, UI::Box::AlignCenter, false);
		UI::Box * exp_mid = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
		vbox->add(exp_mid, UI::Box::AlignCenter, false);
		UI::Box * exp_bot = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
		vbox->add(exp_bot, UI::Box::AlignCenter, false);

		// TODO: Add a spacer between navigation block and bottom buttons?

		m_btn_scout_nw =
			make_button
				(exp_top, "scnw", _("Scout towards the north west"), pic_scout_nw,
				 boost::bind(&ShipWindow::act_scout_towards, this, dir_nw));
		exp_top->add(m_btn_scout_nw, 0, false);

		m_btn_explore_island_cw =
			make_button
				(exp_top, "expcw", _("Explore the island's coast clockwise"), pic_explore_cw,
				 boost::bind(&ShipWindow::act_explore_island, this, exp_cw));
		exp_top->add(m_btn_explore_island_cw, 0, false);

		m_btn_scout_ne =
			make_button
				(exp_top, "scne", _("Scout towards the north east"), pic_scout_ne,
				 boost::bind(&ShipWindow::act_scout_towards, this, dir_ne));
		exp_top->add(m_btn_scout_ne, 0, false);

		m_btn_scout_w =
			make_button
				(exp_mid, "scw", _("Scout towards the west"), pic_scout_w,
				 boost::bind(&ShipWindow::act_scout_towards, this, dir_w));
		exp_mid->add(m_btn_scout_w, 0, false);

		m_btn_construct_port =
			make_button
				(exp_mid, "buildport", _("Construct a port at the current location"), pic_construct_port,
				 boost::bind(&ShipWindow::act_construct_port, this));
		exp_mid->add(m_btn_construct_port, 0, false);

		m_btn_scout_e =
			make_button
				(exp_mid, "sce", _("Scout towards the east"), pic_scout_e,
				 boost::bind(&ShipWindow::act_scout_towards, this, dir_e));
		exp_mid->add(m_btn_scout_e, 0, false);

		m_btn_scout_sw =
			make_button
				(exp_bot, "scsw", _("Scout towards the south west"), pic_scout_sw,
				 boost::bind(&ShipWindow::act_scout_towards, this, dir_sw));
		exp_bot->add(m_btn_scout_sw, 0, false);

		m_btn_explore_island_ccw =
			make_button
				(exp_bot, "expccw", _("Explore the island's coast counter clockwise"), pic_explore_ccw,
				 boost::bind(&ShipWindow::act_explore_island, this, exp_ccw));
		exp_bot->add(m_btn_explore_island_ccw, 0, false);

		m_btn_scout_se =
			make_button
				(exp_bot, "scse", _("Scout towards the south east"), pic_scout_se,
				 boost::bind(&ShipWindow::act_scout_towards, this, dir_se));
		exp_bot->add(m_btn_scout_se, 0, false);

	}

	// Bottom buttons
	UI::Box * buttons = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
	vbox->add(buttons, UI::Box::AlignLeft, false);

	m_btn_goto =
		make_button
			(buttons, "goto", _("Go to ship"), pic_goto,
			 boost::bind(&ShipWindow::act_goto, this));
	buttons->add(m_btn_goto, 0, false);
	m_btn_destination =
		make_button
			(buttons, "destination", _("Go to destination"), pic_destination,
			 boost::bind(&ShipWindow::act_destination, this));
	m_btn_destination->set_enabled(false);
	buttons->add(m_btn_destination, 0, false);

	set_center_panel(vbox);
	set_think(true);

	center_to_parent();
	move_out_of_the_way();
	set_fastclick_panel(m_btn_goto);
}

ShipWindow::~ShipWindow()
{
	assert(m_ship.m_window == this);
	m_ship.m_window = 0;
}

void ShipWindow::think()
{
	UI::Window::think();

	m_btn_destination->set_enabled(m_ship.get_destination(m_igbase.egbase()));

	m_display->clear();
	for (uint32_t idx = 0; idx < m_ship.get_nritems(); ++idx) {
		Widelands::ShippingItem item = m_ship.get_item(idx);
		Widelands::WareInstance * ware;
		Widelands::Worker * worker;
		item.get(m_igbase.egbase(), ware, worker);

		if (ware) {
			m_display->add(false, ware->descr_index());
		}
		if (worker) {
			m_display->add(true, worker->descr().worker_index());
		}
	}
}

UI::Button * ShipWindow::make_button
	(UI::Panel * parent, const std::string & name, const std::string & title,
	 const std::string & picname, boost::function<void()> callback)
{
	UI::Button * btn =
		new UI::Button
			(parent, name, 0, 0, 34, 34,
			 g_gr->images().get("pics/but4.png"),
			 g_gr->images().get(picname),
			 title);
	btn->sigclicked.connect(callback);
	return btn;
}

/// Move the main view towards the current ship location
void ShipWindow::act_goto()
{
	m_igbase.move_view_to(m_ship.get_position());
}

/// Move the main view towards the current destination of the ship
void ShipWindow::act_destination()
{
	if (PortDock * destination = m_ship.get_destination(m_igbase.egbase())) {
		m_igbase.move_view_to(destination->get_warehouse()->get_position());
	}
}

/// Sends a player command to the ship to scout towards a specific direction
void ShipWindow::act_scout_towards(uint8_t direction) {
}

/// Constructs a port at the port build space in vision range
void ShipWindow::act_construct_port() {
}

/// Explores the island cw or ccw
void ShipWindow::act_explore_island(uint8_t direction) {
}


/**
 * Show the window for this ship: either bring it to the front,
 * or create it.
 */
void Ship::show_window(Interactive_GameBase & igb)
{
	if (m_window) {
		if (m_window->is_minimal())
			m_window->restore();
		m_window->move_to_top();
	} else {
		new ShipWindow(igb, *this);
		m_window->warp_mouse_to_fastclick_panel();
	}
}

/**
 * Close the window for this ship.
 */
void Ship::close_window()
{
	delete m_window;
}

} // namespace Widelands
