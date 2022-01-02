#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <mutex>
#include <memory>
// Required before GL/gl.h
#include <GL/glew.h>
#include <GL/gl.h>

#include "unified_render/path.hpp"
#include "unified_render/print.hpp"
#include "unified_render/texture.hpp"
#include "unified_render/primitive.hpp"
#include "unified_render/shader.hpp"
#include "unified_render/framebuffer.hpp"
#include "unified_render/model.hpp"

#include "client/map.hpp"
#include "client/map_render.hpp"
#include "client/game_state.hpp"
#include "unified_render/model.hpp"
#include "client/interface/province_view.hpp"
#include "client/interface/lobby.hpp"
#include "world.hpp"
#include "client/orbit_camera.hpp"
#include "client/flat_camera.hpp"
#include "client/camera.hpp"
#include "province.hpp"

Map::Map(const World& _world, int screen_width, int screen_height)
    : world(_world)
{
    camera = new FlatCamera(screen_width, screen_height);

    map_render = new MapRender(world);

    // Shader used for drawing the models using custom model render
    obj_shader = UnifiedRender::OpenGL::Program::create("shaders/simple_model.vs", "shaders/simple_model.fs");

    // Set the mapmode
    set_map_mode(political_map_mode);

    print_info("Preloading-important stuff");
    // Query the initial nation flags
    for(const auto& nation : world.nations) {
        UnifiedRender::TextureOptions mipmap_options{};
        mipmap_options.wrap_s = GL_REPEAT;
        mipmap_options.wrap_t = GL_REPEAT;
        mipmap_options.min_filter = GL_NEAREST_MIPMAP_LINEAR;
        mipmap_options.mag_filter = GL_LINEAR;
        std::string path = Path::get("ui/flags/" + nation->ref_name + "_" +
            (nation->ideology == nullptr ? "none" : nation->ideology->ref_name) + ".png"
        );
        auto flag_texture = &UnifiedRender::State::get_instance().tex_man->load(path, mipmap_options);
        flag_texture->gen_mipmaps();
        nation_flags.push_back(flag_texture);
    }

    for(const auto& building_type : world.building_types) {
        std::string path;
        path = Path::get("3d/building_types/" + building_type->ref_name + ".obj");
        building_type_models.push_back(&UnifiedRender::State::get_instance().model_man->load(path));
        path = Path::get("ui/icons/building_types/" + building_type->ref_name + ".png");
        building_type_icons.push_back(&UnifiedRender::State::get_instance().tex_man->load(path));
    }

    for(const auto& unit_type : world.unit_types) {
        std::string path;
        path = Path::get("3d/unit_types/" + unit_type->ref_name + ".obj");
        unit_type_models.push_back(&UnifiedRender::State::get_instance().model_man->load(path));
        path = Path::get("ui/icons/unit_types/" + unit_type->ref_name + ".png");
        unit_type_icons.push_back(&UnifiedRender::State::get_instance().tex_man->load(path));
    }
}

Map::~Map() {

}

void Map::set_view(MapView view) {
    view_mode = view;
    int old_width = camera->screen_size.x;
    int old_height = camera->screen_size.y;
    delete camera;
    if(view == MapView::PLANE_VIEW) {
        camera = new FlatCamera(old_width, old_height);
    } else if(view == MapView::SPHERE_VIEW) {
        camera = new OrbitCamera(old_width, old_height, 100.f);
    }
}

// The standard map mode with each province color = country color
std::vector<ProvinceColor> political_map_mode(const World& world) {
    std::vector<ProvinceColor> province_color;
    for(unsigned int i = 0; i < world.provinces.size(); i++) {
        Nation* province_owner = world.provinces[i]->owner;
        if(province_owner == nullptr) {
            province_color.push_back(ProvinceColor(i, UnifiedRender::Color::rgba32(0xffdddddd)));
        } else if(province_owner->cached_id == (Nation::Id)-1) {
            province_color.push_back(ProvinceColor(i, UnifiedRender::Color::rgba32(0xffdddddd)));
        } else {
            province_color.push_back(ProvinceColor(i, UnifiedRender::Color::rgba32(province_owner->get_client_hint().color)));
        }
    }
    // Water
    province_color.push_back(ProvinceColor((Province::Id)-2, UnifiedRender::Color::rgba32(0x00000000)));
    // Land
    province_color.push_back(ProvinceColor((Province::Id)-1, UnifiedRender::Color::rgba32(0xffdddddd)));
    return province_color;
}

void Map::reload_shaders() {
    map_render->reload_shaders();
}

void Map::set_map_mode(mapmode_generator _mapmode_func){
    mapmode_func = _mapmode_func;
    update_mapmode();
}

void Map::draw_flag(const Nation* nation) {
    // Draw a flag that "waves" with some cheap wind effects it
    // looks nice and it's super cheap to make - only using sine
    const float n_steps = 8.f; // Resolution of flag in one side (in vertices)
    const float step = 90.f; // Steps per vertice

    auto flag = UnifiedRender::OpenGL::PackedModel<glm::vec3, glm::vec2>(GL_TRIANGLE_STRIP);
    for(float r = 0.f; r <= (n_steps * step); r += step) {
        float sin_r = (sin(r + wind_osc) / 24.f);

        sin_r = sin(r + wind_osc) / 24.f;
        flag.buffer.push_back(UnifiedRender::OpenGL::PackedData<glm::vec3, glm::vec2>(
            glm::vec3(((r / step) / n_steps) * 1.5f, sin_r, -2.f),
            glm::vec2((r / step) / n_steps, 0.f)
            ));

        sin_r = sin(r + wind_osc + 160.f) / 24.f;
        flag.buffer.push_back(UnifiedRender::OpenGL::PackedData<glm::vec3, glm::vec2>(
            glm::vec3(((r / step) / n_steps) * 1.5f, sin_r, -1.f),
            glm::vec2((r / step) / n_steps, 1.f)
            ));
    }

    flag.vao.bind();
    flag.vbo.bind(GL_ARRAY_BUFFER);
    glBufferData(GL_ARRAY_BUFFER, flag.buffer.size() * sizeof(flag.buffer[0]), &flag.buffer[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(flag.buffer[0]), (void*)0);  // Vertices
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(flag.buffer[0]), (void*)(3 * sizeof(float)));  // Texcoords
    glEnableVertexAttribArray(1);

    nation_flags[world.get_id(nation)]->bind();
    flag.draw();
}

#include "client/client_network.hpp"
#include "unified_render/serializer.hpp"
#include "io_impl.hpp"
#include "action.hpp"
#ifdef windows
#	define bswap_32(x) _byteswap_ulong(x)
#	define bswap_64(x) _byteswap_uint64(x)
#else
#	include <byteswap.h>
#endif

void Map::handle_click(GameState& gs, SDL_Event event) {
    Input& input = gs.input;

    if(input.select_pos.first < 0 || input.select_pos.first >= gs.world->width || input.select_pos.second < 0 || input.select_pos.second >= gs.world->height) {
        return;
    }

    if(event.button.button == SDL_BUTTON_LEFT) {
        std::pair<float, float>& select_pos = input.select_pos;
        const Tile& tile = gs.world->get_tile(select_pos.first, select_pos.second);
        switch(gs.current_mode) {
        case MapMode::COUNTRY_SELECT:
            if(tile.province_id < (Province::Id)-3) {
                auto province = world.provinces[tile.province_id];
                if(province->controller == nullptr) {
                    break;
                }
                gs.select_nation->change_nation(province->controller->cached_id);
            }
            break;
        case MapMode::NORMAL:
            // Check if we selected an unit
            input.selected_units.clear();
            for(const auto& unit : gs.world->units) {
                // We can't control others units
                if(unit->owner != gs.curr_nation) {
                    continue;
                }

                std::pair<float, float> pos = unit->get_pos();

                std::pair<float, float> start_pos = input.drag_coord;
                if(input.select_pos.first < input.drag_coord.first) {
                    start_pos.first = input.select_pos.first;
                } if(input.select_pos.second < input.drag_coord.second) {
                    start_pos.second = input.select_pos.second;
                }

                std::pair<float, float> end_pos = input.drag_coord;
                if(input.select_pos.first > input.drag_coord.first) {
                    end_pos.first = input.select_pos.first;
                } if(input.select_pos.second > input.drag_coord.second) {
                    end_pos.second = input.select_pos.second;
                }

                if(pos.first >= start_pos.first && pos.first <= end_pos.first && pos.second >= start_pos.second && pos.second <= end_pos.second) {
                    input.selected_units.push_back(unit);
                }
            }
            input.is_drag = false;

            if(input.selected_units.empty()) {
                // Show province information when clicking on a province
                if(tile.province_id < gs.world->provinces.size()) {
                    new Interface::ProvinceView(gs, gs.world->provinces[tile.province_id]);
                    return;
                }
            }
            break;
        default:
            break;
        }
        return;
    } else if(event.button.button == SDL_BUTTON_RIGHT) {
        const Tile& tile = gs.world->get_tile(input.select_pos.first, input.select_pos.second);
        if(tile.province_id == (Province::Id)-1) {
            return;
        }

        Province* province = gs.world->provinces[tile.province_id];

        if(input.selected_units.empty()) {
            FILE* fp = fopen("test.lua", "a+t");
            if(fp != nullptr) {
                fprintf(fp, "{ ref_name = \"%s\", name = _(\"%s\"), color = 0x%06x },\r\n", province->ref_name.c_str(), province->name.c_str(), bswap_32((province->color & 0x00ffffff) << 8));
                fclose(fp);
                gs.curr_nation->give_province(*province);
            }
        }

        for(const auto& unit : input.selected_units) {
            if(!unit->province->is_neighbour(*province)) {
                continue;
            }

            if(unit->province->controller != nullptr && unit->province->controller != gs.curr_nation) {
                // Must either be our ally, have military access with them or be at war
                const NationRelation& relation = gs.curr_nation->relations[gs.world->get_id(unit->province->controller)];
                if(!(relation.has_war || relation.has_alliance || relation.has_military_access)) {
                    continue;
                }
            }

            UnifiedRender::Networking::Packet packet = UnifiedRender::Networking::Packet();
            Archive ar = Archive();
            ActionType action = ActionType::UNIT_CHANGE_TARGET;
            ::serialize(ar, &action);
            ::serialize(ar, &unit);
            ::serialize(ar, &province);
            packet.data(ar.get_buffer(), ar.size());
            std::scoped_lock lock(gs.client->pending_packets_mutex);
            gs.client->pending_packets.push_back(packet);
        }
    }
}

void Map::update(const SDL_Event& event, Input& input) {
    std::pair<int, int>& mouse_pos = input.mouse_pos;
    // std::pair<float, float>& select_pos = input.select_pos;
    switch(event.type) {
    case SDL_JOYBUTTONDOWN:
    case SDL_MOUSEBUTTONDOWN:
        SDL_GetMouseState(&mouse_pos.first, &mouse_pos.second);

        input.is_drag = false;
        if(event.button.button == SDL_BUTTON_MIDDLE) {
            input.last_camera_drag_pos = camera->get_map_pos(mouse_pos);
            input.last_camera_mouse_pos = mouse_pos;
        } else if(event.button.button == SDL_BUTTON_LEFT) {
            if(!input.is_drag) {
                input.drag_coord = input.select_pos;
                if(view_mode == MapView::SPHERE_VIEW) {
                    input.drag_coord.first = (int)(world.width * input.drag_coord.first / (2. * M_PI));
                    input.drag_coord.second = (int)(world.height * input.drag_coord.second / M_PI);
                } else {
                    input.drag_coord.first = (int)input.drag_coord.first;
                    input.drag_coord.second = (int)input.drag_coord.second;
                }
                input.is_drag = true;
            }
        }
        break;
    case SDL_JOYBUTTONUP:
    case SDL_MOUSEBUTTONUP:
        break;
    case SDL_MOUSEMOTION:
        SDL_GetMouseState(&mouse_pos.first, &mouse_pos.second);
        if(view_mode == MapView::SPHERE_VIEW) {
            if(input.middle_mouse_down) {  // Drag the map with middlemouse
                float scale = glm::length(camera->position) / GLOBE_RADIUS;
                float x_pos = input.last_camera_drag_pos.first - (mouse_pos.first - input.last_camera_mouse_pos.first) * 0.001 * scale;
                float y_pos = input.last_camera_drag_pos.second - (mouse_pos.second - input.last_camera_mouse_pos.second) * 0.001 * scale;
                camera->set_pos(x_pos, y_pos);
            }
            input.select_pos = camera->get_map_pos(input.mouse_pos);
            input.select_pos.first = (int)(world.width * input.select_pos.first / (2. * M_PI));
            input.select_pos.second = (int)(world.height * input.select_pos.second / M_PI);
        }
        else {
            if(input.middle_mouse_down) {  // Drag the map with middlemouse
                std::pair<float, float> map_pos = camera->get_map_pos(mouse_pos);
                float x_pos = camera->position.x + input.last_camera_drag_pos.first - map_pos.first;
                float y_pos = camera->position.y + input.last_camera_drag_pos.second - map_pos.second;
                camera->set_pos(x_pos, y_pos);
            }
            input.select_pos = camera->get_map_pos(input.mouse_pos);
            input.select_pos.first = (int)input.select_pos.first;
            input.select_pos.second = (int)input.select_pos.second;
        }
        break;
    case SDL_MOUSEWHEEL:
        SDL_GetMouseState(&mouse_pos.first, &mouse_pos.second);
        camera->move(0.f, 0.f, event.wheel.y * 2.f);
        break;
    case SDL_KEYDOWN:
        switch(event.key.keysym.sym) {
        case SDLK_UP:
            camera->move(0.f, -1.f, 0.f);
            break;
        case SDLK_DOWN:
            camera->move(0.f, 1.f, 0.f);
            break;
        case SDLK_LEFT:
            camera->move(-1.f, 0.f, 0.f);
            break;
        case SDLK_RIGHT:
            camera->move(1.f, 0.f, 0.f);
            break;
        }
        break;
    case SDL_JOYAXISMOTION: {
        const float sensivity = UnifiedRender::State::get_instance().joy_sensivity;
        float force = std::abs(event.jaxis.value - sensivity);
        if(force <= sensivity) {
            force = 0.f;
        }

        // First joystick is used as a "move around map"
        if(event.jaxis.which == 0) {
            if(event.jaxis.axis == 0) {
                // X
                camera->move(force, 0.f, 0.f);
            } else if(event.jaxis.axis == 1) {
                // Y
                camera->move(0.f, force, 0.f);
            }
        }
        // Second one is used to mvoe around UI (wip)
        else if(event.jaxis.which == 1) {
            if(event.jaxis.axis == 0) {
                mouse_pos.first += force;
            } else if(event.jaxis.axis == 1) {
                mouse_pos.second += force;
            }

            if(view_mode == MapView::SPHERE_VIEW) {
                if(input.middle_mouse_down) {  // Drag the map with middlemouse
                    float scale = glm::length(camera->position) / GLOBE_RADIUS;
                    float x_pos = input.last_camera_drag_pos.first - (mouse_pos.first - input.last_camera_mouse_pos.first) * 0.001 * scale;
                    float y_pos = input.last_camera_drag_pos.second - (mouse_pos.second - input.last_camera_mouse_pos.second) * 0.001 * scale;
                    camera->set_pos(x_pos, y_pos);
                }
                input.select_pos = camera->get_map_pos(input.mouse_pos);
                input.select_pos.first = (int)(world.width * input.select_pos.first / (2. * M_PI));
                input.select_pos.second = (int)(world.height * input.select_pos.second / M_PI);
            }
            else {
                if(input.middle_mouse_down) {  // Drag the map with middlemouse
                    std::pair<float, float> map_pos = camera->get_map_pos(mouse_pos);
                    float x_pos = camera->position.x + input.last_camera_drag_pos.first - map_pos.first;
                    float y_pos = camera->position.y + input.last_camera_drag_pos.second - map_pos.second;
                    camera->set_pos(x_pos, y_pos);
                }
                input.select_pos = camera->get_map_pos(input.mouse_pos);
                input.select_pos.first = (int)input.select_pos.first;
                input.select_pos.second = (int)input.select_pos.second;
            }
        }
    } break;
    case SDL_WINDOWEVENT:
        if(event.window.event == SDL_WINDOWEVENT_RESIZED) {
            int width, height;
            SDL_GetWindowSize(SDL_GetWindowFromID(event.window.windowID), &width, &height);
            camera->set_screen(width, height);
        }
        break;
    }
}

// Updates the province color texture with the changed provinces
void Map::update_mapmode() {
    std::vector<ProvinceColor> province_colors = mapmode_func(world);
    map_render->update_mapmode(province_colors);
}

void Map::draw(const GameState& gs) {
    glm::mat4 view, projection;

    map_render->draw(camera, view_mode);

    // TODO: We need to better this
    view = camera->get_view();
    projection = camera->get_projection();
    
    obj_shader->use();
    obj_shader->set_uniform("projection", projection);
    obj_shader->set_uniform("view", view);

    for(const auto& unit : world.units) {
        glm::mat4 model(1.f);
        std::pair<float, float> pos = unit->get_pos();
        model = glm::translate(model, glm::vec3(pos.first, pos.second, 0.f));
        //model = glm::rotate(model, 180.f, glm::vec3(1.f, 0.f, 0.f));
        obj_shader->set_uniform("model", model);
        draw_flag(unit->owner);
#if defined TILE_GRANULARITY
        model = glm::rotate(model, std::atan2(unit->tx - unit->x, unit->ty - unit->y), glm::vec3(0.f, 1.f, 0.f));
#endif
        obj_shader->set_uniform("model", model);
        unit_type_models[world.get_id(unit->type)]->draw(*obj_shader);
    }

    //(&UnifiedRender::State::get_instance().model_man->load(Path::get("3d/unit_types/" + unit_type->ref_name + ".obj")));
    for(const auto& unit_type : world.unit_types) {
        glm::mat4 model(1.f);
        model = glm::translate(model, glm::vec3(world.get_id(unit_type) * 8.f, -4.f, 0.f));
        model = glm::rotate(model, -90.f, glm::vec3(1.f, 0.f, 0.f));
        obj_shader->set_uniform("model", model);
        unit_type_models[world.get_id(unit_type)]->draw(*obj_shader);
    }

    for(const auto& building_type : world.building_types) {
        glm::mat4 model(1.f);
        model = glm::translate(model, glm::vec3(world.get_id(building_type) * 8.f, 0.f, 0.f));
        model = glm::rotate(model, -90.f, glm::vec3(1.f, 0.f, 0.f));
        obj_shader->set_uniform("model", model);
        building_type_models[world.get_id(building_type)]->draw(*obj_shader);
    }

    for(const auto& province : world.provinces) {
        const float size = 2.f;
        unsigned int i = 0;
        std::vector<Unit*> units = province->get_units();
        const float row_width = units.size() * size;
        for(const auto& unit : units) {
            std::pair<float, float> pos = province->get_pos();
            pos.first -= row_width / 2.f;
            pos.first += i * size;

            glm::mat4 model(1.f);
            model = glm::translate(model, glm::vec3(pos.first, pos.second, 0.f));
            obj_shader->set_uniform("model", model);

            UnifiedRender::Square plane = UnifiedRender::Square(0.f, 0.f, size, size);
            obj_shader->set_texture(0, "diffuse_map", *nation_flags[world.get_id(unit->owner)]);
            obj_shader->use();
            plane.draw();

            if(!unit->size) {
                continue;
            }

            /*if(unit->target != nullptr) {
                std::pair<float, float> pos = unit->get_pos();
                glBegin(GL_LINES);
                glColor3f(0.f, 0.f, 1.f / std::max(0.1f, unit->move_progress));
                glVertex2f(pos.first, pos.second);
                glVertex2f(unit->target->min_x + ((unit->target->max_x - unit->target->min_x) / 2.f), unit->target->min_y + ((unit->target->max_y - unit->target->min_y) / 2.f));
                glEnd();
            }

            for(const auto& selected : gs.input.selected_units) {
                if(selected == unit) {
                    std::pair<float, float> pos = unit->get_pos();
                    glPushMatrix();
                    glTranslatef(0.f, 0.f, -0.1f);
                    glLineWidth(8.f);
                    glBegin(GL_LINE_STRIP);
                    glColor3f(1.f, 1.f, 1.f);
                    glVertex2f(pos.first, pos.second);
                    glVertex2f(pos.first + 2.f, pos.second);
                    glVertex2f(pos.first + 2.f, pos.second + 2.f);
                    glVertex2f(pos.first, pos.second + 2.f);
                    glEnd();
                    glLineWidth(1.f);
                    glPopMatrix();
                    break;
                }
            }*/

            i++;
        }
    }

    // Draw the "drag area" box
    /*if(gs.input.is_drag) {
        glPushMatrix();
        glTranslatef(0.f, 0.f, -0.1f);
        glColor3f(1.f, 1.f, 1.f);
        glBegin(GL_LINE_STRIP);
        glVertex2f(gs.input.drag_coord.first, gs.input.drag_coord.second);
        glVertex2f(gs.input.select_pos.first, gs.input.drag_coord.second);
        glVertex2f(gs.input.select_pos.first, gs.input.select_pos.second);
        glVertex2f(gs.input.drag_coord.first, gs.input.select_pos.second);
        glVertex2f(gs.input.drag_coord.first, gs.input.drag_coord.second);
        glEnd();
        glPopMatrix();
    }*/

    wind_osc += 0.01f;
    if(wind_osc >= 180.f) {
        wind_osc = 0.f;
    }
}
