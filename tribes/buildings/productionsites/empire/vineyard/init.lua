-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_vineyard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Vineyard",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		planks = 2,
		granite = 1,
		marble = 2,
		marble_column = 2
	},
	return_on_dismantle = {
		planks = 1,
		marble = 2
	},

   helptexts = {
		-- #TRANSLATORS: Lore helptext for a building
		lore = no_lore_text_yet(),
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = no_lore_author_text_yet(),
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Plants grapevines and harvests grapes.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- #TRANSLATORS: Performance helptext for a building
		performance = no_performance_text_yet()
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 53, 53 },
		},
	},

   aihints = {
		space_consumer = true,
		forced_after = 120
   },

	working_positions = {
		empire_vinefarmer = 1
	},

   outputs = {
		"grape"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"call=plant_vine",
				"call=harvest_vine",
				"return=skipped"
			}
		},
		plant_vine = {
			-- TRANSLATORS: Completed/Skipped/Did not start planting grapevines because ...
			descname = _"planting grapevines",
			actions = {
				"sleep=20000",
				"worker=plantvine"
			}
		},
		harvest_vine = {
			-- TRANSLATORS: Completed/Skipped/Did not start harvesting grapevines because ...
			descname = _"harvesting grapes",
			actions = {
				"sleep=5000",
				"worker=harvestvine"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Out of Fields",
		message = _"The vine farmer working at this vineyard has no cleared soil to plant his grapevines.",
		productivity_threshold = 30
	},
}
