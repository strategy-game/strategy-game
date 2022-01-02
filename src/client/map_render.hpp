#pragma once

#include <cstddef>

namespace UnifiedRender {
    class Texture;
    class Model;
    class TextureArray;
    class Model;
    namespace OpenGl {
        class Square;
        class Sphere;
        class Program;
        class Quad2D;
        class Framebuffer;
    }
}

#include "unified_render/color.hpp"
#include "unified_render/shader.hpp"

#include <vector>
#include <memory>
#include <utility>
#include <functional>

class World;
class Camera;
class Nation;
class GameState;
enum class MapView;
union SDL_Event;
struct Input;
struct ProvinceColors;

typedef UnifiedRender::OpenGl::Option Option;
struct MapOptions {
    Option noise{"NOISE", false};
    Option sdf{"SDF", false};
    Option lighting{"LIGHTING", false};
    Option diag_borders{"DIAG_BORDER", false};
    Option parallax{"PARALLAX", false};
    Option landscape{"LANDSCAPE", false};
    Option rivers{"RIVERS", false};

    std::vector<Option> get_options() {
        return std::vector<Option>{noise, sdf};
    }
};
class MapRender {
    std::unique_ptr<UnifiedRender::Texture> gen_border_sdf();
public:
    MapRender(const World& world);
    ~MapRender() {};
    void update_mapmode(std::vector<ProvinceColor> province_colors);
    void draw(Camera* camera, MapView view_mode);
    void reload_shaders();

private:
    const World& world;
    
    MapOptions options;
    // Map textures
    UnifiedRender::Texture* tile_map;
    UnifiedRender::Texture* tile_sheet;
    const UnifiedRender::Texture* water_tex;
    const UnifiedRender::Texture* noise_tex;
    const UnifiedRender::Texture* topo_map;
    const UnifiedRender::Texture* river_tex;
    UnifiedRender::Texture* terrain_map;
    const UnifiedRender::Texture* landscape_map;
    const UnifiedRender::Texture* wave1;
    const UnifiedRender::Texture* wave2;
    UnifiedRender::Texture* normal_topo;
    UnifiedRender::TextureArray* terrain_sheet;

    UnifiedRender::OpenGl::Square* map_quad;
    UnifiedRender::OpenGl::Sphere* map_sphere;
    UnifiedRender::OpenGl::Quad2D* map_2d_quad;

    std::unique_ptr<UnifiedRender::OpenGl::Program> map_shader;
    std::unique_ptr<UnifiedRender::OpenGl::Program> border_sdf_shader;
    std::unique_ptr<UnifiedRender::OpenGl::Program> border_gen_shader;
    std::unique_ptr<UnifiedRender::OpenGl::Program> output_shader;
    std::unique_ptr<UnifiedRender::Texture> border_sdf;
};