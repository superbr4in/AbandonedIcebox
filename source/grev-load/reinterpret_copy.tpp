#ifdef LINT
#include "reinterpret_copy.hpp"
#endif

namespace grev
{
    template <typename T>
    void reinterpret_copy(T* const destination, std::span<std::uint8_t const> const& source)
    {
        *destination = *reinterpret_cast<T const*>(source.data());
    }
}
