// Unified Render - General purpouse game engine
// Copyright (C) 2021, Unified Render contributors
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
//      font_sdf.cpp
//
// Abstract:
//      Does important stuff.
// ----------------------------------------------------------------------------

#include "unified_render/font_sdf.hpp"
#include "unified_render/state.hpp"
#include "unified_render/texture.hpp"
#include "unified_render/path.hpp"
#include "unified_render/primitive.hpp"
#include "unified_render/shader.hpp"

using namespace UnifiedRender;

#include <iostream>
#include <fstream>
#include <string>


FontSDF::FontSDF(std::string filename) {
    sdf_font_shader = OpenGL::Program::create("shaders/font_sdf.vs", "shaders/font_sdf.fs");

    auto tex_man = UnifiedRender::State::get_instance().tex_man;

    UnifiedRender::TextureOptions mipmap_options;
    mipmap_options.min_filter = GL_LINEAR;
    mipmap_options.mag_filter = GL_LINEAR;
    mipmap_options.wrap_s = GL_CLAMP_TO_EDGE;
    mipmap_options.wrap_t = GL_CLAMP_TO_EDGE;
    atlas = &tex_man->load(Path::get(filename + ".png"), mipmap_options);

    char buff;
    uint32_t unicode;
    float advance, top, bottom, left, right;
    std::string line;
    std::ifstream glyph_data(Path::get(filename + ".csv"));
    if(glyph_data.is_open()) {
        while(std::getline(glyph_data, line)) {
            std::istringstream data(line);
            data >> unicode >> buff;
            data >> advance >> buff;
            data >> left >> buff >> bottom >> buff >> right >> buff >> top >> buff;
            Rectangle plane_bounds(left, top, right - left, bottom - top);
            data >> left >> buff >> bottom >> buff >> right >> buff >> top;
            left   /= atlas->width;
            right  /= atlas->width;
            top    /= atlas->height;
            bottom /= atlas->height;
            Rectangle atlas_bounds(left, top, right - left, bottom - top);
            Glyph glyph(advance, atlas_bounds, plane_bounds);
            unicode_map.insert({ unicode, glyph });
        }
    }
}

Label3d* FontSDF::gen_text(std::string text, glm::vec3 center, glm::vec3 top, glm::vec3 right, float width) {
    UnifiedRender::Color color = UnifiedRender::Color(0.f, 0.f, 0.f);
    float text_width = 0;
    for(char& character : text) {
        uint16_t unicode = (u_int16_t)character;
        if(!unicode_map.count(unicode))
            continue;
        Glyph& glyph = unicode_map.at(unicode);
        text_width += glyph.advance;
    }
    float scale = width / text_width;
    top = glm::normalize(top);
    right = glm::normalize(right);
    glm::vec3 start = center - right * width * 0.5f;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> tex_coords;
    for(char& character : text) {
        uint16_t unicode = (u_int16_t)character;
        if(!unicode_map.count(unicode))
            continue;
        Glyph glyph = unicode_map.at(unicode);

        glm::vec2 atlas_tl(glyph.atlas_bounds.left, glyph.atlas_bounds.top);
        glm::vec2 atlas_bl(glyph.atlas_bounds.left, glyph.atlas_bounds.bottom);
        glm::vec2 atlas_tr(glyph.atlas_bounds.right, glyph.atlas_bounds.top);
        glm::vec2 atlas_br(glyph.atlas_bounds.right, glyph.atlas_bounds.bottom);

        glm::vec2 plane_tl(glyph.plane_bounds.left, glyph.plane_bounds.top);
        glm::vec2 plane_bl(glyph.plane_bounds.left, glyph.plane_bounds.bottom);
        glm::vec2 plane_tr(glyph.plane_bounds.right, glyph.plane_bounds.top);
        glm::vec2 plane_br(glyph.plane_bounds.right, glyph.plane_bounds.bottom);

        glm::mat2x3 base(right, top);
        glm::vec3 char_tl = start + base * plane_tl * scale;
        glm::vec3 char_bl = start + base * plane_bl * scale;
        glm::vec3 char_tr = start + base * plane_tr * scale;
        glm::vec3 char_br = start + base * plane_br * scale;

        tex_coords.push_back(atlas_tl);
        positions.push_back(char_tl);
        tex_coords.push_back(atlas_bl);
        positions.push_back(char_bl);
        tex_coords.push_back(atlas_br);
        positions.push_back(char_br);
        tex_coords.push_back(atlas_br);
        positions.push_back(char_br);
        tex_coords.push_back(atlas_tr);
        positions.push_back(char_tr);
        tex_coords.push_back(atlas_tl);
        positions.push_back(char_tl);

        start += right * glyph.advance * scale;
    }

    TriangleList* triangles = new TriangleList(positions, tex_coords);
    return new Label3d(triangles, scale);
}

void FontSDF::draw(std::vector<Label3d*>& labels, glm::mat4 projection, glm::mat4 view) {
    sdf_font_shader->use();
    sdf_font_shader->set_uniform("projection", projection);
    sdf_font_shader->set_uniform("view", view);
    sdf_font_shader->set_texture(0, "atlas", *atlas);
    for(auto& label : labels) {
        sdf_font_shader->set_uniform("pxRange", label->size * 0.25f);
        label->draw();
    }
}

Label3d::Label3d(TriangleList* _triangles, float _size): triangles(_triangles), size{ _size } {};
Label3d::~Label3d() { delete triangles; };
void Label3d::draw() { triangles->draw(); };