dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "mushroom_red_wasteland_sapling",
   descname = _ "Red Mushroom Tree (Sapling)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "seed" },
   programs = {
      program = {
         "animate=idle 42000",
         "remove=32",
         "grow=mushroom_red_wasteland_pole",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "sapling/", "idle_\\d+.png"),
         hotspot = { 5, 12 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "mushroom_red_wasteland_pole",
   descname = _ "Red Mushroom Tree (Pole)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 40000",
         "remove=25",
         "grow=mushroom_red_wasteland_mature",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "pole/", "idle_\\d+.png"),
         hotspot = { 12, 28 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "mushroom_red_wasteland_mature",
   descname = _ "Red Mushroom Tree (Mature)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 27000",
         "remove=10",
         "seed=mushroom_red_wasteland_sapling",
         "animate=idle 29000",
         "remove=10",
         "grow=mushroom_red_wasteland_old",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "mature/", "idle_\\d+.png"),
         hotspot = { 18, 48 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "mushroom_red_wasteland_old",
   descname = _ "Red Mushroom Tree (Old)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 800000",
         "transform=deadtree2 50",
         "seed=mushroom_red_wasteland_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "old/", "idle_\\d+.png"),
         hotspot = { 24, 60 },
         fps = 10,
      },
   },
}
