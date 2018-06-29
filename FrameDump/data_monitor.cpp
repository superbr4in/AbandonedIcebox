#include "stdafx.h"

#include "data_monitor.h"

void data_monitor::apply(const instruction_x86& instruction)
{
    std::map<x86_reg, expr> register_updates;
    std::map<expr, expr> memory_updates;

    inspect_updates(instruction, register_updates, memory_updates);

    for (const auto& [reg, data_expr] : register_updates)
        register_map_[reg] = data_expr;

    for (const auto& [mem_expr, data_expr] : memory_updates)
        memory_map_[mem_expr] = data_expr;
}

std::string data_monitor::check(const x86_reg reg) const
{
    return safe_at_reg(reg).evaluate();
}

expr data_monitor::safe_at_reg(const x86_reg reg) const
{
    const auto it = register_map_.find(reg);

    if (it == register_map_.end())
        return expr::make_var(reg);

    return it->second;
}
expr data_monitor::safe_at_mem(const expr expr) const
{
    const auto it = memory_map_.find(expr);

    if (it == memory_map_.end())
        return expr;

    return it->second;
}

expr data_monitor::to_expr(const x86_op_mem mem) const
{
    std::optional<expr> expr = std::nullopt;

    if (mem.base != 0)
        expr = safe_at_reg(static_cast<x86_reg>(mem.base));
    if (mem.index != 0)
    {
        const auto index = safe_at_reg(static_cast<x86_reg>(mem.index));
        if (expr.has_value())
            expr = expr->append('+', index);
        else expr = index;
        if (mem.scale != 1)
            expr = expr->append('*', expr::make_const(mem.scale));
    }
    if (mem.disp != 0)
    {
        const auto disp = expr::make_const(mem.disp);
        if (expr.has_value())
            expr = expr->append('+', disp);
        else expr = disp;
    }

    return *expr;
}

void data_monitor::inspect_updates(const instruction_x86& instruction,
    std::map<x86_reg, expr>& reg_updates, std::map<expr, expr>& mem_updates) const
{
    reg_updates =
    {
        { X86_REG_RIP, expr::make_const(/*static_cast<int64_t>(*/instruction.address/*)*/) } // TODO
    };
    mem_updates = { };

    std::optional<operand_x86> op0 = std::nullopt;
    if (!instruction.operands.empty())
        op0.emplace(instruction.operands.at(0));
    std::optional<operand_x86> op1 = std::nullopt;
    if (instruction.operands.size() > 1)
        op1.emplace(instruction.operands.at(1));

    switch (instruction.type)
    {
    case ins_push:
        {
            const auto rsp = safe_at_reg(X86_REG_RSP).append('-', expr::make_const(8));
            reg_updates.emplace(X86_REG_RSP, rsp);
            if (op0->type == op_register)
                mem_updates.emplace(rsp, safe_at_reg(std::get<op_register>(op0->value)));
            if (op0->type == op_immediate)
                mem_updates.emplace(rsp, expr::make_const(std::get<op_immediate>(op0->value)));
        }
        break;
    case ins_pop:
        {
            const auto rsp = safe_at_reg(X86_REG_RSP);
            reg_updates.emplace(std::get<op_register>(op0->value), safe_at_mem(rsp));
            reg_updates.emplace(X86_REG_RSP, rsp.append('+', expr::make_const(8)));
        }
        break;
    case ins_call:
        {
            const auto rsp = safe_at_reg(X86_REG_RSP).append('-', expr::make_const(8));
            reg_updates.emplace(X86_REG_RSP, rsp);
            mem_updates.emplace(rsp, expr::make_const(instruction.address + instruction.code.size())); // TODO: reg_updates[RIP]
        }
        break;
    case ins_return:
        {
            const auto rsp = safe_at_reg(X86_REG_RSP);
            reg_updates.emplace(X86_REG_RIP, safe_at_mem(rsp));
            reg_updates.emplace(X86_REG_RSP, rsp.append('+', expr::make_const(8)));
        }
        break;
    case ins_move:
        if (op0->type == op_register)
        {
            const auto reg0 = std::get<op_register>(op0->value);
            if (op1->type == op_register)
                reg_updates.emplace(reg0, safe_at_reg(std::get<op_register>(op1->value)));
            if (op1->type == op_immediate)
                reg_updates.emplace(reg0, expr::make_const(std::get<op_immediate>(op1->value)));
            if (op1->type == op_memory)
                reg_updates.emplace(reg0, safe_at_mem(to_expr(std::get<op_memory>(op1->value))));
        }
        if (op0->type == op_memory)
        {
            const auto mem0 = std::get<op_memory>(op0->value);
            if (op1->type == op_register)
                mem_updates.emplace(to_expr(mem0), safe_at_reg(std::get<op_register>(op1->value)));
            if (op1->type == op_immediate)
                mem_updates.emplace(to_expr(mem0), expr::make_const(std::get<op_immediate>(op1->value)));
        }
        break;
    case ins_arithmetic:
        if (instruction.id == X86_INS_MUL)
        {
            reg_updates.emplace(X86_REG_RAX, safe_at_reg(X86_REG_RAX).append('*', safe_at_reg(std::get<op_register>(op0->value))));
            reg_updates.emplace(X86_REG_RDX, expr::make_const(0));
        }
        else if (instruction.id == X86_INS_DIV)
        {
            const auto rax = safe_at_reg(X86_REG_RAX);
            const auto reg0 = safe_at_reg(std::get<op_register>(op0->value));
            reg_updates.emplace(X86_REG_RAX, rax.append('/', reg0));
            reg_updates.emplace(X86_REG_RDX, rax.append('%', reg0));
        }
        else
        {
            expr other;
            if (op1->type == op_register)
                other = safe_at_reg(std::get<op_register>(op1->value));
            if (op1->type == op_immediate)
                other = expr::make_const(std::get<op_immediate>(op1->value));
            if (op1->type == op_memory)
                other = safe_at_mem(to_expr(std::get<op_memory>(op1->value)));
            reg_updates.emplace(std::get<op_register>(op0->value),
                safe_at_reg(std::get<op_register>(op0->value)).append(instruction.id == X86_INS_ADD ? '+' : '-', other));
        }
        break;
    default:
        if (instruction.id == X86_INS_DEC)
        {
            const auto reg0 = std::get<op_register>(op0->value);
            reg_updates.emplace(reg0, safe_at_reg(reg0).append('-', expr::make_const(1)));
        }
        if (instruction.id == X86_INS_INC)
        {
            const auto reg0 = std::get<op_register>(op0->value);
            reg_updates.emplace(reg0, safe_at_reg(reg0).append('+', expr::make_const(1)));
        }
        if (instruction.id == X86_INS_LEA)
            reg_updates.emplace(std::get<op_register>(op0->value), to_expr(std::get<op_memory>(op1->value)));
        if (instruction.id == X86_INS_NEG)
        {
            const auto reg0 = std::get<op_register>(op0->value);
            reg_updates.emplace(reg0, safe_at_reg(reg0).wrap('-'));
        }
        if (instruction.id == X86_INS_XCHG)
        {
            if (op0->type == op_register)
            {
                const auto reg0 = std::get<op_register>(op0->value);
                if (op1->type == op_register)
                {
                    const auto reg1 = std::get<op_register>(op1->value);
                    reg_updates.emplace(reg0, safe_at_reg(reg1));
                    reg_updates.emplace(reg1, safe_at_reg(reg0));
                }
                if (op1->type == op_memory)
                {
                    const auto mem1 = std::get<op_memory>(op1->value);
                    reg_updates.emplace(reg0, safe_at_mem(to_expr(mem1)));
                    mem_updates.emplace(to_expr(mem1), safe_at_reg(reg0));
                }
            }
            if (op0->type == op_memory)
            {
                const auto mem0 = std::get<op_memory>(op0->value);
                const auto reg1 = std::get<op_register>(op1->value);
                mem_updates.emplace(to_expr(mem0), safe_at_reg(reg1));
                reg_updates.emplace(reg1, safe_at_mem(to_expr(mem0)));
            }
        }
        break;
    }
}
