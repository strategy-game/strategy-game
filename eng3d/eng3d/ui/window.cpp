// Symphony of Empires
// Copyright (C) 2021, Symphony of Empires contributors
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// ----------------------------------------------------------------------------
// Name:
//      eng3d/ui/window.cpp
//
// Abstract:
//      Does some important stuff.
// ----------------------------------------------------------------------------

#include "eng3d/ui/window.hpp"
#include "eng3d/ui/image.hpp"
#include "eng3d/ui/div.hpp"
#include "eng3d/ui/ui.hpp"
#include "eng3d/state.hpp"
#include "eng3d/texture.hpp"

using namespace UI;

Window::Window(int _x, int _y, unsigned w, unsigned h, Widget* _parent)
    : Widget(_parent, _x, _y, w, h, UI::WidgetType::WINDOW),
    is_movable{ true }
{
    auto& s = Eng3D::State::get_instance();
    this->padding = glm::ivec2(8, 24 + 8);
    this->current_texture = s.tex_man.load(s.package_man.get_unique("gfx/window_background.png"));
    this->is_scroll = true;
    this->text_color = Eng3D::Color(1.f, 1.f, 1.f);

    glm::ivec2 size(4, 4);
    glm::ivec2 texture_size(10, 10);
    glm::ivec2 offset(0, 24);
    this->border = Border(g_ui_context->border_tex, size, texture_size, offset);

    this->set_on_drag([this](glm::ivec2 start_pos, glm::ivec2 current_pos) {
        if (start_pos == current_pos) {
            this->start_drag_position = glm::ivec2{this->x, this->y};
        }
        if(!this->is_movable) return;
        const auto move_offset = current_pos - start_pos;
        this->x = this->start_drag_position.x + move_offset.x;
        this->y = this->start_drag_position.y + move_offset.y;
    });
}

// There are a number of improvement to be made here
// * This places the button on in the end of the children vector,
//      would be better to have it in the beginning
// * This function is not general, would be nice to set your own exit btn
// * We should add an on_close event that is called when exiting
void Window::set_close_btn_function(std::function<void(Widget&)> _on_click) {
    if(_on_click) {
        if(!this->close_btn) {
            const int size = 24;
            auto& btn_wrapper = this->make_widget<UI::Div>(-size - padding.x, -padding.y, size, size);
            btn_wrapper.origin = UI::Origin::UPPER_RIGHT;

            btn_wrapper.make_widget<UI::Image>(0, 0, size, size, "gfx/ui/button/exit_btn_shadow.png", true);
            const int btn_size = (int)(size * 0.75f);
            const int offset = (size - btn_size) / 2;
            auto* btn = Image::make_transparent(offset, offset, btn_size, btn_size, "gfx/ui/button/exit_btn.png", true, &btn_wrapper);
            btn->set_on_click(_on_click);
            this->close_btn = &btn_wrapper;
        }
    } else {
        if(this->close_btn) {
            this->close_btn->kill();
            this->close_btn = nullptr;
        }
    }
}
