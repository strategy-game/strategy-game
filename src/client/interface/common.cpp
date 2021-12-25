#include "client/interface/common.hpp"
#include "client/interface/good_view.hpp"
#include "unified_render/path.hpp"
#include "nation.hpp"
#include "world.hpp"
#include "client/game_state.hpp"
#include "unit.hpp"
#include "province.hpp"
#include "product.hpp"
#include "building.hpp"
#include "unified_render/texture.hpp"

#include "client/client_network.hpp"
#include "action.hpp"
#include "io_impl.hpp"

using namespace Interface;

UnitButton::UnitButton(GameState& _gs, int x, int y, Unit* _unit, UI::Widget* parent)
    : gs{ _gs },
    unit{ _unit },
    UI::Button(x, y, parent->width, 24, parent)
{
    text(std::to_string(unit->size) + " " + unit->type->name);
    on_each_tick = ([](UI::Widget& w, void*) {
        auto& o = static_cast<UnitButton&>(*w.parent);
        w.text(std::to_string(o.unit->size) + " " + o.unit->type->name);
    });
}

UnitTypeButton::UnitTypeButton(GameState& _gs, int x, int y, UnitType* _unit_type, UI::Widget* parent)
    : gs{ _gs },
    unit_type{ _unit_type },
    UI::Group(x, y, parent->width, 24, parent)
{
    this->is_scroll = false;

    this->icon_img = new UI::Image(0, 0, 32, 24, nullptr, this);
    this->icon_img->current_texture = &g_texture_manager->load_texture(Path::get("ui/icons/unit_types/" + unit_type->ref_name + ".png"));

    this->name_btn = new UI::Button(0, 0, this->width - 32, 24, this);
    this->name_btn->right_side_of(*this->icon_img);
    this->name_btn->text(unit_type->name);
}

ProvinceButton::ProvinceButton(GameState& _gs, int x, int y, Province* _province, UI::Widget* parent)
    : gs{ _gs },
    province{ _province },
    UI::Button(x, y, parent->width, 24, parent)
{
    text(province->name);
    on_each_tick = ([](UI::Widget& w, void*) {
        auto& o = static_cast<ProvinceButton&>(w);
        if(o.gs.world->time % o.gs.world->ticks_per_month) return;
        w.text(o.province->name);
    });
}

NationButton::NationButton(GameState& _gs, int x, int y, Nation* _nation, UI::Widget* parent)
    : gs{ _gs },
    nation{ _nation },
    UI::Group(x, y, parent->width, 24, parent)
{
    this->is_scroll = false;

    this->flag_icon = new UI::Image(0, 0, 32, 24, nullptr, this);
    this->flag_icon->current_texture = &gs.get_nation_flag(*nation);
    this->flag_icon->on_each_tick = ([](UI::Widget& w, void*) {
        auto& o = static_cast<NationButton&>(*w.parent);
        if(o.gs.world->time % o.gs.world->ticks_per_month) return;
        w.current_texture = &o.gs.get_nation_flag(*o.nation);
    });

    this->name_btn = new UI::Button(0, 0, this->width - 32, 24, this);
    this->name_btn->right_side_of(*this->flag_icon);
    this->name_btn->text(nation->get_client_hint().alt_name);
    this->name_btn->on_each_tick = ([](UI::Widget& w, void*) {
        auto& o = static_cast<NationButton&>(*w.parent);
        if(o.gs.world->time % o.gs.world->ticks_per_month) return;
        w.text(o.nation->get_client_hint().alt_name);
    });
}

BuildingInfo::BuildingInfo(GameState& _gs, int x, int y, Building* _building, UI::Widget* parent)
    : gs{_gs},
    building{_building},
    UI::Group(x, y, parent->width, 24 * 8, parent)
{
    is_scroll = false;

    auto* name_btn = new UI::Button(0, 0, 128, 24, this);
    name_btn->on_each_tick = ([](UI::Widget& w, void*) {
        auto& o = static_cast<BuildingInfo&>(*w.parent);
        w.text(o.building->type->name);
    });

    unsigned int dx;

    auto* input_lab = new UI::Label(0, 0, "Inputs:", this);
    input_lab->below_of(*name_btn);
    dx = input_lab->width;
    for(const auto& good : building->type->inputs) {
        auto* icon_img = new UI::Image(dx, 0, 24, 24, &g_texture_manager->load_texture(Path::get("ui/icons/goods/" + good->ref_name + ".png")), this);
        icon_img->below_of(*name_btn);
        x += icon_img->width;
    }

    auto* output_lab = new UI::Label(0, 0, "Outputs:", this);
    output_lab->below_of(*input_lab);
    dx = output_lab->width;
    for(const auto& good : building->type->outputs) {
        auto* icon_img = new UI::Image(dx, 0, 24, 24, &g_texture_manager->load_texture(Path::get("ui/icons/goods/" + good->ref_name + ".png")), this);
        icon_img->below_of(*input_lab);
        x += icon_img->width;
    }
}

BuildingTypeButton::BuildingTypeButton(GameState& _gs, int x, int y, BuildingType* _building_type, UI::Widget* parent)
    : gs{ _gs },
    building_type{_building_type},
    UI::Button(x, y, parent->width, 24, parent)
{
    text(building_type->name);
}

TechnologyInfo::TechnologyInfo(GameState& _gs, int x, int y, Technology* _technology, UI::Widget* parent)
    : gs{ _gs },
    technology{_technology},
    UI::Group(x, y, parent->width, 48, parent)
{
    is_scroll = false;

    auto* chk = new UI::Checkbox(0, 0, 128, 24, this);
    chk->text(technology->name);
    chk->tooltip = new UI::Tooltip(chk, 512, 24);
    chk->on_each_tick = ([](UI::Widget& w, void*) {
        auto& o = static_cast<TechnologyInfo&>(*w.parent);
        if(o.technology == o.gs.curr_nation->focus_tech || !o.gs.curr_nation->research[o.gs.world->get_id(o.technology)]) {
            ((UI::Checkbox&)w).value = true;
        } else {
            ((UI::Checkbox&)w).value = false;
        }

        if(o.gs.curr_nation->can_research(o.technology)) {
            w.tooltip->text("We can research this");
        } else {
            std::string text = "";
            text = "We can't research this because we don't have ";
            for(const auto& req_tech : o.technology->req_technologies) {
                if(o.gs.curr_nation->research[o.gs.world->get_id(req_tech)] > 0.f) {
                    text += req_tech->name + ", ";
                }
            }
            w.tooltip->text(text);
        }
    });
    chk->on_click = ([](UI::Widget& w, void*) {
        auto& o = static_cast<TechnologyInfo&>(*w.parent);
        if(o.gs.curr_nation->can_research(o.technology)) {
            o.gs.client->send(Action::FocusTech::form_packet(o.technology));
        }
    });
    chk->on_each_tick(*chk, nullptr);

    auto* pgbar = new UI::ProgressBar(0, 24, 128, 24, 0.f, technology->cost, this);
    pgbar->on_each_tick = ([](UI::Widget& w, void*) {
        auto& o = static_cast<TechnologyInfo&>(*w.parent);
        ((UI::ProgressBar&)w).value = std::fabs(o.gs.curr_nation->research[o.gs.world->get_id(o.technology)] - o.technology->cost);
    });
}


PopInfo::PopInfo(GameState& _gs, int x, int y, Province* _province, int _index, UI::Widget* parent)
    : gs{ _gs },
    province{ _province },
    index{ _index },
    UI::Group(x, y, parent->width, 24, parent)
{
    this->is_scroll = false;

    this->size_btn = new UI::Button(0, 0, 96, 24, this);

    this->budget_btn = new UI::Button(0, 0, 128, 24, this);
    this->budget_btn->right_side_of(*this->size_btn);
    this->budget_btn->set_tooltip(new UI::Tooltip(this->budget_btn, 512, 24));

    this->religion_btn = new UI::Button(0, 0, 128, 24, this);
    this->religion_btn->right_side_of(*this->budget_btn);

    this->culture_btn = new UI::Button(0, 0, 128, 24, this);
    this->culture_btn->right_side_of(*this->religion_btn);

    if(index < province->pops.size()) {
        const Pop& pop = province->pops[index];
        this->size_btn->text(std::to_string(pop.size));
        this->budget_btn->text(std::to_string(pop.budget));
        this->religion_btn->text(pop.religion->name);
        this->culture_btn->text(pop.culture->name);
    }
    this->on_each_tick = ([](UI::Widget& w, void*) {
        auto& o = static_cast<PopInfo&>(w);
        if(o.gs.world->time % o.gs.world->ticks_per_month) return;
        if(o.index >= o.province->pops.size()) return;

        const Pop& pop = o.province->pops[o.index];
        o.size_btn->text(std::to_string(pop.size));
        o.budget_btn->text(std::to_string(pop.budget / pop.size));
        o.budget_btn->tooltip->text("Total of " + std::to_string(pop.budget));
        o.religion_btn->text(pop.religion->name);
        o.culture_btn->text(pop.culture->name);
    });
}

ProductInfo::ProductInfo(GameState& _gs, int x, int y, Product* _product, UI::Widget* parent)
    : gs{ _gs },
    product{ _product },
    UI::Group(x, y, parent->width, 24, parent)
{
    this->is_scroll = false;

    this->good_btn = new UI::Button(0, 0, 96, 24, this);
    this->good_btn->text(product->good->name);
    this->good_btn->on_click = ([](UI::Widget& w, void*) {
        auto& o = static_cast<ProductInfo&>(*w.parent);
        new GoodView(o.gs, o.product->good);
    });

    this->price_rate_btn = new UI::Button(0, 0, 96, 24, this);
    this->price_rate_btn->right_side_of(*this->good_btn);

    this->price_chart = new UI::Chart(0, 0, 96, 24, this);
    this->price_chart->right_side_of(*this->price_rate_btn);
    this->price_chart->on_click = ([](UI::Widget& w, void*) {
        auto& o = static_cast<ProductInfo&>(*w.parent);
        new ProductView(o.gs, o.product);
    });

    this->supply_chart = new UI::Chart(0, 0, 96, 24, this);
    this->supply_chart->right_side_of(*this->price_chart);
    this->supply_chart->on_click = ([](UI::Widget& w, void*) {
        auto& o = static_cast<ProductInfo&>(*w.parent);
        new ProductView(o.gs, o.product);
    });

    this->demand_chart = new UI::Chart(0, 0, 96, 24, this);
    this->demand_chart->right_side_of(*this->supply_chart);
    this->demand_chart->on_click = ([](UI::Widget& w, void*) {
        auto& o = static_cast<ProductInfo&>(*w.parent);
        new ProductView(o.gs, o.product);
    });

    this->on_each_tick = ([](UI::Widget& w, void*) {
        auto& o = static_cast<ProductInfo&>(w);

        // Only update every ticks_per_month ticks
        if(o.gs.world->time % o.gs.world->ticks_per_month) return;

        o.price_chart->data.push_back(o.product->price);
        if(o.price_chart->data.size() >= 30)
            o.price_chart->data.pop_back();
        o.price_chart->text(std::to_string(o.product->price));
        
        o.supply_chart->data.push_back(o.product->supply);
        if(o.supply_chart->data.size() >= 30)
            o.supply_chart->data.pop_back();

        o.demand_chart->data.push_back(o.product->demand);
        if(o.demand_chart->data.size() >= 30)
            o.demand_chart->data.pop_back();
        
        o.price_rate_btn->text(std::to_string(o.product->price_vel));

        if(o.product->price_vel >= 0.f) {
            o.price_rate_btn->text_color = UnifiedRender::Color(0, 255, 0);
        } else {
            o.price_rate_btn->text_color = UnifiedRender::Color(255, 0, 0);
        }
    });
}