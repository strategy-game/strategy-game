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
//      policy.hpp
//
// Abstract:
//      Does some important stuff.
// ----------------------------------------------------------------------------

#pragma once

#include <cmath>
#include "unified_render/decimal.hpp"

enum AllowancePolicy {
    // Nobody can enter the country
    ALLOW_NOBODY,

    // Only accepted cultures can enter the country
    ALLOW_ACCEPTED_CULTURES,

    // Everyone can enter, but at a cost
    ALLOW_ALL_PAYMENT,

    // Everyone can exit freely
    ALLOW_ALL,
};

enum CensorshipPolicy {
    // All media censored
    CENSORSHIP_ALL_CENSORED,

    // Only state run media allowed
    CENSORSHIP_ONLY_STATE,

    // Only "approved" media is allowed
    CENSORSHIP_ONLY_APPROVED,

    // All media allowed
    CENSORSHIP_ALL_ALLOWED,
};

enum AutoBuildPolicy {
    // POPs cannot auto build stuff
    AUTO_BUILD_NONE,

    // POPs can only build with approval
    AUTO_BUILD_ONLY_APPROVED,

    // All POPs can build freely
    AUTO_BUILD_ALLOWED,
};

enum TreatmentPolicy {
    // Everyone is equal
    TREATMENT_EVERYONE_EQUAL,

    // Only accepted POPs are treated
    TREATMENT_ONLY_ACCEPTED,

    // Non-accepted are enslaved
    TREATMENT_ENSLAVED,

    // Exterminate
    TREATMENT_EXTERMINATE,
};

class MaterialPolicy {
    bool is_banned;
    float max_stockpile_saved;
};

class Policies {
public:
    TreatmentPolicy treatment;

    AllowancePolicy migration;
    AllowancePolicy immigration;
    CensorshipPolicy censorship;

    // Whetever POPs are able to build infrastructure
    AutoBuildPolicy build_infrastructure;

    // Whatever POPs are able to build factories
    AutoBuildPolicy build_factories;

    bool national_id;

    bool men_suffrage;
    bool men_labour;

    bool women_suffrage;
    bool women_labour;

    bool private_property;
    bool companies_allowed;

    bool public_education;
    bool secular_education;

    bool public_healthcare;

    // Goverment pays bought food
    bool social_security;

    // Is slavery allowed?
    bool slavery;

    // Is there a parliament that is needed to approve laws?
    bool legislative_parliament;

    // Is there a parliament that is needed to do executive actions?
    bool executive_parliament;

    // Is there a constitution to limit the power of the leader?
    bool constitutional;

    // Do we allow foreign trade?
    bool foreign_trade;

    // Tax % for importing products
    DECIAML_TYPE_3P import_tax;

    // Tax % for exporting products
    DECIAML_TYPE_3P export_tax;

    // Tax % for domestic imports
    DECIAML_TYPE_3P domestic_import_tax;

    // Tax % for domestic exports
    DECIAML_TYPE_3P domestic_export_tax;

    // Flat tax rhs on the low-wage pops
    DECIAML_TYPE_3P poor_flat_tax;

    // Flat tax rhs on the medium-wage pops
    DECIAML_TYPE_3P med_flat_tax;

    // Flat tax rhs on the high-wage pops
    DECIAML_TYPE_3P rich_flat_tax;

    // The tax given to the industrial sector
    DECIAML_TYPE_3P industry_tax;

    // Spending done on military (% of budget)
    DECIAML_TYPE_3P military_spending;

    // Units can grab anything they want from DOMESTIC provinces
    // Please notice the DOMESTIC part of this, for foreign "free grab"; check diplomatic relations!!
    bool free_supplies;

    // Minimum wage
    DECIAML_TYPE_3P min_wage;

    // Minimum social value for people to be accounted on the parliament
    DECIAML_TYPE_3P min_sv_for_parliament;

    inline int difference(const Policies& rhs) {
        int diff = 0;

        diff += std::abs(static_cast<int>(rhs.migration) - static_cast<int>(this->migration));
        diff += std::abs(static_cast<int>(rhs.immigration) - static_cast<int>(this->immigration));
        diff += std::abs(static_cast<int>(rhs.censorship) - static_cast<int>(this->censorship));
        diff += std::abs(static_cast<int>(rhs.build_factories) - static_cast<int>(this->build_factories));
        diff += std::abs(rhs.build_infrastructure - this->build_infrastructure);
        diff += (rhs.national_id != this->national_id) ? 1 : 0;
        diff += (rhs.men_suffrage != this->men_suffrage) ? 1 : 0;
        diff += (rhs.men_labour != this->men_labour) ? 1 : 0;
        diff += (rhs.women_suffrage != this->women_suffrage) ? 1 : 0;
        diff += (rhs.women_labour != this->women_labour) ? 1 : 0;
        diff += (rhs.private_property != this->private_property) ? 1 : 0;
        diff += (rhs.companies_allowed != this->companies_allowed) ? 1 : 0;
        diff += (rhs.public_education != this->public_education) ? 1 : 0;
        diff += (rhs.secular_education != this->secular_education) ? 1 : 0;
        diff += (rhs.public_healthcare != this->public_healthcare) ? 1 : 0;
        diff += (rhs.social_security != this->social_security) ? 1 : 0;
        diff += (rhs.slavery != this->slavery) ? 1 : 0;
        diff += (rhs.import_tax != this->import_tax) ? 1 : 0;
        diff += (rhs.export_tax != this->export_tax) ? 1 : 0;
        diff += (rhs.poor_flat_tax != this->poor_flat_tax) ? 1 : 0;
        diff += (rhs.med_flat_tax != this->med_flat_tax) ? 1 : 0;
        diff += (rhs.rich_flat_tax != this->rich_flat_tax) ? 1 : 0;
        diff += (rhs.free_supplies != this->free_supplies) ? 1 : 0;
        diff += std::abs(rhs.min_wage - this->min_wage);
        diff += std::abs(rhs.min_sv_for_parliament - this->min_sv_for_parliament);
        return diff;
    };
};