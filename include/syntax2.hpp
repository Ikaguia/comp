#pragma once

#include <span>

#include <lexic.hpp>

namespace Syntax {
	using Token = Lexic::LexicTokenizer::Token;
	void analyze(std::span<const Token> tks);
};
