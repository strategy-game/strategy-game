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
//      string.cpp
//
// Abstract:
//      Does some important stuff.
// ----------------------------------------------------------------------------

#include <unordered_map>
#include <cstring>
#include <mutex>
#include <memory>
#include "eng3d/string.hpp"

//
// StringRef
//
Eng3D::StringRef::StringRef(const std::string_view str) {
    *this = Eng3D::StringManager::get_instance().insert(std::string{str});
}

const std::string_view Eng3D::StringRef::get_string() const {
    if(this->id == static_cast<size_t>(-1)) return "";
    return Eng3D::StringManager::get_instance().get_by_id(this->id);
}

//
// StringManager
//
static Eng3D::StringManager *g_string_man = nullptr;
Eng3D::StringManager::StringManager(Eng3D::State& _s)
    : s{ _s }
{
    g_string_man = this;
}

Eng3D::StringManager& Eng3D::StringManager::get_instance() {
    return *g_string_man;
}

static std::unordered_map<std::string, std::string> trans_msg;
static std::mutex trans_lock;
void Eng3D::Locale::from_file(const std::string& filename) {
    std::unique_ptr<FILE, decltype(&fclose)> fp(fopen(filename.c_str(), "rt"), fclose);
    char tmp[100];
    while(fgets(tmp, sizeof tmp, fp.get()) != nullptr) {
        if(!strncmp(tmp, "msgid", 5)) {
            char msgid[100];
            sscanf(tmp + 5, " %*c%[^\"]s%*c ", msgid);
            fgets(tmp, sizeof tmp, fp.get());
            if(!strncmp(tmp, "msgstr", 6)) {
                char msgstr[100];
                sscanf(tmp + 6, " %*c%[^\"]s%*c ", msgstr);
                trans_msg[msgid] = msgstr;
            }
        }
    }
}

std::string Eng3D::Locale::translate(const std::string_view str) {
    std::scoped_lock lock(trans_lock);
    if(trans_msg[str.data()].empty())
        return std::string(str);
    return trans_msg[str.data()];
}

std::string Eng3D::Locale::format_number(double num) {
    if (std::abs(num) < 1000) {
        return std::to_string(num);
    }
    static const std::string numbers[] = {"k",  "M",  "B",  "T",  "Qa", "Qn",
                                        "Sx", "Sp", "O",  "N",  "De", "Ud",
                                        "Dd", "Td", "Qd", "Qi", "Sd"};
    int exponent = static_cast<int>(log10(abs(num)) / 3);

    // Now get the number
    double d = static_cast<double>(num) / pow(10, exponent * 3);

    // Round this to two decimal points
    const int precision = 100;
    d = round(d * precision) / precision;

    return string_format("%.2f %s", d, numbers[exponent - 1].c_str());
}
