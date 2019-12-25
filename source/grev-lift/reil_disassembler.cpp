#include <climits>
#include <functional>
#include <string>

#include <libopenreil.h>

#include <grev-lift/reil_disassembler.hpp>

namespace grev
{
    static std::size_t get_width(reil_arg_t const& argument)
    {
        if (argument.type == A_LOC)
            return sizeof(std::uint32_t) * CHAR_BIT; // TODO Depending on machine architecture

        switch (argument.size)
        {
        case U1:
            return 1;
        case U8:
            return sizeof(std::uint8_t) * CHAR_BIT;
        case U16:
            return sizeof(std::uint16_t) * CHAR_BIT;
        case U32:
            return sizeof(std::uint32_t) * CHAR_BIT;
        case U64:
            return sizeof(std::uint64_t) * CHAR_BIT;
        default:
            throw std::logic_error("Unexpected size");
        }
    }

    static z3::expression get_key(reil_arg_t const& argument)
    {
        switch (argument.type)
        {
        case A_REG:
        case A_TEMP:
            return z3::expression(get_width(argument), std::string{std::begin(argument.name)});
        default:
            throw std::logic_error("Unexpected type");
        }
    }

    static std::function<z3::expression (z3::expression)>
        get_unary_operation(reil_op_t const& operation)
    {
        switch (operation)
        {
        case I_NEG:
            return std::mem_fn(&z3::expression::operator-);
        case I_NOT:
            return std::mem_fn(&z3::expression::operator~);
        default:
            throw std::logic_error("Unexpected operation");
        }
    }
    static std::function<z3::expression (z3::expression, z3::expression)>
        get_binary_operation(reil_op_t const& operation)
    {
        switch (operation)
        {
        case I_ADD:
            return z3::operator+;
        case I_SUB:
            return z3::operator-;
        case I_MUL:
            return z3::operator*;
        case I_DIV:
            return z3::operator/;
        case I_MOD:
            return z3::operator%;
        case I_SHL:
            return z3::operator<<;
        case I_SHR:
            return z3::operator>>;
        case I_AND:
            return z3::operator&;
        case I_OR:
            return z3::operator|;
        case I_XOR:
            return z3::operator^;
        case I_EQ:
            return std::mem_fn(&z3::expression::equals);
        case I_LT:
            return std::mem_fn(&z3::expression::less_than);
        default:
            throw std::logic_error("Unexpected operation");
        }
    }

    reil_disassembler::reil_disassembler(machine_architecture architecture) :
        architecture_(std::move(architecture))
    {
        reil_arch_t converted_architecture;
        switch (architecture_)
        {
            case machine_architecture::x86_32:
                converted_architecture = ARCH_X86;
                break;
            default:
                throw std::runtime_error("Unexpected architecture");
        }
        handle_ = reil_init(
            converted_architecture,
            [](auto* const instruction, auto* const instructions)
            {
                static_cast<std::list<reil_inst_t>*>(instructions)->push_back(*instruction);
                return 0;
            },
            &instructions_);
    }
    reil_disassembler::~reil_disassembler()
    {
        reil_close(handle_);
    }

    reil_disassembler::reil_disassembler(reil_disassembler const& other) :
        reil_disassembler(other.architecture_) { }
    reil_disassembler::reil_disassembler(reil_disassembler&& other) noexcept :
        architecture_(std::move(other.architecture_)),
        handle_(std::exchange(other.handle_, nullptr)) { }

    reil_disassembler& reil_disassembler::operator=(reil_disassembler other) noexcept
    {
        std::swap(architecture_, other.architecture_);
        std::swap(handle_, other.handle_);

        return *this;
    }

    std::forward_list<execution_path>
        reil_disassembler::operator()(std::uint32_t* const address, std::span<std::uint8_t const>* const code) const
    {
        update_paths_.emplace_front(z3::expression(sizeof(std::uint32_t) * CHAR_BIT, *address));

        std::vector<unsigned char> narrowed_code(
            code->begin(),
            std::next(
                code->begin(),
                std::min(code->size(), long{MAX_INST_LEN})));

        instructions_.clear();
        reil_translate_insn(handle_, *address, narrowed_code.data(), narrowed_code.size());

        auto const size = instructions_.front().raw_info.size;

        *address += size;
        *code = code->subspan(size);

        auto step_value = *address;
        for (auto const& instruction : instructions_)
        {
            auto& state = path().state();
            switch (instruction.op)
            {
            case I_NONE:
                break;
            case I_UNK:
                step_value = *address - size + 1;
                break;
            case I_JCC:
                if (instruction.c.inum == 0) // Prohibit instruction-internal jumps (TODO)
                    jump(instruction.a, get_value(instruction.c));
                break;
            case I_STR:
                set_value(instruction.c, get_value(instruction.a));
                break;
            case I_STM:
                state.define(
                    get_value(instruction.c).dereference(get_width(instruction.a)),
                    get_value(instruction.a));
                break;
            case I_LDM:
                set_value(instruction.c, state[get_value(instruction.a)].dereference(get_width(instruction.c)));
                break;
            case I_OR:
                if (instruction.a.size != instruction.c.size)
                {
                    set_value(
                        instruction.c,
                        (get_value(instruction.a) | get_value(instruction.b)).resize(get_width(instruction.c)));
                    break;
                }
            default:
                if (instruction.b.type == A_NONE)
                {
                    auto const unary_operation = get_unary_operation(instruction.op);
                    set_value(instruction.c, unary_operation(get_value(instruction.a)));
                }
                else
                {
                    auto const binary_operation = get_binary_operation(instruction.op);
                    set_value(instruction.c, binary_operation(get_value(instruction.a), get_value(instruction.b)));
                }
                break;
            }

            if (path().condition() == z3::expression::boolean_false())
                break;
        }

        if (path().condition() == z3::expression::boolean_false())
            update_paths_.pop_front();
        else
            path().proceed(z3::expression(sizeof step_value * CHAR_BIT, step_value));

        temporary_state_.clear();
        return std::move(update_paths_);
    }

    execution_path& reil_disassembler::path() const
    {
        return update_paths_.front();
    }

    void reil_disassembler::jump(_reil_arg_t const& argument, z3::expression value) const
    {
        auto const jump_condition = argument.type == A_CONST && argument.val != 0
            ? z3::expression::boolean_true()
            : get_value(argument);

        auto& forked_path = path();

        update_paths_.push_front(path());

        forked_path.condition() &= jump_condition;
        forked_path.proceed(std::move(value));

        path().condition() &= ~jump_condition;
    }

    z3::expression reil_disassembler::get_value(reil_arg_t const& argument) const
    {
        auto& state = path().state();
        switch (argument.type)
        {
        case A_REG:
            return state[get_key(argument)];
        case A_TEMP:
            return temporary_state_[get_key(argument)];
        case A_CONST:
        case A_LOC:
            return z3::expression(get_width(argument), argument.val);
        default:
            throw std::logic_error("Unexpected type");
        }
    }
    void reil_disassembler::set_value(reil_arg_t const& argument, z3::expression value) const
    {
        auto key = get_key(argument);

        auto& state = path().state();
        switch (argument.type)
        {
        case A_REG:
            state.define(std::move(key), std::move(value));
            break;
        case A_TEMP:
            temporary_state_.define(std::move(key), std::move(value));
            break;
        default:
            throw std::logic_error("Unexpected type");
        }
    }
}

static_assert(std::is_destructible_v<grev::reil_disassembler>);

static_assert(std::is_copy_constructible_v<grev::reil_disassembler>);
static_assert(std::is_nothrow_move_constructible_v<grev::reil_disassembler>);

static_assert(std::is_copy_assignable_v<grev::reil_disassembler>);
static_assert(std::is_nothrow_move_assignable_v<grev::reil_disassembler>);
