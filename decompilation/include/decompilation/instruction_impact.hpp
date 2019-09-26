#pragma once

#include <unordered_map>

#include <z3++.h>

namespace std
{
    template<>
    struct hash<z3::expr>
    {
        std::size_t operator()(z3::expr const& expression) const;
    };
}

namespace dec
{
    using instruction_impact = std::unordered_map<z3::expr, z3::expr>;
}
