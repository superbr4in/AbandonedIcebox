#include "sort.hpp"

namespace grev
{
    sort::sort(std::size_t const size) :
        ast(Z3_mk_bv_sort(context(), size)) { }
}

static_assert(std::is_destructible_v<grev::sort>);

static_assert(std::is_copy_constructible_v<grev::sort>);
static_assert(std::is_copy_assignable_v<grev::sort>);

static_assert(std::is_move_constructible_v<grev::sort>);
static_assert(std::is_move_assignable_v<grev::sort>);
