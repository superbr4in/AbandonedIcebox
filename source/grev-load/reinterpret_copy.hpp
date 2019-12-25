#pragma once

#include <span>

namespace grev
{
    template <typename T>
    void reinterpret_copy(T* destination, std::span<std::uint8_t const> const& source);
}

#ifndef LINT
#include "reinterpret_copy.tpp"
#endif
