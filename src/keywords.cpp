#include "keywords.hpp"

#include <unordered_map>

const std::unordered_map<string_view, Keyword> keyword_map {
    {"BEGIN", Keyword::BEGIN},
    {"END", Keyword::END},
    {"WHILE", Keyword::WHILE},
    {"FOR", Keyword::FOR},
    {"TO", Keyword::TO},
    {"DO", Keyword::DO},
    {"IF", Keyword::IF},
    {"THEN", Keyword::THEN},
    {"ELSE", Keyword::ELSE},
    {"DISPLAY", Keyword::DISPLAY},
    {"VAR", Keyword::VAR}
};
