#pragma once

#include <tokenizer.hpp>

namespace Lexic {
	enum struct TYPE : size_t { OPINC = 0, OPDEC, OPSUM, OPSUB, OPMUL, OPDIV, OPREM, OPEQ, OPNEQ, OPASGN, PAOPEN, PACLOSE, NUMREAL, NUMINT, BOOLTRUE, BOOLFALSE, INVALID, COUNT };
	using LexicTokenizer = Tokenizer<TYPE, TYPE::INVALID, static_cast<size_t>(TYPE::COUNT)>;
	extern const LexicTokenizer tokenizer;
}
