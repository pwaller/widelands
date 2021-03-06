dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 7, 22 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {7, 20}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {8, 22}, 10)


tribes:new_carrier_type {
   msgctxt = "empire_worker",
   name = "empire_carrier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Carrier"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {}, -- This will give the worker the property "buildable"

   animations = animations,
}
