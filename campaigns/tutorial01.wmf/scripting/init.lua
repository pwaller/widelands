-- =======================================================================
--                       Barbarians Campaign Mission 1
-- =======================================================================

set_textdomain("scenario_tutorial.wmf")

-- ===============
-- Initialization
-- ===============
plr = wl.game.Player(1)
plr:allow_buildings("all")

-- A default headquarters
use("tribe_barbarians", "sc00_headquarters_medium")
init.func(plr) -- defined in sc00_headquarters_medium


use("aux", "coroutine")
use("aux", "ui")
use("aux", "table")

-- Constants
first_lumberjack_field = wl.map.Field(16,10)
first_quarry_field = wl.map.Field(8,12)
conquer_field = wl.map.Field(6,18)

-- Global variables
registered_player_immovables = {}
terminate_bad_boy_sentinel = false
illegal_immovable_found = function(i) return false end

use("map", "texts")


-- TODO: add objectives, there are none currently
-- TODO: add soldiers, training of soldiers and warfare

-- =================
-- Helper functions 
-- =================
function msg_box(i)
   wl.game.set_speed(1000)
   local user_input_state = wl.ui.get_user_input_allowed()

   if i.field then
      scroll_smoothly_to(i.field.trn.trn.trn.trn)

      i.field = nil -- Otherwise message box jumps back
   end

   if i.pos == "topleft" then
      i.posx = 0
      i.posy = 0
   elseif i.pos == "topright" then
      i.posx = 10000
      i.posy = 0
   end

   wl.ui.set_user_input_allowed(true)

   plr:message_box(i.title, i.body, i)

   wl.ui.set_user_input_allowed(user_input_state)

   sleep(130)
end

function send_message(i)
   plr:send_message(i.title, i.body, i)

   sleep(130)
end
   
function click_on_field(f, g_T)
   local user_input_state = wl.ui.get_user_input_allowed()
   wl.ui.set_user_input_allowed(false)

   mouse_smoothly_to(f, g_T)
   sleep(500)

   wl.ui.MapView():click(f)
   sleep(500)

   wl.ui.set_user_input_allowed(user_input_state)
end

function click_on_panel(panel, g_T)
   local user_input_state = wl.ui.get_user_input_allowed()
   wl.ui.set_user_input_allowed(false)

   sleep(500)
   if not panel.active then -- If this is a tab and already on, do nothing
      mouse_smoothly_to_panel(panel, g_T)
      sleep(500)
      if panel.press then panel:press() sleep(250) end
      if panel.click then panel:click() end
      sleep(500)
   end

   wl.ui.set_user_input_allowed(user_input_state)
end
   
-- Remove all stones in a given environment. This is done
-- in a loop for a nice optical effect
function remove_all_stones(fields, g_sleeptime)
   local sleeptime = g_sleeptime or 150 
   while #fields > 0 do
      local idx = math.random(#fields)
      local f = fields[idx]
      local remove_field = true

      if f.immovable then
         local n = f.immovable.name:match("stones(%d*)")
         if n then
            n = tonumber(n)
            f.immovable:remove()
            if n > 1 then 
               remove_field = false
               wl.map.create_immovable("stones" .. n-1, f)
               sleep(sleeptime)
            end
         end
      end

      if remove_field then
         table.remove(fields, idx)
      end
   end
end

-- ==============
-- Sentry Thread 
-- ==============
-- This thread makes sure that the player does not build stuff where he
-- is not supposed to. He gets a message box when he tries and what he build
-- gets immediately ripped. This thread can be disabled temporarily.
function _fmt(f) return ("%i_%i"):format(f.x, f.y) end
function register_immovable_as_allowed(i)
   for idx, f in ipairs(i.fields) do
      registered_player_immovables[_fmt(f)] = true
   end

    -- buildings and constructionsite have a flag
   if i.building_type or i.type == "constructionsite" then
      registered_player_immovables[_fmt(i.fields[1].brn)] = true
   end
end
register_immovable_as_allowed(plr.starting_field.immovable)

function bad_boy_sentry()
   while not terminate_bad_boy_sentinel do
      -- Check all fields.
      local sent_msg = false
      for idx,f in ipairs(plr.starting_field:region(8)) do
         if f.immovable and f.immovable.player == plr and
               not registered_player_immovables[_fmt(f)] then

            -- Give the callback a chance to veto the deletion. Maybe
            -- we expect the player to build something at the moment
            if not illegal_immovable_found(f.immovable) then
               print ("Killing", f.x .. '_' .. f.y)

               -- scould the player
               if not sent_msg then
                  msg_box(scould_player)
                  sent_msg = true
               end

               -- Remove the object again
               f.immovable:remove()
            end
         end
      end
      sleep(1000)
   end
end
   
-- Allows constructionsites for the given buildings, all others are invalid
-- as is any other immovable build by the player
function allow_constructionsite(i, buildings)
   if i.type == "constructionsite" then
      if not buildings then return i end
      for idx,n in ipairs(buildings) do
         if i.building == n then return i end
      end
      return false
   elseif i.type == "flag" then
      local tr = i.fields[1].tln.immovable
      if tr and tr.type == "constructionsite" then
         return allow_constructionsite(tr, buildings)
      end
   end

   return false
end

-- ================
-- Message threads 
-- ================
function starting_infos()
   sleep(100)

   msg_box(initial_message_01)
   sleep(500)
   msg_box(initial_message_02)

   -- Wait for buildhelp to come on
   while not wl.ui.MapView().buildhelp do
      sleep(200)
   end
   sleep(500)

   build_lumberjack()
end

function build_lumberjack()
   sleep(100)

   -- We take control, everything that we build is legal
   illegal_immovable_found = function(i) return true end

   msg_box(lumberjack_message_01)

   wl.ui.set_user_input_allowed(false)

   scroll_smoothly_to(first_lumberjack_field)
   mouse_smoothly_to(first_lumberjack_field)
   sleep(500)
   msg_box(lumberjack_message_02)
   sleep(500)

   click_on_field(first_lumberjack_field)
   click_on_panel(wl.ui.MapView().windows.field_action.tabs.small)
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.lumberjacks_hut)

   sleep(500)
   msg_box(lumberjack_message_03)
   sleep(500)

   click_on_field(plr.starting_field.brn)

   msg_box(lumberjack_message_04)
   
   register_immovable_as_allowed(first_lumberjack_field.immovable) -- hut + flag

   local f = wl.map.Field(14,11)
   register_immovable_as_allowed(f.immovable) -- road + everything on it

   illegal_immovable_found = function(i) return false end

   wl.ui.set_user_input_allowed(true)

   sleep(15000)
   
   msg_box(lumberjack_message_05)

   wl.ui.set_user_input_allowed(false)
   
   local f = wl.map.Field(14,11)
   scroll_smoothly_to(f)
   mouse_smoothly_to(f)
   
   wl.ui.set_user_input_allowed(true)
   
   -- Wait for flag
   while not (f.immovable and f.immovable.type == "flag") do sleep(300) end

   sleep(300)
   
   msg_box(lumberjack_message_06)
   
   while #plr:get_buildings("lumberjacks_hut") < 1 do sleep(300) end

   msg_box(lumberjack_message_07)

   learn_to_move()
end

function learn_to_move()
   -- Teaching the user how to scroll on the map
   msg_box(inform_about_stones)
   
   function _wait_for_move()
      local cx = wl.ui.MapView().viewpoint_x
      local cy = wl.ui.MapView().viewpoint_y
      while cx == wl.ui.MapView().viewpoint_x and
            cy == wl.ui.MapView().viewpoint_y do
         sleep(300)
      end
   end

   _wait_for_move()
   sleep(3000) -- Give the player a chance to try this some more

   msg_box(tell_about_right_drag_move)

   _wait_for_move()
   sleep(3000) -- Give the player a chance to try this some more
   
   msg_box(congratulate_and_on_to_quarry)

   build_a_quarry()
end
   
function build_a_quarry()
   sleep(200)

   -- Teaching how to build a quarry and the nits and knacks of road building.
   msg_box(order_quarry_recap_how_to_build)

   local cs = nil
   -- Wait for the constructionsite to come up.
   illegal_immovable_found = function(i)
      cs = allow_constructionsite(i, {"quarry"})
      return cs
   end

   -- Wait for the constructionsite to be placed
   while not cs do sleep(300) end
   register_immovable_as_allowed(cs)

   local function _rip_road()
      for idx,f in ipairs(cs.fields[1].brn:region(2)) do
         if f.immovable and f.immovable.type == "road" then 
            click_on_field(f)
            click_on_panel(wl.ui.MapView().windows.
               field_action.buttons.destroy_road, 300)
            sleep(200)
            return 
         end
      end
   end

   wl.ui.set_user_input_allowed(false)

   illegal_immovable_found = function() return true end

   msg_box(talk_about_roadbuilding_00)
   -- Showoff one-by-one roadbuilding
   click_on_field(wl.map.Field(9,12))
   click_on_field(wl.map.Field(10,12))
   click_on_field(wl.map.Field(11,12))
   click_on_field(wl.map.Field(12,12))
   click_on_field(wl.map.Field(12,11))
   
   sleep(3000)

   _rip_road()
   
   msg_box(talk_about_roadbuilding_01)
   -- Showoff direct roadbuilding
   click_on_field(cs.fields[1].brn)
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.build_road, 300)
   click_on_field(plr.starting_field.brn)
   
   sleep(3000)

   _rip_road()

   wl.ui.set_user_input_allowed(true)

   msg_box(talk_about_roadbuilding_02)
   
   -- Wait till a road is placed to the constructionsite
   local found_road = false

   illegal_immovable_found = function(i) 
      if i.type == "road" then
         local on_cs = i.start_flag == cs.fields[1].brn.immovable or
            i.end_flag == cs.fields[1].brn.immovable
         local on_hq = i.start_flag == plr.starting_field.brn.immovable or
            i.end_flag == plr.starting_field.brn.immovable

         if on_cs and on_hq then 
            register_immovable_as_allowed(i)
            found_road = true
            return true
         end
      end
      return false
   end

   while not found_road do sleep(300) end
   illegal_immovable_found = function() return false end

   -- Interludium: talk about census and statistics
   census_and_statistics(cs)

   while #plr:get_buildings("quarry") < 1 do sleep(1400) end

   messages()
end
   
function census_and_statistics(cs)
   sleep(25000)
   
   wl.ui.set_user_input_allowed(false)

   wl.ui.MapView().census = false
   wl.ui.MapView().statistics = false
  
   msg_box(census_and_statistics_00)
   -- Pick any empty field
   local function _pick_empty_field()
      local reg = cs.fields[1]:region(2)
      local f
      repeat
         f = reg[math.random(#reg)]
      until not f.immovable
      return f
   end

   click_on_field(_pick_empty_field())
   click_on_panel(wl.ui.MapView().windows.field_action.tabs.watch)
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.census)
   sleep(300)
   click_on_field(_pick_empty_field())
   click_on_panel(wl.ui.MapView().windows.field_action.tabs.watch)
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.statistics)

   msg_box(census_and_statistics_01)
   
   wl.ui.set_user_input_allowed(true)
end

function messages()
   -- Teach the player about receiving messages
   sleep(10)

   send_message(teaching_about_messages)

   while #plr.inbox > 0 do sleep(200) end
   sleep(500)

   msg_box(closing_msg_window_00)

   -- Wait for messages window to close
   while wl.ui.MapView().windows.messages do sleep(300) end
   
   msg_box(closing_msg_window_01)

   -- Remove all stones
   remove_all_stones(first_quarry_field:region(6))

   -- Wait for message to arrive
   while #plr.inbox < 1 do sleep(300) end

   sleep(800)
   msg_box(conclude_messages)

   sleep(3000)
   expansion()
end

function expansion() 
   -- Teach about expanding your territory.
   sleep(10)
   
   -- This is not really needed since the stones are already
   -- removed, but while debugging and we start with this function
   -- it is most useful to have the stones away already
   remove_all_stones(first_quarry_field:region(6), 20)

   msg_box(introduce_expansion)

   -- From now on, the player can build whatever he wants
   terminate_bad_boy_sentinel = true
   
   while #conquer_field.owners < 1 do sleep(100) end

   mining()
end

function mining()
   -- Teach about geologist and resources
   sleep(10)

   msg_box(mining_00)

   local function _find_good_flag_position()
      fields = conquer_field:region(8)
      while #fields > 0 do
         local idx = math.random(#fields)
         local f = fields[idx]
         
         if f.terr:match("berg%d+") and f.terd:match("berg%d+") then
            if pcall(function() plr:place_flag(f) end) then
               f.immovable:remove()
               return f
            end
         end

         table.remove(fields, idx)
      end
   end

   local function _find_nearby_flag()
      for i=2,8 do
         for idx, f in ipairs(conquer_field:region(i)) do
            if f.immovable and f.immovable.type == "flag" then
               return f
            end
         end
      end
   end

   scroll_smoothly_to(conquer_field)
   
   local dest = _find_good_flag_position()
   local start = _find_nearby_flag()

   -- Build a road, call a geologist
   click_on_field(start)
   click_on_panel(wl.ui.MapView().windows.field_action.tabs.roads)
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.build_road)
   click_on_field(dest)
   click_on_field(dest) -- second click
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.build_flag)
   click_on_field(dest)
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.geologist)

   sleep(6000)

   msg_box(mining_01)

   local function _wait_for_some_resi(wanted)
      while 1 do
         local cnt = 0
         for idx, f in ipairs(dest:region(6)) do 
            if f.immovable and f.immovable.name:sub(1,4) == "resi" then
               cnt = cnt + 1
               if cnt >= wanted then return end
            end
         end
         sleep(500)
      end
   end
   _wait_for_some_resi(8)

   scroll_smoothly_to(dest)

   msg_box(mining_02)

   conclusion()
end

function conclusion()
   -- Conclude the tutorial with final words and information
   -- on how to quit

   sleep(4000)
   msg_box(conclude_tutorial)

end

run(bad_boy_sentry)
run(starting_infos)

