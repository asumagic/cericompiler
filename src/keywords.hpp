#pragma once

#include "util/string_view.hpp"

#include <unordered_map>

enum class Keyword
{
	BAD_KEYWORD,
	BEGIN,
	END,
	WHILE,
	FOR,
	TO,
	DO,
	IF,
	THEN,
	ELSE,
	DISPLAY,
	VAR
};

extern const std::unordered_map<string_view, Keyword> keyword_map;
