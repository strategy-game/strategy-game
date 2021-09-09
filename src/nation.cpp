#include "nation.hpp"
#include "world.hpp"

inline void Nation::do_diplomacy() {
    // TODO: Fix this formula which is currently broken
    //diplomatic_timer = std::max((60 * 48) - std::min(10.f * 48.f, prestige / 100.f), 4.f);
    diplomatic_timer = 48;
}

inline bool Nation::can_do_diplomacy() {
    return diplomatic_timer == 0;
}

#include "print.hpp"
void Nation::increase_relation(const World& world, Nation* target) {
    if(!can_do_diplomacy())
        return;
    
    const Nation::Id t1_idx = world.get_id(target);
    this->relations[t1_idx].relation += 5.f;
    const Nation::Id t2_idx = world.get_id(this);
    target->relations[t2_idx].relation += 5.f;

    print_info("%s increases relations with %s", name.c_str(), target->name.c_str());
    do_diplomacy();
}

void Nation::decrease_relation(const World& world, Nation* target) {
    if(!can_do_diplomacy())
        return;

    const Nation::Id t1_idx = world.get_id(target);
    this->relations[t1_idx].relation += 5.f;
    const Nation::Id t2_idx = world.get_id(this);
    target->relations[t2_idx].relation += 5.f;

    print_info("%s decreases relations with %s", name.c_str(), target->name.c_str());
    do_diplomacy();
}

/**
* Automatically relocates the capital of a nation to another province
* Use this when a treaty makes a nation lose it's capital
 */
void Nation::auto_relocate_capital(void) {
    auto best_candidate = std::max_element(owned_provinces.begin(), owned_provinces.end(),
        [] (const auto* lhs, const auto* rhs) {
            return (lhs->total_pops() < rhs->total_pops());
        });
    capital = *best_candidate;
}

/**
* Enacts a policy on a nation
* @return false if policy draft failed to be applied, true if policy passed and is in-effect
 */
void Nation::set_policy(Policies& policies) {
    // TODO: Make parliament (aristocrat POPs) be able to reject policy changes
    // TODO: Increase militancy on non-agreeing POPs
    
    memcpy(&this->current_policy, &policies, sizeof(Policies));
    return;
}

/**
 * Checks if a POP is part of one of our accepted cultures
*/
bool Nation::is_accepted_culture(const Pop& pop) const {
    for(const auto& culture: accepted_cultures) {
        if(pop.culture_id == g_world->get_id(culture)) {
            return true;
        }
    }
    return false;
}

/**
 * Gets the total tax applied to a POP depending on their "wealth"
 * (not exactly like that, more like by their type/status)
*/
float Nation::get_tax(const Pop& pop) const {
    float base_tax = 1.f;

    // Advanced discrimination mechanics ;)

    // Only-accepted POPs policy should impose higher prices to non-accepted POPs
    // We are not an accepted-culture POP
    if(is_accepted_culture(pop) != true) {
        if(current_policy.treatment == TREATMENT_ONLY_ACCEPTED) {
            base_tax += 1.5f;
        } else if(current_policy.treatment == TREATMENT_EXTERMINATE) {
            // Fuck you
            base_tax += 1000.f;
        }
    }

    if(pop.type_id == POP_TYPE_FARMER
    || pop.type_id == POP_TYPE_SOLDIER
    || pop.type_id == POP_TYPE_LABORER
    || pop.type_id == POP_TYPE_SLAVE) {
        return current_policy.poor_flat_tax * base_tax;
    }
    // For the medium class
    else if(pop.type_id == POP_TYPE_ARTISAN
    || pop.type_id == POP_TYPE_BUREAUCRAT
    || pop.type_id == POP_TYPE_CLERGYMEN
    || pop.type_id == POP_TYPE_OFFICER) {
        return current_policy.med_flat_tax * base_tax;
    }
    // For the high class
    else if(pop.type_id == POP_TYPE_ENTRPRENEUR
    || pop.type_id == POP_TYPE_ARISTOCRAT) {
        return current_policy.rich_flat_tax * base_tax;
    }
    return base_tax;
}