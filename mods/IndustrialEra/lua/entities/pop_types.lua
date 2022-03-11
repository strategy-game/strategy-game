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
--  	pop_types.lua
--
-- 	Abstract:
--      Does important stuff
-- ----------------------------------------------------------------------------

-- Pop types

burgeoise = PopType:new{
    name = _("Burgeoise"),
    ref_name = "burgeoise",
    social_value = 2.0,
    is_burgeoise = true,
    needs = {
        {bread.id, 1.0},
        {salt.id, 0.1}
    }
}
burgeoise:register()

artisan = PopType:new{
    name = _("Artisan"),
    ref_name = "artisan",
    social_value = 1.0
}
artisan:register()

bureaucrat = PopType:new{
    name = _("Bureaucrat"),
    ref_name = "bureaucrat",
    social_value = 2.5
}
bureaucrat:register()

clergymen = PopType:new{
    name = _("Clergymen"),
    ref_name = "clergymen",
    social_value = 2.5
}
clergymen:register()

soldier = PopType:new{
    name = _("Soldier"),
    ref_name = "soldier",
    social_value = 1.5
}
soldier:register()

farmer = PopType:new{
    name = _("Farmer"),
    ref_name = "farmer",
    social_value = 1.0,
    is_farmer = true
}
farmer:register()

laborer = PopType:new{
    name = _("Laborer"),
    ref_name = "laborer",
    social_value = 1.0,
    is_laborer = true
}
laborer:register()

slave = PopType:new{
    name = _("Slave"),
    ref_name = "slave",
    social_value = 0.1,
    is_slave = true
}
slave:register()

-- TODO: Remove these, because they are deprecated
officer = PopType:new{
    name = _("Officer"),
    ref_name = "officer",
    social_value = 1.5
}
officer:register()

craftsmen = PopType:new{
    name = _("Craftsmen"),
    ref_name = "craftsmen",
    social_value = 1.5
}
craftsmen:register()