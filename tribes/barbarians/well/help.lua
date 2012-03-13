use("aux", "formatting")
use("aux", "format_help")

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Well")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/well/well_i_00.png", p(
			_[[Text needed]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", _[[Source needed]])) ..
	--General Section
		rt(h2(_"General")) ..
		text_line(_"Purpose:", _"Draw Water out of the deep.","tribes/barbarians/water/menu.png") ..
		text_line(_"Working radius:", "1") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		text_line(_"Incoming:", "n/a") ..
		rt(h3(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/well/menu.png","tribes/barbarians/water/menu.png"},p(_"Water")) ..
		dependencies({"tribes/barbarians/water/menu.png","tribes/barbarians/bakery/menu.png"},p(_"Bakery")) ..
		dependencies({"tribes/barbarians/water/menu.png","tribes/barbarians/micro-brewery/menu.png"},p(_"Micro Brewery")) ..
		dependencies({"tribes/barbarians/water/menu.png","tribes/barbarians/brewery/menu.png"},p(_"Brewery")) ..
		dependencies({"tribes/barbarians/water/menu.png","tribes/barbarians/cattlefarm/menu.png"},p(_"Cattle Farm")) ..
		dependencies({"tribes/barbarians/water/menu.png","tribes/barbarians/lime_kiln/menu.png"},p(_"Lime Kiln")) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Small plot","pics/small.png") ..
		text_line(_"Upgraded from:", "n/a") ..
		rt(h3(_"Build Cost:")) ..
		image_line("tribes/barbarians/trunk/menu.png", 4, p("4 " .. _"Trunk")) ..
		rt(h3(_"Dismantle yields:")) ..
		image_line("tribes/barbarians/trunk/menu.png", 2, p("2 " .. _"Trunk")) ..
		text_line(_"Upgradeable to:","n/a") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/carrier/menu.png", 1, p(_"Carrier")) ..
		text_line(_"Worker uses:", "n/a") ..
		text_line(_"Experience levels:","n/a") ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"The Carrier needs %s to get one bucket full of water.":format("40s"))
}
