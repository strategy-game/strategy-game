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
//      sound.cpp
//
// Abstract:
//      Does some important stuff.
// ----------------------------------------------------------------------------

#include <cstring>
#include <algorithm>

#include "SDL.h"
#include "SDL_audio.h"
#include "SDL_mixer.h"

#include "eng3d/audio.hpp"
#include "eng3d/utils.hpp"
#include "eng3d/log.hpp"
#include "eng3d/state.hpp"

#define STB_VORBIS_HEADER_ONLY 1
#include "stb_vorbis.c"

//
// Audio
//
Eng3D::Audio::Audio(const std::string_view path, bool is_sound) {
    Eng3D::Log::debug("audio", Eng3D::translate_format("Decoding audio %s", path.data()));
    this->is_sound = is_sound;
    if(this->is_sound) this->stream = (void*)Mix_LoadWAV(path.data());
    else this->stream = (void*)Mix_LoadMUS(path.data());
}

Eng3D::Audio::~Audio() {
    if(this->stream != nullptr) {
        if(this->is_sound) Mix_FreeChunk((Mix_Chunk *)this->stream);
        else Mix_FreeMusic((Mix_Music *)this->stream);
    }
}

//
// Audio manager
//
Eng3D::AudioManager::AudioManager(Eng3D::State& _s)
    : s{ _s }
{
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0)
        CXX_THROW(std::runtime_error, Eng3D::translate_format("Unable to open audio: %s", Mix_GetError()));
    Mix_Init(MIX_INIT_OGG);
}

void Eng3D::AudioManager::play_sound(const std::string_view path) {
    auto audio = this->load(path.data(), true);
    if((audio->channel = Mix_PlayChannel(-1, (Mix_Chunk *)audio->stream, 0)) < 0)
        Eng3D::Log::warning("audio", Eng3D::translate_format("Unable to load audio %s: %s", path.data(), Mix_GetError()));
    this->current_sound = audio;
}

bool Eng3D::AudioManager::can_play_sound() {
    if(this->current_sound.get() == nullptr || this->current_sound->channel < 0)
        return true;
    bool is_busy = Mix_Playing(this->current_sound->channel) != 0;
    return !is_busy;
}

void Eng3D::AudioManager::play_music(const std::string_view path) {
    auto audio = this->load(path.data(), false);
    if(Mix_PlayMusic((Mix_Music *)audio->stream, 0) < 0)
        Eng3D::Log::warning("audio", Eng3D::translate_format("Unable to load audio %s: %s", path.data(), Mix_GetError()));
    this->current_music = audio;
}

bool Eng3D::AudioManager::can_play_music() {
    if(this->current_music.get() == nullptr)
        return true;
    bool is_busy = Mix_PlayingMusic() != 0;
    return !is_busy;
}

Eng3D::AudioManager::~AudioManager() {
    Mix_CloseAudio();
}

const std::shared_ptr<Eng3D::Audio> Eng3D::AudioManager::load(const std::string_view path, bool is_sound) {
    const auto key = std::hash<std::string_view>{}(path);
    // Find Sound when wanting to be loaded
    auto it = audios.find(key);
    if(it != audios.cend()) return (*it).second;
    // Otherwise Sound is not in our control, so we create a new one
    audios[key] = std::make_shared<Eng3D::Audio>(path, is_sound);
    Eng3D::Log::debug("audio", Eng3D::translate_format("Loaded and cached sound %s", path.data()));
    return audios[key];
}
