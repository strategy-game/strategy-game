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
//      eng3d/ui/piechart.hpp
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

#include <glm/vec2.hpp>
#include "eng3d/rectangle.hpp"
#include "eng3d/color.hpp"
#include "eng3d/ui/widget.hpp"
#include "eng3d/primitive.hpp"

namespace UI {
	class Context;
    /// @ingroup UI
    /// @brief Piechart widget
	class PieChart: public Widget {
	public:
		PieChart(int x, int y, unsigned w, unsigned h, std::vector<ChartData> data = std::vector<ChartData>(), Widget* _parent = nullptr);
		PieChart(int x, int y, unsigned w, unsigned h, Widget* _parent = nullptr);
		virtual ~PieChart() override {}
		virtual void on_render(Context& ctx, Eng3D::Rect viewport) override;
		void set_data(std::vector<ChartData> data);
	private:
        static void on_hover_default(Widget& w, glm::ivec2 mouse_pos, glm::ivec2 widget_pos);
		inline void draw_triangle(float start_angle, float end_angle, Eng3D::Color color);
		std::vector<ChartData> data;
		Tooltip* slice_tooltip;
		float max = 1.f;
	};
}
