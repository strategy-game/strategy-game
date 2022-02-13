// Symphony of Empires
// Copyright (C) 2021, Symphony of Empires contributors
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// ----------------------------------------------------------------------------
// Name:
//      client/interface/ai.cpp
//
// Abstract:
//      Does some important stuff.
// ----------------------------------------------------------------------------

#include "client/interface/ai.hpp"
#include "io_impl.hpp"
#include "unified_render/ui/components.hpp"
#include "client/client_network.hpp"
#include "unified_render/locale.hpp"

using namespace Interface;

AISettingsWindow::AISettingsWindow(GameState& _gs)
    : UI::Window(0, 0, 512, 256),
    gs{ _gs }
{
    auto* build_prod_chk = new UI::Checkbox(0, 0, this->width, 24, this);
    build_prod_chk->text(UnifiedRender::Locale::translate("Build and production"));
    build_prod_chk->set_on_click([this](UI::Widget& w) {
        this->gs.curr_nation->ai_do_build_production = ((UI::Checkbox&)w).get_value();
    });

    auto* cmd_chk = new UI::Checkbox(0, 0, this->width, 24, this);
    cmd_chk->below_of(*build_prod_chk);
    cmd_chk->text(UnifiedRender::Locale::translate("Command troops"));
    cmd_chk->set_on_click([](UI::Widget& w) {
        auto& o = static_cast<AISettingsWindow&>(*w.parent);
        o.gs.curr_nation->ai_do_cmd_troops = ((UI::Checkbox&)w).get_value();
    });

    auto* diplo_chk = new UI::Checkbox(0, 0, this->width, 24, this);
    diplo_chk->below_of(*cmd_chk);
    diplo_chk->text(UnifiedRender::Locale::translate("Diplomacy"));
    diplo_chk->set_on_click([](UI::Widget& w) {
        auto& o = static_cast<AISettingsWindow&>(*w.parent);
        o.gs.curr_nation->ai_do_diplomacy = ((UI::Checkbox&)w).get_value();
    });

    auto* policies_chk = new UI::Checkbox(0, 0, this->width, 24, this);
    policies_chk->below_of(*diplo_chk);
    policies_chk->text(UnifiedRender::Locale::translate("Policies"));
    policies_chk->set_on_click([](UI::Widget& w) {
        auto& o = static_cast<AISettingsWindow&>(*w.parent);
        o.gs.curr_nation->ai_do_policies = ((UI::Checkbox&)w).get_value();
    });

    auto* research_chk = new UI::Checkbox(0, 0, this->width, 24, this);
    research_chk->below_of(*policies_chk);
    research_chk->text(UnifiedRender::Locale::translate("Research"));
    research_chk->set_on_click([](UI::Widget& w) {
        auto& o = static_cast<AISettingsWindow&>(*w.parent);
        o.gs.curr_nation->ai_do_research = ((UI::Checkbox&)w).get_value();
    });

    auto* unit_production_chk = new UI::Checkbox(0, 0, this->width, 24, this);
    unit_production_chk->below_of(*research_chk);
    unit_production_chk->text(UnifiedRender::Locale::translate("Unit Production"));
    unit_production_chk->set_on_click([](UI::Widget& w) {
        auto& o = static_cast<AISettingsWindow&>(*w.parent);
        o.gs.curr_nation->ai_do_unit_production = ((UI::Checkbox&)w).get_value();
    });

    auto* hdl_event_chk = new UI::Checkbox(0, 0, this->width, 24, this);
    hdl_event_chk->below_of(*unit_production_chk);
    hdl_event_chk->text(UnifiedRender::Locale::translate("Handle events"));
    hdl_event_chk->set_on_click([](UI::Widget& w) {
        auto& o = static_cast<AISettingsWindow&>(*w.parent);
        o.gs.curr_nation->ai_handle_events = ((UI::Checkbox&)w).get_value();
    });

    auto* hdl_treaties_chk = new UI::Checkbox(0, 0, this->width, 24, this);
    hdl_treaties_chk->below_of(*hdl_event_chk);
    hdl_treaties_chk->text(UnifiedRender::Locale::translate("Handle treaties"));
    hdl_treaties_chk->set_on_click([](UI::Widget& w) {
        auto& o = static_cast<AISettingsWindow&>(*w.parent);
        o.gs.curr_nation->ai_handle_treaties = ((UI::Checkbox&)w).get_value();
    });

    auto* close_btn = new UI::CloseButton(0, 0, this->width, 24, this);
    close_btn->below_of(*hdl_treaties_chk);
    close_btn->text(UnifiedRender::Locale::translate("Close"));
    close_btn->on_click = ([](UI::Widget& w) {
        auto& o = static_cast<AISettingsWindow&>(*w.parent);

        o.gs.client->send(Action::AiControl::form_packet(o.gs.curr_nation));
        w.parent->kill();
    });
}