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
//      client/orbit_camera.hpp
//
// Abstract:
//      Does some important stuff.
// ----------------------------------------------------------------------------

#pragma once

#include <cmath>

#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/intersect.hpp>

#include "unified_render/print.hpp"
#include "client/camera.hpp"
#include "value_chase.hpp"

class OrbitCamera: public Camera {
public:
    float radius;
    glm::vec3 target;
    ValueChase<glm::vec3> chase{ 0.2f };

    OrbitCamera(glm::vec2 screen_size, glm::vec2 map_size, float _radius)
        : Camera(screen_size, map_size)
    {
        radius = _radius;
        map_position = glm::vec3(M_PI, M_PI * 0.5f, radius * 1.5f);
        target = map_position;
    }

    OrbitCamera(Camera* camera, float _radius)
        : radius{_radius}, Camera(camera)
    {
        world_position = map_position;
        target = map_position;
    }

    void move(float x_dir, float y_dir, float z_dir) override {
        float scale = glm::abs(target.z * map_size.x / 1e5);
        target.x += x_dir * scale;
        target.y += y_dir * scale;
        target.y = glm::clamp(target.y, 0.f, map_size.y);
        target.z += z_dir * scale;
        target.z = glm::clamp(target.z, 0.f, map_size.x / 2.f);
    }

    void update(void) override {
        map_position = chase.move_toward(map_position, target);
        map_position.y = glm::clamp(map_position.y, 0.f, map_size.y);
        map_position.z = glm::clamp(map_position.z, 0.f, map_size.x / 2.f);

        glm::vec3 normalized_pos = map_position / glm::vec3(map_size.x, map_size.y, map_size.x / 2.f);
        glm::vec2 radiance_pos;
        float pi = glm::pi<float>();
        radiance_pos.x = glm::mod(normalized_pos.x * 2.f * pi, 2.f * pi);
        radiance_pos.y = glm::max(0.f, glm::min(pi, normalized_pos.y * pi));

        float distance = radius + normalized_pos.z * radius * 7.;
        world_position.x = distance * cos(radiance_pos.x) * sin(radiance_pos.y);
        world_position.y = distance * sin(radiance_pos.x) * sin(radiance_pos.y);
        world_position.z = distance * cos(radiance_pos.y);
    };

    glm::mat4 get_projection() override {
        float aspect_ratio = screen_size.x / screen_size.y;
        return glm::perspective(glm::radians(fov), aspect_ratio, near_plane, far_plane);
    };

    void set_pos(float x, float y) override {
        map_position.x = glm::mod(x, map_size.x);
        map_position.y = glm::clamp(y, 0.f, map_size.y);
        target = map_position;

        update();
    }

    glm::mat4 get_view() override {
        glm::vec3 look_at = glm::vec3(0);
        glm::vec3 up_vector = glm::vec3(0.f, -1.f, 0.f);

        glm::vec3 normalized_pos = map_position;
        normalized_pos.x = glm::mod(normalized_pos.x, map_size.x);
        normalized_pos = normalized_pos / glm::vec3(map_size.x, map_size.y, radius);
        glm::vec2 radiance_pos;
        float pi = glm::pi<float>();
        radiance_pos.x = glm::mod(normalized_pos.x * 2.f * pi, 2.f * pi);
        radiance_pos.y = glm::max(0.f, glm::min(pi, normalized_pos.y * pi));
        up_vector.x = -cos(radiance_pos.x) * cos(radiance_pos.y);
        up_vector.y = -sin(radiance_pos.x) * cos(radiance_pos.y);
        up_vector.z = sin(radiance_pos.y);
        return glm::lookAt(world_position, look_at, up_vector);
    };

    glm::vec3 get_map_pos() override {
        glm::vec3 out_pos = map_position;
        out_pos.x = glm::mod(out_pos.x, map_size.x);
        return out_pos;
    }

    bool get_cursor_map_pos(std::pair<int, int> mouse_pos, glm::ivec2& out_pos) override {
        float mouse_x = mouse_pos.first;
        float mouse_y = screen_size.y - 1.f - mouse_pos.second;

        glm::mat4 view = get_view();
        glm::mat4 projection = get_projection();

        glm::vec3 world_space_near = glm::unProject(
            glm::vec3(mouse_x, mouse_y, 0.f),
            view, projection,
            glm::vec4(0.f, 0.f, screen_size));

        glm::vec3 world_space_far = glm::unProject(
            glm::vec3(mouse_x, mouse_y, 1.f),
            view, projection,
            glm::vec4(0.f, 0.f, screen_size)
        );

        glm::vec3 ray_direction = world_space_far - world_space_near;
        ray_direction = glm::normalize(ray_direction);

        float distance = 0.f;
        bool hit = glm::intersectRaySphere(world_space_near, ray_direction, glm::vec3(0, 0, 0), radius * radius, distance);

        glm::vec3 intersection_point = world_space_near + ray_direction * distance;
        float pi = glm::pi<float>();
        float y_rad = glm::acos(intersection_point.z / radius);
        float x_rad = glm::atan(intersection_point.y, intersection_point.x);

        out_pos.x = map_size.x * x_rad / (2.f * pi);
        out_pos.y = map_size.y * y_rad / (pi);

        return hit;
    };
};