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
//      binary_image.hpp
//
// Abstract:
//      Does some important stuff.
// ----------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <cstddef>
#include <exception>
#include <memory>
#include <string>
#include <variant>

#include "eng3d/io.hpp"
#include "eng3d/color.hpp"

namespace Eng3D {
    class BinaryImageException: public std::exception {
        std::string buffer;
    public:
        BinaryImageException(const std::string& filename, const std::string& message) {
            buffer = "" + filename + ": " + message;
        }

        virtual const char* what() const noexcept {
            return buffer.c_str();
        }
    };

    /// @brief This binary image class helps load images and visual resources from the disk; the
    /// binary image IS NOT a texture, it is intended to be used in contextes which are
    /// NOT rendering - for rendering purpouses see texture class from client's implementation
    struct BinaryImage {
        BinaryImage() = default;
        BinaryImage(const Eng3D::IO::Path& path);
        BinaryImage(size_t _width, size_t _height, size_t bpp = 32);
        BinaryImage(const BinaryImage& tex);
        BinaryImage& operator=(const BinaryImage&) = delete;
        virtual ~BinaryImage() = default;
        virtual void from_file(const Eng3D::IO::Path& path);
        virtual void to_file(const std::string& filename);

        /// @brief Obtains a pixel from the binary image
        /// @param x X coordinate
        /// @param y Y coordinate
        /// @return uint32_t The colour
        inline Eng3D::Color get_pixel(size_t x, size_t y) const {
            if(x >= width || y >= height)
                return Eng3D::Color::argb32(0xffffffff);
            return Eng3D::Color::argb32(buffer[x + y * width]);
        }
        
        std::unique_ptr<uint32_t[], decltype(&std::free)> buffer = std::unique_ptr<uint32_t[], decltype(&std::free)>(nullptr, &std::free);
        size_t width = 0;
        size_t height = 0;
        size_t bpp = 0;
    };
}
