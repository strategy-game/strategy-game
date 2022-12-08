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
//      product.hpp
//
// Abstract:
//      Does some important stuff.
// ----------------------------------------------------------------------------

#pragma once

#include <string>
#include <deque>
#include <limits>
#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>
#include "objects.hpp"

/// @brief A commodity, mostly serves as a "product type"
struct Commodity : RefnameEntity<CommodityId> {
    Eng3D::StringRef name;
    std::string get_icon_path() const {
        return string_format("gfx/commodity/%s.png", ref_name.c_str());
    }
};
template<>
struct Eng3D::Deser::Serializer<Commodity> {
    template<bool is_const>
    using type = Eng3D::Deser::CondConstType<is_const, Commodity>::type;
    template<bool is_serialize>
    static inline void deser_dynamic(Eng3D::Deser::Archive& ar, type<is_serialize>& obj) {
        Eng3D::Deser::deser_dynamic<is_serialize>(ar, obj.cached_id);
        Eng3D::Deser::deser_dynamic<is_serialize>(ar, obj.name);
        Eng3D::Deser::deser_dynamic<is_serialize>(ar, obj.ref_name);
    }
};

/// @brief A product (based off a Commodity) which can be bought by POPs, converted by factories and transported
struct Product : Entity<ProductId> {
    void close_market() {
        this->demand = 0.f;
    }

    /// @brief Buy a portion of the item
    /// @param amount Amount to buy
    /// @return float Total cost of purchase
    float buy(float amount) {
        this->demand += amount;
        this->supply -= glm::min(amount, this->supply);
        return this->price * amount;
    }

    /// @brief Produce the product
    /// @param amount Amount to produce
    /// @return float Total gains from production
    float produce(float amount) {
        this->supply += amount;
        return this->price * amount;
    }

    float price = 1.f;
    float supply = 1.f;
    float demand = 1.f;
    float global_demand = 1.f;
    float speculative_demand = 0.f;
};
template<>
struct Eng3D::Deser::Serializer<Product> {
    template<bool is_const>
    using type = Eng3D::Deser::CondConstType<is_const, Product>::type;
    template<bool is_serialize>
    static inline void deser_dynamic(Eng3D::Deser::Archive& ar, type<is_serialize>& obj) {
        Eng3D::Deser::deser_dynamic<is_serialize>(ar, obj.cached_id);
        Eng3D::Deser::deser_dynamic<is_serialize>(ar, obj.price);
        Eng3D::Deser::deser_dynamic<is_serialize>(ar, obj.supply);
        Eng3D::Deser::deser_dynamic<is_serialize>(ar, obj.demand);
        Eng3D::Deser::deser_dynamic<is_serialize>(ar, obj.global_demand);
        Eng3D::Deser::deser_dynamic<is_serialize>(ar, obj.speculative_demand);
    }
};

