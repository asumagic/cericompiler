#pragma once

// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="
enum class RelationalOperator
{
	EQU,  // ==
	DIFF, // !=
	INF,  // <
	SUP,  // >
	INFE, // <=
	SUPE, // >=
	WTFR
};

// AdditiveOperator := "+" | "-" | "||"
enum class AdditiveOperator
{
	ADD, // +
	SUB, // -
	OR,  // ||
	WTFA
};

// MultiplicativeOperator := "*" | "/" | "%" | "&&"
enum class MultiplicativeOperator
{
	MUL, // *
	DIV, // /
	MOD, // %
	AND, // &&
	WTFM
};
