#pragma once

#include <unordered_map>

#include "typedefs.hpp"

// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="
enum class RelationalOperator
{
	WTFR,
	EQU,  // ==
	DIFF, // !=
	INF,  // <
	SUP,  // >
	INFE, // <=
	SUPE  // >=
};

extern const std::unordered_map<StringView, RelationalOperator> relational_operator_names;

// AdditiveOperator := "+" | "-" | "||"
enum class AdditiveOperator
{
	WTFA,
	ADD, // +
	SUB, // -
	OR   // ||
};

extern const std::unordered_map<StringView, AdditiveOperator> additive_operator_names;

// MultiplicativeOperator := "*" | "/" | "%" | "&&"
enum class MultiplicativeOperator
{
	WTFM,
	MUL, // *
	DIV, // /
	MOD, // %
	AND  // &&
};

extern const std::unordered_map<StringView, MultiplicativeOperator> multiplicative_operator_names;
