/*
 * Copyright (C) 2004, 2006-2013 by the Widelands Development Team
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

#ifndef WL_ECONOMY_WARE_INSTANCE_H
#define WL_ECONOMY_WARE_INSTANCE_H

#include "economy/transfer.h"
#include "logic/instances.h"
#include "logic/ware_descr.h"
#include "logic/widelands.h"
#include "map_io/tribes_legacy_lookup_table.h"

namespace Widelands {

class Building;
class Economy;
class EditorGameBase;
class Game;
struct IdleWareSupply;
class MapObject;
struct PlayerImmovable;
struct Transfer;

/**
 * WareInstance represents one item while it is being carried around.
 *
 * The WareInstance never draws itself; the carrying worker or the current flag
 * location are responsible for that.
 *
 * For robustness reasons, a WareInstance can only exist in a location that
 * assumes responsible for updating the instance's economy via \ref set_economy,
 * and that destroys the WareInstance when the location is destroyed.
 *
 * Currently, the location of a ware can be one of the following:
 * \li a \ref Flag
 * \li a \ref Worker that is currently carrying the ware
 * \li a \ref PortDock or \ref Ship where the ware is encapsulated in a \ref ShippingItem
 *     for seafaring
 */
class WareInstance : public MapObject {
	friend struct MapWaredataPacket;

	MO_DESCR(WareDescr)

public:
	WareInstance(WareIndex, const WareDescr* const);
	~WareInstance();

	MapObject* get_location(EditorGameBase& egbase) {
		return m_location.get(egbase);
	}
	Economy* get_economy() const {
		return m_economy;
	}
	WareIndex descr_index() const {
		return m_descr_index;
	}

	void init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;
	void act(Game&, uint32_t data) override;
	void update(Game&);

	void set_location(EditorGameBase&, MapObject* loc);
	void set_economy(Economy*);

	void enter_building(Game&, Building& building);

	bool is_moving() const;
	void cancel_moving();

	PlayerImmovable* get_next_move_step(Game&);

	void set_transfer(Game&, Transfer&);
	void cancel_transfer(Game&);
	Transfer* get_transfer() const {
		return m_transfer;
	}

	void log_general_info(const EditorGameBase& egbase) override;

private:
	ObjectPointer m_location;
	Economy* m_economy;
	WareIndex m_descr_index;

	IdleWareSupply* m_supply;
	Transfer* m_transfer;
	ObjectPointer m_transfer_nextstep;  ///< cached PlayerImmovable, can be 0

	// loading and saving stuff
protected:
	struct Loader : MapObject::Loader {
		Loader();

		void load(FileRead&);
		void load_pointers() override;
		void load_finish() override;

	private:
		uint32_t m_location;
		uint32_t m_transfer_nextstep;
		Transfer::ReadData m_transfer;
	};

public:
	bool has_new_save_support() override {
		return true;
	}

	void save(EditorGameBase&, MapObjectSaver&, FileWrite&) override;
	static MapObject::Loader* load(EditorGameBase&, MapObjectLoader&, FileRead&,
											 const TribesLegacyLookupTable& lookup_table);
};
}

#endif  // end of include guard: WL_ECONOMY_WARE_INSTANCE_H
