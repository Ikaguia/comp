#include <syntax2.hpp>

#include <print>
#include <regex>
#include <map>
#include <set>
#include <iostream>
#include <ranges>
#include <algorithm>
#include <span>
#include <limits>
#include <variant>
#include <optional>
#include <format>
#include <functional>
#include <vector>
#include <string>
#include <string_view>
#include <charconv>
#include <generator>

using namespace Syntax;

std::set<Lexic::TYPE> tkAssign = {
	Lexic::TYPE::OPASGNSHL, Lexic::TYPE::OPASGNSHR,
	Lexic::TYPE::OPASGNSUM, Lexic::TYPE::OPASGNSUB, Lexic::TYPE::OPASGNMUL, Lexic::TYPE::OPASGNDIV, Lexic::TYPE::OPASGNREM, Lexic::TYPE::OPASGNAND, Lexic::TYPE::OPASGNXOR, Lexic::TYPE::OPASGNOR,
	Lexic::TYPE::OPASGN,
}, tkOperator = {
	Lexic::TYPE::OPINC, Lexic::TYPE::OPDEC,
	// Lexic::TYPE::OPASGNSHL, Lexic::TYPE::OPASGNSHR,
	Lexic::TYPE::OPSHL, Lexic::TYPE::OPSHR,
	// Lexic::TYPE::OPASGNSUM, Lexic::TYPE::OPASGNSUB, Lexic::TYPE::OPASGNMUL, Lexic::TYPE::OPASGNDIV, Lexic::TYPE::OPASGNREM, Lexic::TYPE::OPASGNAND, Lexic::TYPE::OPASGNXOR, Lexic::TYPE::OPASGNOR,
	Lexic::TYPE::OPEQ, Lexic::TYPE::OPNEQ,
	Lexic::TYPE::OPLAND, Lexic::TYPE::OPLOR,
	Lexic::TYPE::OPARROW,
	Lexic::TYPE::OPLE, Lexic::TYPE::OPGE,
	// Lexic::TYPE::OPASGN,
	Lexic::TYPE::OPSUM, Lexic::TYPE::OPSUB, Lexic::TYPE::OPMUL, Lexic::TYPE::OPDIV, Lexic::TYPE::OPREM,
	Lexic::TYPE::OPNOT, Lexic::TYPE::OPBNOT, Lexic::TYPE::OPAND, Lexic::TYPE::OPOR, Lexic::TYPE::OPXOR,
	Lexic::TYPE::OPLT, Lexic::TYPE::OPGT,
	Lexic::TYPE::OPCOND, Lexic::TYPE::OPCOLON, Lexic::TYPE::OPCOMMA, Lexic::TYPE::OPDOT,
	// Lexic::TYPE::PAOPEN, Lexic::TYPE::PACLOSE,
	// Lexic::TYPE::OPBRKOPEN, Lexic::TYPE::OPBRKCLOSE,
	// Lexic::TYPE::NUMREAL, Lexic::TYPE::NUMINT, Lexic::TYPE::BOOLTRUE, Lexic::TYPE::BOOLFALSE,
	// Lexic::TYPE::SIZEOF, Lexic::TYPE::IDENT,
	// Lexic::TYPE::INVALID,
}, tkOperand = {
	Lexic::TYPE::NUMREAL, Lexic::TYPE::NUMINT, Lexic::TYPE::BOOLTRUE, Lexic::TYPE::BOOLFALSE, Lexic::TYPE::IDENT,
};

void Syntax::analyze(std::span<const Token> tks) {
	int operators=0, operands=0;
	bool assign=false, delimit=false;
	Token last{Lexic::TYPE::INVALID, ""};
	for (const Token& tk : tks) {
		if (delimit) {
			std::println("Erro: token encontrado após o delimitador de fim.");
			return;
		}

		if (tkAssign.contains(tk.type)) {
			if (assign) {
				std::println("Erro: Foi encontrado mais de um enunciado de atribuição.");
				return;
			} else if (operators > 0) {
				std::println("Erro: foi encontrado um operador antes do enunciado de atribuição.");
				return;
			} else if (operands < 1) {
				std::println("Erro: não foi encontrado um operando antes do enunciado de atribuição.");
				return;
			} else assign = true;
		} else if (tkOperator.contains(tk.type)) {
			if (tkOperator.contains(last.type)) {
				std::println("Erro: foram encontrados dois operadores seguidos, '{}' e '{}'.", last.str, tk.str);
				return;
			} else if (tkAssign.contains(last.type)) {
				std::println("Erro: foi encontrado um operador imediatamente após o enunciado de atribuição.");
				return;
			} else operators++;
			if (operators > 3) {
				std::println("Erro: foram encontrados mais de 3 operadores.");
				return;
			}
		} else if (tkOperand.contains(tk.type)) {
			if (tkOperand.contains(last.type)) {
				std::println("Erro: foram encontrados dois operandos seguidos, '{}' e '{}'.", last.str, tk.str);
				return;
			} else operands++;
		} else if (tk.type == Lexic::TYPE::DELIMIT) {
			delimit = true;
		} else {
			std::println("Erro: token inválido encontrado, '{}'.", tk.str);
			return;
		}
		last = tk;
	}
	if (!delimit) {
		std::println("Erro: delimitador de fim não encontrado.");
		return;
	}
}