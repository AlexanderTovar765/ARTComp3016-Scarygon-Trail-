#include "Effects.h"

namespace {
    void append(std::string& out, const std::string& name, int value) {
        if (value == 0) return;
        if (!out.empty()) out += ", ";
        out += name;
        out += ' ';
        if (value > 0) out += '+';
        out += std::to_string(value);
    }
}
// This part of the code turns the choice effects into short text.
std::string Effects::describe() const {
    std::string out;
    append(out, "Food", food);
    append(out, member.empty() ? std::string("Health") : member + "'s health", health);
    append(out, "Sanity", sanity);
    append(out, "Wagon", wagon);
    return out.empty() ? "no change" : out;
}
