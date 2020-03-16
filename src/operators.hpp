#pragma once

#include "util/string_view.hpp"

#include <unordered_map>

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

extern const std::unordered_map<string_view, RelationalOperator> relational_operator_names;

// AdditiveOperator := "+" | "-" | "||"
enum class AdditiveOperator
{
	WTFA,
	ADD, // +
	SUB, // -
	OR   // ||
};

extern const std::unordered_map<string_view, AdditiveOperator> additive_operator_names;

// MultiplicativeOperator := "*" | "/" | "%" | "&&"
enum class MultiplicativeOperator
{
	WTFM,
	MUL, // *
	DIV, // /
	MOD, // %
	AND  // &&
};

extern const std::unordered_map<string_view, MultiplicativeOperator> multiplicative_operator_names;
