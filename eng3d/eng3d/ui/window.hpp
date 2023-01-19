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
//      eng3d/ui/window.hpp
//
// Abstract:
//      Does some important stuff.
// ----------------------------------------------------------------------------

#pragma once

#include <cstddef>
#include <deque>
#include <functional>
#include <vector>
#include <string>

#include "eng3d/ui/widget.hpp"

namespace UI {
    /// @ingroup UI
    /// @brief Window widget, this widget is similar to a Group widget, the key
    /// difference is that this one can be moved along the screen.
    class Window: public Widget {
        glm::ivec2 start_drag_position;
    public:
        Window(int x, int y, unsigned w, unsigned h, Widget* parent = nullptr);
        virtual ~Window() override {}
        void set_close_btn_function(std::function<void(Widget&)> on_click);

        bool is_movable = true;
        // For the Lua API bindings
        int lua_on_close_btn = 0;
    private:
        Widget* close_btn = nullptr;
    };
}
