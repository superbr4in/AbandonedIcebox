#include <climits>

#ifdef LINT
#include <grev/machine_process.hpp>
#endif

namespace grev
{
    template <typename Disassembler>
    execution machine_process::execute(Disassembler const& disassembler) const
    {
        execution execution;

        auto& initial_path =
            execution.paths.emplace_front(z3::expression(sizeof(std::uint32_t) * CHAR_BIT, program_.entry_point_address()));

        std::forward_list<execution_path*> pending_paths;
        pending_paths.push_front(&initial_path);
        do
        {
            auto* const path = pending_paths.front();
            pending_paths.pop_front();

            // Follow the current path as far as possible (depth-first search)
            for (auto [address, code] = std::pair<std::optional<std::uint32_t>, std::span<std::uint8_t const>> { };;)
            {
                // Proceed if jump is available
                if (auto jump = path->jump())
                {
                    // Workaround for jump = [REG] (TODO)
                    if (auto jump_reference = jump->reference())
                    {
                        path->state().resolve(&*jump_reference);
                        jump = jump_reference->dereference(jump->width());
                    }

                    auto const prev_jump = *jump;
                    environment_.patch(program_, jump->dependencies()).resolve(&*jump);
                    if (*jump != prev_jump)
                        path->patch_jump(*jump);

                    // Proceed if unambiguous
                    if (auto next_address = jump->evaluate())
                    {
                        // (Re-)extract code if the next address is the first one or not adjacent to the previous address
                        if (address != *next_address)
                        {
                            address = std::move(*next_address);
                            code = program_[*address];
                        }
                    }
                    else break;
                }
                else break;

                std::forward_list<execution_path> update_paths;
                if (code.empty())
                {
                    if (auto import = program_.load_imported(*address))
                    {
                        auto const import_execution = machine_process{std::move(*import), environment_}.execute(disassembler);
                        update_paths = std::move(import_execution.paths);
                    }
                    else break; // TODO Do not rely on missing code for import call detection.

                    execution_state call_state;
                    for (auto const& import_path : update_paths)
                    {
                        auto const& import_state = import_path.state();

                        // TODO Use conditions
                        for (auto const& import_path_dependency : import_state.dependencies())
                        {
                            auto ipd_copy = import_path_dependency;
                            if (auto import_path_dependency_reference = import_path_dependency.reference())
                            {
                                path->state().resolve(&*import_path_dependency_reference);
                                ipd_copy = import_path_dependency_reference->dereference(import_path_dependency.width());
                            }

                            call_state.define(std::move(ipd_copy), path->state()[import_path_dependency]);
                        }
                    }

                    execution.import_calls.emplace_back(*address, std::move(call_state));
                }
                else
                {
                    // Disassemble next code
                    update_paths = disassembler(&*address, &code);
                }

                std::forward_list<execution_path> resolved_update_paths;
                for (auto& update_path : update_paths)
                {
                    if (auto const update_jump = update_path.jump())
                    {
                        auto resolved_update_jump = *update_jump;
                        path->state().resolve(&resolved_update_jump);
                        if (resolved_update_jump != update_jump)
                            update_path.patch_jump(std::move(resolved_update_jump));
                    }
                    else continue; // Prohibit loops (TODO)

                    path->state().resolve(&update_path.state());

                    environment_.patch(program_, update_path.state().dependencies()).resolve(&update_path.state());

                    path->state().resolve(&update_path.condition());

                    if (update_path.condition() == z3::expression::boolean_false())
                        continue;

                    resolved_update_paths.push_front(std::move(update_path));
                }

                if (resolved_update_paths.empty())
                    break;

                auto update_path = resolved_update_paths.begin();
                for (; std::next(update_path) != resolved_update_paths.end(); ++update_path)
                {
                    auto& next_path = execution.paths.emplace_front(*path);
                    pending_paths.push_front(&next_path);

                    next_path.condition() &= update_path->condition();
                    next_path.state() += update_path->state();
                    next_path.proceed(std::move(*update_path));
                }

                path->condition() &= update_path->condition();
                path->state() += update_path->state();
                path->proceed(std::move(*update_path));
            }
        }
        while (!pending_paths.empty());

        return execution;
    }
}

#ifdef LINT
#include <grev-lift/reil_disassembler.hpp>
template grev::execution grev::machine_process::execute(grev::reil_disassembler const&) const;
#endif
