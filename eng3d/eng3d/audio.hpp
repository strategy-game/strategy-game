// Eng3D - General purpouse game engine
// Copyright (C) 2021, Eng3D contributors
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
//      audio.hpp
//
// Abstract:
//      Does some important stuff.
// ----------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <exception>
#include <mutex>
#include <vector>
#include <memory>

struct pa_simple;
namespace Eng3D {
    class State;

    class AudioException : public std::exception {
        std::string buffer;
    public:
        AudioException(const std::string& filename, const std::string& message) {
            buffer = filename + message;
        }

        virtual const char* what() const noexcept {
            return buffer.c_str();
        }
    };

    struct Audio {
        Audio() = default;
        Audio(const std::string& path);
        ~Audio();
        void *stream = nullptr; // TODO: Use RAII pointers for this
    };

    class AudioManager {
        static void mixaudio(void* userdata, uint8_t* stream, int len);
        std::map<std::string, std::shared_ptr<Eng3D::Audio>> sounds;
        Eng3D::State& s;
        int audio_dev_id = 0;
    public:
        AudioManager() = delete;
        AudioManager(Eng3D::State& s);
        ~AudioManager();
        const std::shared_ptr<Audio> load(const std::string& path);

        // Queue of sounds/music
        std::mutex sound_lock;
        std::vector<std::shared_ptr<Eng3D::Audio>> sound_queue;
        std::vector<std::shared_ptr<Eng3D::Audio>> music_queue;
        float music_fade_value = 1.f;
        float music_volume = 0.5f, sound_volume = 0.5f;
    };
}
