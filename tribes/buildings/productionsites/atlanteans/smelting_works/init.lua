-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_smelting_works",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Smelting Works",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 1,
		granite = 4,
		planks = 1,
		spidercloth = 1
	},
	return_on_dismantle = {
		granite = 3
	},

   helptexts = {
		-- #TRANSLATORS: Lore helptext for a building
		lore = no_lore_text_yet(),
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = no_lore_author_text_yet(),
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Smelts iron ore into iron and gold ore into gold.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- #TRANSLATORS: Performance helptext for a building
		performance = no_performance_text_yet()
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname,  "idle_\\d+.png"),
			hotspot = { 57, 72 },
		},
		working = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"), -- TODO(GunChleoc): No animation yet.
			hotspot = { 57, 72 },
		}
	},

   aihints = {
		prohibited_till = 600
   },

	working_positions = {
		atlanteans_smelter = 1
	},

   inputs = {
		iron_ore = 8,
		gold_ore = 8,
		coal = 8
	},
   outputs = {
		"iron",
		"gold"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"call=smelt_iron",
				"call=smelt_gold",
				"call=smelt_iron",
				"return=skipped"
			}
		},
		smelt_iron = {
			-- TRANSLATORS: Completed/Skipped/Did not start smelting iron because ...
			descname = _"smelting iron",
			actions = {
				"return=skipped unless economy needs iron",
				"sleep=25000",
				"consume=iron_ore coal",
				"animate=working 35000",
				"produce=iron"
			}
		},
		smelt_gold = {
			-- TRANSLATORS: Completed/Skipped/Did not start smelting gold because ...
			descname = _"smelting gold",
			actions = {
				"return=skipped unless economy needs gold",
				"sleep=25000",
				"consume=gold_ore coal",
				"animate=working 35000",
				"produce=gold"
			}
		},
	},
}
