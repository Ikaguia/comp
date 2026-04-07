#include <lexic.hpp>

namespace Lexic {
	using Type = LexicTokenizer::Type;

	const LexicTokenizer tokenizer{
		std::array<Type, static_cast<size_t>(TYPE::COUNT)> {
			Type{ TYPE::OPINC,		"OpInc",		R"([+]{2})",	false, {} },
			Type{ TYPE::OPDEC,		"OpDec",		R"([-]{2})",	false, {} },
			Type{ TYPE::OPSUM,		"OpSum",		R"([+])",		false, {} },
			Type{ TYPE::OPSUB,		"OpSub",		R"([-])",		false, {} },
			Type{ TYPE::OPMUL,		"OpMul",		R"([*])",		false, {} },
			Type{ TYPE::OPDIV,		"OpDiv",		R"([/])",		false, {} },
			Type{ TYPE::OPREM,		"OpRem",		R"([%])",		false, {} },
			Type{ TYPE::OPEQ,		"OpEquals",		R"([=]{2})",	false, {} },
			Type{ TYPE::OPNEQ,		"OpNotEquals",	R"([!][=])",	false, {} },
			Type{ TYPE::OPASGN,		"OpAssign",		R"([=])",		false, {} },
			Type{ TYPE::PAOPEN,		"ParOpen",		R"([(])",		false, {} },
			Type{ TYPE::PACLOSE,	"ParClose",		R"([)])",		false, {} },
			Type{ TYPE::NUMREAL,	"NumReal",		R"(\d+\.\d+)",	true,  {} },
			Type{ TYPE::NUMINT,		"NumInt",		R"(\d+)",		true,  {} },
			Type{ TYPE::BOOLTRUE,	"BoolTrue",		R"([Tt]rue)",	false, {} },
			Type{ TYPE::BOOLFALSE,	"BoolFalse",	R"([Ff]alse)",	false, {} },
			Type{ TYPE::INVALID,	"Invalid",		"[^\\s]",		false, {} },
		}
	};
}
