// EvBuilder: tiny helper to emit correctly nested Wolf event commands.
#pragma once
namespace FM2
{
namespace C = ::Command;
using CT    = C::CommandType;
using Cmds  = C::Commands;
struct EB
{
	Cmds& out; uint8_t ind = 0;
	explicit EB(Cmds& o) : out(o) {}
	void raw(C::CommandShPtr::Command c) { c->m_indent = ind; out.push_back(c); }   // push a ready-made command object (e.g. a cloned Move)
	void cmd(CT code, uInts a = {}) { out.push_back(std::make_shared<C::Command>(code, a, tStrings{}, ind)); }
	// a body ends with one Blank at its own indent (a nested 499+Blank already provides it)
	void endBody() { if (out.empty() || out.back()->m_cid != CT::Blank || out.back()->m_indent != ind) cmd(CT::Blank); }
	void set(uint32_t v, uint32_t x) { cmd(CT::SetVariable, {v, x, 0, 0}); }
	void add(uint32_t v, uint32_t x) { cmd(CT::SetVariable, {v, x, 0, 0x100}); }
	void sub(uint32_t v, uint32_t x) { cmd(CT::SetVariable, {v, x, 0, 0x200}); }
	void mul(uint32_t v, uint32_t x) { cmd(CT::SetVariable, {v, x, 0, 0x300}); }
	void pic(uInts a, const std::wstring& text) { out.push_back(std::make_shared<C::CommandSpecialClasses::Picture>(CT::Picture, a, tStrings{text}, ind)); }
	template <class F, class G> void ifEq(uint32_t v, uint32_t x, F then, G els, bool hasElse)
	{
		cmd(CT::VariableCondition, {1, v, x, 2}); cmd(CT::ChoiceCase, {1});
		ind++; then(); endBody(); ind--;
		if (hasElse) { cmd(CT::ElseCase, {0}); ind++; els(); endBody(); ind--; }
		cmd(CT::BranchEnd); cmd(CT::Blank);
	}
	template <class F> void ifEq(uint32_t v, uint32_t x, F then) { ifEq(v, x, then, []{}, false); }
	// Write one cell of a runtime database table (CDB, "VarDB"): Database [type, item, field, packed, value].
	// packed = (7 << 16) [type, item and field are constants] | (0 << 8) [CDB, write] | (op << 4) [assignment operator: 0 '=', 1 '+=', 2 '-=']
	// The value is a constant (< 1,000,000) or a variable id (>= 1,000,000); the game's own commands have one empty string argument.
	void dbWrite(uint32_t type, uint32_t item, uint32_t field, uint32_t value, uint32_t op = 0)
	{
		out.push_back(std::make_shared<C::Command>(CT::Database, uInts{type, item, field, (7u << 16) | (op << 4), value}, tStrings{L""}, ind));
	}
	// --- string commands (SetString = code 122, StringCondition = code 112); layouts copied from the game's own commands:
	//  SetString [var, (operation << 8) | contentType, srcVar-or-0] + literal string for literal content (contentType 0), none for string-variable content (1).
	//  operations: 0 '=', 1 '+=', 8 remove all occurrences of the content from the variable.
	//  StringCondition [caseCount, var | (operator << 28)] + 4 strings (case literals, padded): operator 0 '==', 1 '!=', 2 includes, 3 starts with.
	void setStr(uint32_t var, const std::wstring& lit, uint32_t op = 0)
	{
		out.push_back(std::make_shared<C::Command>(CT::SetString, uInts{var, op << 8, 0}, tStrings{lit}, ind));
	}
	void setStrFromVar(uint32_t var, uint32_t src, uint32_t op)
	{
		out.push_back(std::make_shared<C::Command>(CT::SetString, uInts{var, (op << 8) | 1u, src}, tStrings{}, ind));
	}
	template <class F> void ifStrNe(uint32_t var, const std::wstring& lit, F then)
	{
		out.push_back(std::make_shared<C::Command>(CT::StringCondition, uInts{1, var | (1u << 28)}, tStrings{lit, L"", L"", L""}, ind));
		cmd(CT::ChoiceCase, {1});
		ind++; then(); endBody(); ind--;
		cmd(CT::BranchEnd); cmd(CT::Blank);
	}
	// numeric comparison without else: op 0 '>=', 1 '>', 2 '==', 3 '<=', 4 '<', 5 '!=' (as seen in the game's data)
	template <class F> void ifOp(uint32_t v, uint32_t x, uint32_t op, F then)
	{
		cmd(CT::VariableCondition, {1, v, x, op}); cmd(CT::ChoiceCase, {1});
		ind++; then(); endBody(); ind--;
		cmd(CT::BranchEnd); cmd(CT::Blank);
	}
	// if (var > x) with optional else: comparison op 1 is ">" in this game's data (e.g. event 122 tests V31 > 5)
	template <class F, class G> void ifGt(uint32_t v, uint32_t x, F then, G els, bool hasElse)
	{
		cmd(CT::VariableCondition, {1, v, x, 1}); cmd(CT::ChoiceCase, {1});
		ind++; then(); endBody(); ind--;
		if (hasElse) { cmd(CT::ElseCase, {0}); ind++; els(); endBody(); ind--; }
		cmd(CT::BranchEnd); cmd(CT::Blank);
	}
	void mod(uint32_t v, uint32_t x) { cmd(CT::SetVariable, {v, x, 0, 0x500}); }   // v %= x
	// if (var < 0): comparison op 4 is "<" in this game's data; used to clamp a currency at zero after a negative adjustment
	template <class F> void ifBelowZero(uint32_t v, F then)
	{
		cmd(CT::VariableCondition, {1, v, 0, 4}); cmd(CT::ChoiceCase, {1});
		ind++; then(); endBody(); ind--;
		cmd(CT::BranchEnd); cmd(CT::Blank);
	}
	// edge-triggered on a value that was already read (e.g. a BASIC input result): runs 'act' once when cell == code
	template <class F> void edgeVal(uint32_t cell, uint32_t code, uint32_t latch, F act)
	{
		ifEq(cell, code, [&] { ifEq(latch, 0, [&] { set(latch, 1); act(); }); }, [&] { set(latch, 0); }, true);
	}
	// edge-triggered key: reads key into 'cell', runs 'act' once per press (latch var)
	template <class F> void edge(uint32_t cell, uint32_t code, uint32_t latch, F act)
	{
		cmd(CT::InputKey, {cell, 4352, code});
		ifEq(cell, code, [&] { ifEq(latch, 0, [&] { set(latch, 1); act(); }); }, [&] { set(latch, 0); }, true);
	}
};
} // namespace FM2
