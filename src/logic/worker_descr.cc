/*
 * Copyright (C) 2002-2004, 2006-2010, 2012-2013 by the Widelands Development Team
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

#include "logic/worker_descr.h"

#include <memory>

#include "base/i18n.h"
#include "base/point.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "logic/carrier.h"
#include "logic/game_data_error.h"
#include "logic/nodecaps.h"
#include "logic/soldier.h"
#include "logic/tribes/tribe.h"
#include "logic/worker.h"
#include "logic/worker_program.h"
#include "sound/sound_handler.h"

namespace Widelands {

WorkerDescr::WorkerDescr(const std::string& init_descname,
								 const std::string& init_genericname,
								 MapObjectType init_type,
								 const LuaTable& table,
								 const EditorGameBase& egbase) :
	BobDescr(init_descname, init_type, MapObjectDescr::OwnerType::kTribe, table),
	ware_hotspot_      (Point(0, 15)),
	icon_fname_        (table.get_string("icon")),
	icon_              (nullptr),
	needed_experience_ (-1),
	becomes_           (INVALID_INDEX),
	generic_name_      (init_genericname),
	egbase_            (egbase)
{
	i18n::Textdomain td("tribes");
	std::unique_ptr<LuaTable> items_table;
	if (table.has_key("buildcost")) {
		items_table = table.get_table("buildcost");
		for (const std::string& key : items_table->keys<std::string>()) {
			int32_t value;
			try {
				if (buildcost_.count(key)) {
					throw GameDataError("a buildcost item of this ware type has already been defined: %s",
											  key.c_str());
				}

				if (egbase_.tribes().ware_index(key) == INVALID_INDEX &&
					 egbase_.tribes().worker_index(key) == INVALID_INDEX) {
					throw GameDataError
						("\"%s\" has not been defined as a ware/worker type (wrong "
						 "declaration order?)",
						 key.c_str());
				}
				value = items_table->get_int(key);
				uint8_t const count = value;
				if (count != value)
					throw GameDataError("count is out of range 1 .. 255");
				buildcost_.insert(std::pair<std::string, uint8_t>(key, count));
			} catch (const WException & e) {
				throw GameDataError
					("[buildcost] \"%s=%d\": %s",
					 key.c_str(), value, e.what());
			}
		}
	}

	helptext_ = _(table.get_string("helptext"));

	// Read the walking animations
	add_directional_animation(&walk_anims_, "walk");

	// Many workers don't carry wares, so they have no walkload animation.
	std::unique_ptr<LuaTable> anims(table.get_table("animations"));
	anims->do_not_warn_about_unaccessed_keys();
	if (anims->has_key("walkload")) {
		add_directional_animation(&walkload_anims_, "walkload");
	}

	// Read the becomes and experience
	if (table.has_key("becomes")) {
		becomes_ = egbase_.tribes().safe_worker_index(table.get_string("becomes"));
		needed_experience_ = table.get_int("experience");
	}

	// Read programs
	if (table.has_key("programs")) {
		items_table = table.get_table("programs");
		for (std::string program_name : items_table->keys<std::string>()) {
			std::transform
				(program_name.begin(), program_name.end(), program_name.begin(),
				 tolower);
			WorkerProgram * program = nullptr;

			try {
				if (programs_.count(program_name))
					throw wexception("this program has already been declared");

				WorkerProgram::Parser parser;

				parser.descr = this;
				parser.table = items_table.get();

				program = new WorkerProgram(program_name);
				program->parse(this, &parser, program_name.c_str(), egbase_.tribes());
				programs_[program_name.c_str()] = program;
			}

			catch (const std::exception & e) {
				delete program;
				throw wexception("program %s: %s", program_name.c_str(), e.what());
			}
		}
	}

	if (table.has_key("default_target_quantity")) {
		default_target_quantity_ = table.get_int("default_target_quantity");
	} else {
		default_target_quantity_ = std::numeric_limits<uint32_t>::max();
	}
	if (table.has_key("ware_hotspot")) {
		items_table = table.get_table("ware_hotspot");
		ware_hotspot_ = Point(items_table->get_int(1), items_table->get_int(2));
	}
}

WorkerDescr::WorkerDescr(const std::string& init_descname, const std::string& init_genericname,
								 const LuaTable& table, const EditorGameBase& egbase) :
	WorkerDescr(init_descname, init_genericname, MapObjectType::WORKER, table, egbase)
{}


WorkerDescr::~WorkerDescr()
{
	while (programs_.size()) {
		delete programs_.begin()->second;
		programs_.erase(programs_.begin());
	}
}


/**
 * Load graphics (other than animations).
 */
void WorkerDescr::load_graphics()
{
	icon_ = g_gr->images().get(icon_fname_);
}

const std::string& WorkerDescr::genericname() const {
	return generic_name_.empty() ? descname() : generic_name_;
}


/**
 * Get a program from the workers description.
 */
WorkerProgram const * WorkerDescr::get_program
	(const std::string & programname) const
{
	Programs::const_iterator it = programs_.find(programname);

	if (it == programs_.end())
		throw wexception
			("%s has no program '%s'", name().c_str(), programname.c_str());

	return it->second;
}

/**
 * Custom creation routing that accounts for the location.
 */
Worker & WorkerDescr::create
	(EditorGameBase &       egbase,
	 Player           &       owner,
	 PlayerImmovable  * const location,
	 Coords             const coords)
const
{
	Worker & worker = dynamic_cast<Worker&>(create_object());
	worker.set_owner(&owner);
	worker.set_location(location);
	worker.set_position(egbase, coords);
	worker.init(egbase);
	return worker;
}


uint32_t WorkerDescr::movecaps() const {return MOVECAPS_WALK;}


/**
 * Create a generic worker of this type.
 */
Bob & WorkerDescr::create_object() const
{
	return *new Worker(*this);
}


/**
* check if worker can be substitute for a requested worker type
 */
bool WorkerDescr::can_act_as(WareIndex const index) const {
	assert(egbase_.tribes().worker_exists(index));
	if (index == worker_index()) {
		return true;
	}

	// if requested worker type can be promoted, compare with that type
	const WorkerDescr& descr = *egbase_.tribes().get_worker_descr(index);
	WareIndex const becomes_index = descr.becomes();
	return becomes_index != INVALID_INDEX ? can_act_as(becomes_index) : false;
}

WareIndex WorkerDescr::worker_index() const {
	return egbase_.tribes().worker_index(name());
}

}
