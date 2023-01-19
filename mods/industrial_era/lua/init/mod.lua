-- Industrial_Era_mod - Base game files for Symphony of Empires
-- Copyright (C) 2021, Symphony of Empires contributors
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License along
-- with this program; if not, write to the Free Software Foundation, Inc.,
-- 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
--
-- ----------------------------------------------------------------------------
--	Name:
--  	mod.lua
--
-- 	Abstract:
--      Does important stuff
-- ----------------------------------------------------------------------------

require('entities/events/capitalism')
require('entities/events/catholic_relief_act')
require('entities/events/intro')
require('entities/events/generic')
require('entities/events/jutland_sinks')
require('entities/events/panama_canal')
require('entities/events/kalmar_union')
require('entities/events/minas_gerais_crash')
require('entities/events/test_event')
require('entities/events/special')
require('entities/events/taza_concession')
require('entities/events/unify_germany')
require('entities/events/westernization')
require('entities/events/yongle_encyclopedia')

-- Puppets
Nation:get("france"):make_puppet(Nation:get("piedmont_sardinia"))

Nation:get("ottoman"):make_puppet(Nation:get("tunis"))
Nation:get("ottoman"):make_puppet(Nation:get("algiers"))
Nation:get("ottoman"):make_puppet(Nation:get("egypt"))
Nation:get("ottoman"):make_puppet(Nation:get("tripolitania"))
Nation:get("ottoman"):make_puppet(Nation:get("wallachia"))
Nation:get("ottoman"):make_puppet(Nation:get("moldavia"))

Nation:get("netherlands"):make_puppet(Nation:get("luxembourg"))

Nation:get("sweden"):make_puppet(Nation:get("norway"))

Nation:get("united_kingdom"):make_puppet(Nation:get("hannover"))
Nation:get("united_kingdom"):make_puppet(Nation:get("ruperts_land"))
Nation:get("united_kingdom"):make_puppet(Nation:get("lower_canada"))
Nation:get("united_kingdom"):make_puppet(Nation:get("upper_canada"))
Nation:get("united_kingdom"):make_puppet(Nation:get("prince_edward_island"))
Nation:get("united_kingdom"):make_puppet(Nation:get("nova_scotia"))
Nation:get("united_kingdom"):make_puppet(Nation:get("new_brunswick"))
Nation:get("united_kingdom"):make_puppet(Nation:get("east_india_company"))
Nation:get("united_kingdom"):make_puppet(Nation:get("mysore"))
Nation:get("united_kingdom"):make_puppet(Nation:get("haydarabad"))

Nation:get("russia"):make_puppet(Nation:get("finland"))
Nation:get("russia"):make_puppet(Nation:get("congress_poland"))
Nation:get("russia"):make_puppet(Nation:get("mingrelia"))

-- Zollverein not until 1830's
-- Nation:get("prussia"):make_customs_union(Nation:get("salzburg"))
-- Nation:get("prussia"):make_customs_union(Nation:get("thuringia"))
-- Nation:get("prussia"):make_customs_union(Nation:get("lubeck"))
-- Nation:get("prussia"):make_customs_union(Nation:get("waldeck"))
-- Nation:get("prussia"):make_customs_union(Nation:get("hesse_kassel"))
-- Nation:get("prussia"):make_customs_union(Nation:get("reuss_gera"))
-- Nation:get("prussia"):make_customs_union(Nation:get("saxe_weimar"))
-- Nation:get("prussia"):make_customs_union(Nation:get("saxe_gotha"))
-- Nation:get("prussia"):make_customs_union(Nation:get("saxe_coburg"))
-- Nation:get("prussia"):make_customs_union(Nation:get("saxe_hildburghausen"))
-- Nation:get("prussia"):make_customs_union(Nation:get("saxe_meiningen"))
-- Nation:get("prussia"):make_customs_union(Nation:get("lippe"))
-- Nation:get("prussia"):make_customs_union(Nation:get("lorraine"))
-- Nation:get("prussia"):make_customs_union(Nation:get("brandenburg"))
-- Nation:get("prussia"):make_customs_union(Nation:get("frankfurt"))
-- Nation:get("prussia"):make_customs_union(Nation:get("munich"))
-- Nation:get("prussia"):make_customs_union(Nation:get("hamburg"))
-- Nation:get("prussia"):make_customs_union(Nation:get("nassau"))
-- Nation:get("prussia"):make_customs_union(Nation:get("baden"))
-- Nation:get("prussia"):make_customs_union(Nation:get("brunswick"))
-- Nation:get("prussia"):make_customs_union(Nation:get("anhalt"))
-- Nation:get("prussia"):make_customs_union(Nation:get("oldenburg"))
-- Nation:get("prussia"):make_customs_union(Nation:get("silesia"))
-- Nation:get("prussia"):make_customs_union(Nation:get("danzig"))
-- Nation:get("prussia"):make_customs_union(Nation:get("wolfenbuttel"))
-- Nation:get("prussia"):make_customs_union(Nation:get("bremen"))
-- Nation:get("prussia"):make_customs_union(Nation:get("schaumburg_lippe"))
-- Nation:get("prussia"):make_customs_union(Nation:get("ritzebuttel"))
-- Nation:get("prussia"):make_customs_union(Nation:get("liechtenstein"))
-- Nation:get("prussia"):make_customs_union(Nation:get("mecklenburg"))
-- Nation:get("prussia"):make_customs_union(Nation:get("grand_duchy_of_hesse_and_by_rhine"))
-- Nation:get("prussia"):make_customs_union(Nation:get("saxony"))
-- Nation:get("prussia"):make_customs_union(Nation:get("bavaria"))
-- Nation:get("prussia"):make_customs_union(Nation:get("wurttemberg"))
-- Nation:get("prussia"):make_customs_union(Nation:get("parma"))
-- Nation:get("prussia"):make_customs_union(Nation:get("modena_and_reggio"))
-- Nation:get("prussia"):make_customs_union(Nation:get("tuscany"))

Nation:get("denmark"):make_puppet(Nation:get("holstein"))

-- Ongoing wars
Nation:get("russia"):declare_no_cb(Nation:get("ottoman")) -- Russo-Turkish war
Nation:get("russia"):declare_no_cb(Nation:get("circassia")) -- Russo-Circassian war
Nation:get("russia"):declare_no_cb(Nation:get("avar"))
Nation:get("united_kingdom"):declare_no_cb(Nation:get("burma")) -- Anglo-Burmesse war (1825)

-- Relevant nations to choose
-- Nation:get("united_kingdom"):make_relevant()
-- Nation:get("france"):make_relevant()
-- Nation:get("prussia"):make_relevant()
-- Nation:get("russia"):make_relevant()
-- Nation:get("austria"):make_relevant()
-- Nation:get("united_states"):make_relevant()

require('entities/ui/init')

--invalid_shit()
set_date(1825, 0, 0)
print("Industrial era mod")
print("Date: ", get_year(), "/", get_month(), "/", get_day())