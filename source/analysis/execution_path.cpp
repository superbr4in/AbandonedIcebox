#include <generic-reveng/analysis/execution_path.hpp>

namespace grev
{
    execution_path::execution_path(std::uint32_t const start_address) :
        start_jump_(emplace(start_address, nullptr).first),
        current_jump_(begin()) { }
    execution_path::~execution_path() = default;

    execution_path::execution_path(execution_path const& other) :
        std::unordered_map<z3::expression, z3::expression const*>(other),
        start_jump_(find(other.start_jump_->first)),
        current_state_(other.current_state_),
        current_jump_(find(other.current_jump_->first))
    {
        for (auto& [jump, succeeding_jump] : *this)
        {
            if (succeeding_jump == nullptr)
                continue;

            succeeding_jump = &find(*succeeding_jump)->first;
        }
    }
    execution_path::execution_path(execution_path&& other) noexcept = default;

    execution_path& execution_path::operator=(execution_path other) noexcept
    {
        swap(other);

        std::swap(start_jump_, other.start_jump_);

        std::swap(current_state_, other.current_state_);
        std::swap(current_jump_, other.current_jump_);

        return *this;
    }

    std::forward_list<execution_path> execution_path::proceed(execution_update update, execution_state const& memory_patch_state)
    {
        // TODO Support patching (?)

        current_state_.resolve(&update.jumps);
        current_state_ += std::move(update.state);

        // TODO Resolve jumps (?)
        memory_patch_state.resolve(&current_state_);

        if (update.jumps.empty())
        {
            current_jump_ = end();
            return { };
        }

        auto current_jump = update.jumps.begin();

        std::forward_list<execution_path> new_paths;
        while (std::next(current_jump) != update.jumps.end())
        {
            auto& new_path = new_paths.emplace_front(*this);
            new_path.step(std::move(update.jumps.extract(current_jump++).value()));
        }

        step(std::move(update.jumps.extract(current_jump).value()));
        return new_paths;
    }

    std::optional<std::uint32_t> execution_path::next_address() const
    {
        if (current_jump_ == end() || current_jump_->second != nullptr) // TODO Support loops with changing states
            return std::nullopt;

        return current_jump_->first.evaluate();
    }

    // >>-----
    std::vector<std::uint32_t> execution_path::addresses() const
    {
        std::vector<std::uint32_t> addresses;
        for (auto jump = start_jump_;; jump = find(*jump->second))
        {
            if (auto const address = jump->first.evaluate())
                addresses.push_back(*address);
            else
                break;

            if (jump == current_jump_ || jump->second == nullptr)
                break;
        }

        return addresses;
    }
    // -----<<

    void execution_path::step(z3::expression jump)
    {
        auto new_jump = try_emplace(std::move(jump)).first;

        current_jump_->second = &new_jump->first;
        current_jump_ = std::move(new_jump);
    }
}

static_assert(std::is_destructible_v<grev::execution_path>);

static_assert(std::is_copy_constructible_v<grev::execution_path>);
static_assert(std::is_nothrow_move_constructible_v<grev::execution_path>);

static_assert(std::is_copy_assignable_v<grev::execution_path>);
static_assert(std::is_nothrow_move_assignable_v<grev::execution_path>);
