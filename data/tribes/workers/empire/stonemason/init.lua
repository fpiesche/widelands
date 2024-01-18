push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "empire_stonemason",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Stonemason"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      pick = 1
   },

   programs = {
      cut_granite = {
         -- steps from building to rock: 2-9, min+max average 5.5
         -- min. worker time: 2 * 2 * 1.8 + 17.5 = 24.7 sec
         -- max. worker time: 2 * 9 * 1.8 + 17.5 = 49.9 sec
         -- avg. worker time: 2 * 5.5 * 1.8 + 17.5 = 37.3 sec
         "findobject=attrib:rocks radius:6",
         "walk=object",
         "playsound=sound/stonecutting/stonecutter priority:70% allow_multiple",
         "animate=hacking duration:17s500ms",
         "callobject=shrink",
         "createware=granite",
         "return"
      },
      cut_marble = {
         -- steps from building to rock: 2-9, min+max average 5.5
         -- min. worker time: 2 * 2 * 1.8 + 17.5 = 24.7 sec
         -- max. worker time: 2 * 9 * 1.8 + 17.5 = 49.9 sec
         -- avg. worker time: 2 * 5.5 * 1.8 + 17.5 = 37.3 sec
         "findobject=attrib:rocks radius:6",
         "walk=object",
         "playsound=sound/stonecutting/stonecutter priority:70% allow_multiple",
         "animate=hacking duration:17s500ms",
         "callobject=shrink",
         "createware=marble",
         "return"
      }
   },

   spritesheets = {
      idle = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 9, 23 }
      },
      hacking = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 8, 23 }
      },
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 9, 22 }
      },
      walkload = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 8, 25 }
      },
   }
}

pop_textdomain()
