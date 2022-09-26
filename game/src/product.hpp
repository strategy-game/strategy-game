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
#include "objects.hpp"

/// @brief A good, mostly serves as a "product type"
class Good : public RefnameEntity<GoodId> {
public:
    Eng3D::StringRef name;
};
template<>
class Serializer<Good*>: public SerializerReferenceLocal<World, Good> {};
template<>
class Serializer<const Good*>: public SerializerReferenceLocal<World, const Good> {};
template<>
class Serializer<Good> {
public:
    template<bool is_serialize>
    static inline void deser_dynamic(Archive& ar, Good& obj) {
        ::deser_dynamic<is_serialize>(ar, obj.cached_id);
        ::deser_dynamic<is_serialize>(ar, obj.name);
        ::deser_dynamic<is_serialize>(ar, obj.ref_name);
    }
};

/// @brief A product (based off a Good) which can be bought by POPs, converted by factories and transported
class Product : public Entity<ProductId> {
public:
    inline void close_market() {
        if(this->demand > this->supply) {
            // Increase price with more demand
            this->price_vel += 0.001f * (this->demand - this->supply);
        } else if(this->demand < this->supply) {
            // Increase supply with more demand
            this->price_vel -= 0.001f * (this->supply - this->demand);
        } else {
            // Gravitate towards absolute zero due to volatility decay
            // (i.e, product price becomes stable without market activity)
            if(this->price_vel > 0.1f) this->price_vel -= 0.01f;
            else if(this->price_vel < -0.1f) this->price_vel += 0.01f;
            else this->price_vel = -0.01f;
        }

        // Set the new price
        this->price = glm::clamp(this->price + this->price_vel, 0.01f, 100'000.f);
        //this->demand = 0;
    }

    float price = 1.f; // Price of the product
    float price_vel = 0.f; // Velocity of change of price of the product
    float supply = 0.f; // Total supply of the product
    float demand = 0.f; // Total demand of the product
};
template<>
class Serializer<Product> {
public:
    template<bool is_serialize>
    static inline void deser_dynamic(Archive& ar, Product& obj) {
        ::deser_dynamic<is_serialize>(ar, obj.cached_id);
        ::deser_dynamic<is_serialize>(ar, obj.price);
        ::deser_dynamic<is_serialize>(ar, obj.price_vel);
        ::deser_dynamic<is_serialize>(ar, obj.supply);
        ::deser_dynamic<is_serialize>(ar, obj.demand);
    }
};

