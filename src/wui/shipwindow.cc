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

#include "logic/map_objects/tribes/ship.h"

#include "base/macros.h"
#include "economy/portdock.h"
#include "economy/ware_instance.h"
#include "graphic/graphic.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/player.h"
#include "ui_basic/box.h"
#include "wui/actionconfirm.h"
#include "wui/game_debug_ui.h"
#include "wui/interactive_gamebase.h"
#include "wui/interactive_player.h"
#include "wui/itemwaresdisplay.h"

static const char pic_goto[] = "images/wui/ship/menu_ship_goto.png";
static const char pic_destination[] = "images/wui/ship/menu_ship_destination.png";
static const char pic_sink[]     = "images/wui/ship/menu_ship_sink.png";
static const char pic_debug[]     = "images/wui/fieldaction/menu_debug.png";
static const char pic_cancel_expedition[] = "images/wui/ship/menu_ship_cancel_expedition.png";
static const char pic_explore_cw[]  = "images/wui/ship/ship_explore_island_cw.png";
static const char pic_explore_ccw[] = "images/wui/ship/ship_explore_island_ccw.png";
static const char pic_scout_nw[] = "images/wui/ship/ship_scout_nw.png";
static const char pic_scout_ne[] = "images/wui/ship/ship_scout_ne.png";
static const char pic_scout_w[]  = "images/wui/ship/ship_scout_w.png";
static const char pic_scout_e[]  = "images/wui/ship/ship_scout_e.png";
static const char pic_scout_sw[] = "images/wui/ship/ship_scout_sw.png";
static const char pic_scout_se[] = "images/wui/ship/ship_scout_se.png";
static const char pic_construct_port[] = "images/wui/editor/fsel_editor_set_port_space.png";

namespace Widelands {

/**
 * Display information about a ship.
 */
struct ShipWindow : UI::Window {
	ShipWindow(InteractiveGameBase & igb, Ship & ship, const std::string & title);
	virtual ~ShipWindow();

	void think() override;

	UI::Button * make_button
		(UI::Panel * parent,
		 const std::string & name,
		 const std::string & title,
		 const std::string & picname,
		 boost::function<void()> callback);

	void act_goto();
	void act_destination();
	void act_sink();
	void act_debug();
	void act_cancel_expedition();
	void act_scout_towards(WalkingDir);
	void act_construct_port();
	void act_explore_island(IslandExploreDirection);

private:
	InteractiveGameBase & igbase_;
	Ship & ship_;

	UI::Button * btn_goto_;
	UI::Button * btn_destination_;
	UI::Button * btn_sink_;
	UI::Button * btn_debug_;
	UI::Button * btn_cancel_expedition_;
	UI::Button * btn_explore_island_cw_;
	UI::Button * btn_explore_island_ccw_;
	UI::Button * btn_scout_[LAST_DIRECTION]; // format: DIRECTION - 1, as 0 is normally the current location.
	UI::Button * btn_construct_port_;
	ItemWaresDisplay * display_;
};

ShipWindow::ShipWindow(InteractiveGameBase & igb, Ship & ship, const std::string & title) :
	Window(&igb, "shipwindow", 0, 0, 0, 0, title),
	igbase_(igb),
	ship_(ship)
{
	assert(!ship_.window_);
	assert(ship_.get_owner());
	ship_.window_ = this;

	UI::Box * vbox = new UI::Box(this, 0, 0, UI::Box::Vertical);

	display_ = new ItemWaresDisplay(vbox, *ship.get_owner());
	display_->set_capacity(ship.descr().get_capacity());
	vbox->add(display_, UI::Align::kHCenter, false);

	// Expedition buttons
	if (ship_.state_is_expedition()) {
		UI::Box * exp_top = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
		vbox->add(exp_top, UI::Align::kHCenter, false);
		UI::Box * exp_mid = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
		vbox->add(exp_mid, UI::Align::kHCenter, false);
		UI::Box * exp_bot = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
		vbox->add(exp_bot, UI::Align::kHCenter, false);

		btn_scout_[WALK_NW - 1] =
			make_button
				(exp_top, "scnw", _("Scout towards the north west"), pic_scout_nw,
				 boost::bind(&ShipWindow::act_scout_towards, this, WALK_NW));
		exp_top->add(btn_scout_[WALK_NW - 1], UI::Align::kLeft, false);

		btn_explore_island_cw_ =
			make_button
				(exp_top, "expcw", _("Explore the island’s coast clockwise"), pic_explore_cw,
				 boost::bind(&ShipWindow::act_explore_island, this, IslandExploreDirection::kClockwise));
		exp_top->add(btn_explore_island_cw_, UI::Align::kLeft, false);

		btn_scout_[WALK_NE - 1] =
			make_button
				(exp_top, "scne", _("Scout towards the north east"), pic_scout_ne,
				 boost::bind(&ShipWindow::act_scout_towards, this, WALK_NE));
		exp_top->add(btn_scout_[WALK_NE - 1], UI::Align::kLeft, false);

		btn_scout_[WALK_W - 1] =
			make_button
				(exp_mid, "scw", _("Scout towards the west"), pic_scout_w,
				 boost::bind(&ShipWindow::act_scout_towards, this, WALK_W));
		exp_mid->add(btn_scout_[WALK_W - 1], UI::Align::kLeft, false);

		btn_construct_port_ =
			make_button
				(exp_mid, "buildport", _("Construct a port at the current location"), pic_construct_port,
				 boost::bind(&ShipWindow::act_construct_port, this));
		exp_mid->add(btn_construct_port_, UI::Align::kLeft, false);

		btn_scout_[WALK_E - 1] =
			make_button
				(exp_mid, "sce", _("Scout towards the east"), pic_scout_e,
				 boost::bind(&ShipWindow::act_scout_towards, this, WALK_E));
		exp_mid->add(btn_scout_[WALK_E - 1], UI::Align::kLeft, false);

		btn_scout_[WALK_SW - 1] =
			make_button
				(exp_bot, "scsw", _("Scout towards the south west"), pic_scout_sw,
				 boost::bind(&ShipWindow::act_scout_towards, this, WALK_SW));
		exp_bot->add(btn_scout_[WALK_SW - 1], UI::Align::kLeft, false);

		btn_explore_island_ccw_ =
			make_button
				(exp_bot, "expccw", _("Explore the island’s coast counter clockwise"), pic_explore_ccw,
				 boost::bind(&ShipWindow::act_explore_island, this, IslandExploreDirection::kCounterClockwise));
		exp_bot->add(btn_explore_island_ccw_, UI::Align::kLeft, false);

		btn_scout_[WALK_SE - 1] =
			make_button
				(exp_bot, "scse", _("Scout towards the south east"), pic_scout_se,
				 boost::bind(&ShipWindow::act_scout_towards, this, WALK_SE));
		exp_bot->add(btn_scout_[WALK_SE - 1], UI::Align::kLeft, false);

	}

	// Bottom buttons
	UI::Box * buttons = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
	vbox->add(buttons, UI::Align::kLeft, false);

	btn_goto_ =
		make_button
			(buttons, "goto", _("Go to ship"), pic_goto,
			 boost::bind(&ShipWindow::act_goto, this));
	buttons->add(btn_goto_, UI::Align::kLeft, false);
	btn_destination_ =
		make_button
			(buttons, "destination", _("Go to destination"), pic_destination,
			 boost::bind(&ShipWindow::act_destination, this));
	btn_destination_->set_enabled(false);
	buttons->add(btn_destination_, UI::Align::kLeft, false);

	btn_sink_ =
		make_button
			(buttons, "sink", _("Sink the ship"), pic_sink, boost::bind(&ShipWindow::act_sink, this));
	buttons->add(btn_sink_, UI::Align::kLeft, false);

	if (ship_.state_is_expedition()) {
		btn_cancel_expedition_ =
			make_button
				(buttons, "cancel_expedition", _("Cancel the Expedition"), pic_cancel_expedition,
				boost::bind(&ShipWindow::act_cancel_expedition, this));
		buttons->add(btn_cancel_expedition_, UI::Align::kLeft, false);
	}

	if (igbase_.get_display_flag(InteractiveBase::dfDebug)) {
		btn_debug_ =
			make_button
				(buttons, "debug", _("Show Debug Window"), pic_debug,
				boost::bind(&ShipWindow::act_debug, this));
		btn_debug_->set_enabled(true);
		buttons->add
			(btn_debug_, UI::Align::kLeft, false);
	}
	set_center_panel(vbox);
	set_thinks(true);

	center_to_parent();
	move_out_of_the_way();
	set_fastclick_panel(btn_goto_);
}

ShipWindow::~ShipWindow()
{
	assert(ship_.window_ == this);
	ship_.window_ = nullptr;
}

void ShipWindow::think()
{
	UI::Window::think();
	InteractiveBase * ib = ship_.get_owner()->egbase().get_ibase();
	bool can_act = false;
	if (upcast(InteractiveGameBase, igb, ib))
		can_act = igb->can_act(ship_.get_owner()->player_number());

	btn_destination_->set_enabled(ship_.get_destination(igbase_.egbase()));
	btn_sink_->set_enabled(can_act);

	display_->clear();
	for (uint32_t idx = 0; idx < ship_.get_nritems(); ++idx) {
		Widelands::ShippingItem item = ship_.get_item(idx);
		Widelands::WareInstance * ware;
		Widelands::Worker * worker;
		item.get(igbase_.egbase(), &ware, &worker);

		if (ware) {
			display_->add(false, ware->descr_index());
		}
		if (worker) {
			display_->add(true, worker->descr().worker_index());
		}
	}

	// Expedition specific buttons
	Ship::ShipStates state = ship_.get_ship_state();
	if (ship_.state_is_expedition()) {
		/* The following rules apply:
		 * - The "construct port" button is only active, if the ship is waiting for commands and found a port
		 *   buildspace
		 * - The "scout towards a direction" buttons are only active, if the ship can move at least one field
		 *   in that direction without reaching the coast.
		 * - The "explore island's coast" buttons are only active, if a coast is in vision range (no matter if
		 *   in waiting or already expedition/scouting mode)
		 */
		btn_construct_port_->set_enabled(can_act && (state == Ship::ShipStates::kExpeditionPortspaceFound));
		bool coast_nearby = false;
		for (Direction dir = 1; dir <= LAST_DIRECTION; ++dir) {
			// NOTE buttons are saved in the format DIRECTION - 1
			btn_scout_[dir - 1]->set_enabled
				(can_act && ship_.exp_dir_swimable(dir) && (state != Ship::ShipStates::kExpeditionColonizing));
			coast_nearby |= !ship_.exp_dir_swimable(dir);
		}
		btn_explore_island_cw_ ->set_enabled(can_act && coast_nearby &&
														  (state != Ship::ShipStates::kExpeditionColonizing));
		btn_explore_island_ccw_->set_enabled(can_act && coast_nearby &&
														  (state != Ship::ShipStates::kExpeditionColonizing));
		btn_sink_              ->set_enabled(can_act &&
														  (state != Ship::ShipStates::kExpeditionColonizing));
		btn_cancel_expedition_ ->set_enabled(can_act &&
														  (state != Ship::ShipStates::kExpeditionColonizing));
	}
}

UI::Button * ShipWindow::make_button
	(UI::Panel * parent, const std::string & name, const std::string & title,
	 const std::string & picname, boost::function<void()> callback)
{
	UI::Button * btn =
		new UI::Button
			(parent, name, 0, 0, 34, 34,
			 g_gr->images().get("images/ui_basic/but4.png"),
			 g_gr->images().get(picname),
			 title);
	btn->sigclicked.connect(callback);
	return btn;
}

/// Move the main view towards the current ship location
void ShipWindow::act_goto()
{
	igbase_.move_view_to(ship_.get_position());
}

/// Move the main view towards the current destination of the ship
void ShipWindow::act_destination()
{
	if (PortDock * destination = ship_.get_destination(igbase_.egbase())) {
		igbase_.move_view_to(destination->get_warehouse()->get_position());
	}
}

/// Sink the ship if confirmed
void ShipWindow::act_sink()
{
	if (get_key_state(SDL_SCANCODE_LCTRL) || get_key_state(SDL_SCANCODE_RCTRL)) {
		igbase_.game().send_player_sink_ship(ship_);
	}
	else {
		show_ship_sink_confirm(dynamic_cast<InteractivePlayer&>(igbase_), ship_);
	}
}

/// Show debug info
void ShipWindow::act_debug()
{
	show_mapobject_debug(igbase_, ship_);
}

/// Cancel expedition if confirmed
void ShipWindow::act_cancel_expedition()
{
	if (get_key_state(SDL_SCANCODE_LCTRL) || get_key_state(SDL_SCANCODE_RCTRL)) {
		igbase_.game().send_player_cancel_expedition_ship(ship_);
	}
	else {
		show_ship_cancel_expedition_confirm
			(dynamic_cast<InteractivePlayer&>(igbase_), ship_);
	}
}

/// Sends a player command to the ship to scout towards a specific direction
void ShipWindow::act_scout_towards(WalkingDir direction) {
	// ignore request if the direction is not swimable at all
	if (!ship_.exp_dir_swimable(static_cast<Direction>(direction)))
		return;
	igbase_.game().send_player_ship_scouting_direction(ship_, direction);
}

/// Constructs a port at the port build space in vision range
void ShipWindow::act_construct_port() {
	if (ship_.exp_port_spaces().empty())
		return;
	igbase_.game().send_player_ship_construct_port(ship_, ship_.exp_port_spaces().front());
}

/// Explores the island cw or ccw
void ShipWindow::act_explore_island(IslandExploreDirection direction) {
	bool coast_nearby = false;
	bool moveable = false;
	for (Direction dir = 1; (dir <= LAST_DIRECTION) && (!coast_nearby || !moveable); ++dir) {
		if (!ship_.exp_dir_swimable(dir))
			coast_nearby = true;
		else
			moveable = true;
	}
	if (!coast_nearby || !moveable)
		return;
	igbase_.game().send_player_ship_explore_island(ship_, direction);
}


/**
 * Show the window for this ship as long as it is not sinking:
 * either bring it to the front, or create it.
 */
void Ship::show_window(InteractiveGameBase & igb, bool avoid_fastclick)
{
	// No window, if ship is sinking
	if (ship_state_ == ShipStates::kSinkRequest || ship_state_ == ShipStates::kSinkAnimation)
		return;

	if (window_) {
		if (window_->is_minimal())
			window_->restore();
		window_->move_to_top();
	} else {
		const std::string& title = get_shipname();
		new ShipWindow(igb, *this, title);
		if (!avoid_fastclick)
			window_->warp_mouse_to_fastclick_panel();
	}
}

/**
 * Close the window for this ship.
 */
void Ship::close_window()
{
	if (window_) {
		delete window_;
		window_ = nullptr;
	}
}

/**
 * refreshes the window of this ship - useful if some ui elements have to be removed or added
 */
void Ship::refresh_window(InteractiveGameBase & igb) {
	// Only do something if there is actually a window
	if (window_) {
		Point window_position = window_->get_pos();
		close_window();
		show_window(igb, true);
		// show window could theoretically fail if refresh_window was called at the very same moment
		// as the ship begins to sink
		if (window_)
			window_->set_pos(window_position);
	}
}

} // namespace Widelands
