#pragma once

#include <vector>

#include <grev/execution_state.hpp>

namespace grev
{
    class execution_path : std::unordered_map<z3::expression, z3::expression const*>
    {
        const_iterator initial_jump_;

        z3::expression condition_;
        execution_state state_;
        iterator jump_;

    public:

        explicit execution_path(z3::expression initial_jump);
        ~execution_path();

        execution_path(execution_path const& other);
        execution_path(execution_path&& other) noexcept;

        execution_path& operator=(execution_path other) noexcept;

        z3::expression& condition();
        z3::expression const& condition() const;

        execution_state& state();
        execution_state const& state() const;

        void patch_jump(z3::expression value);

        void proceed(z3::expression jump);
        void proceed(execution_path update_path);

        std::optional<z3::expression> jump() const;

        // >>-----
        std::vector<std::uint32_t> addresses() const; // Testing seam TODO
        // -----<<
    };
}
