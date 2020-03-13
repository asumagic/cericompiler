#include "operators.hpp"

const std::unordered_map<StringView, RelationalOperator> relational_operator_names{
	{"==", RelationalOperator::EQU},
	{"!=", RelationalOperator::DIFF},
	{"<", RelationalOperator::INF},
	{">", RelationalOperator::SUP},
	{"<=", RelationalOperator::INFE},
	{">=", RelationalOperator::SUPE}};

const std::unordered_map<StringView, AdditiveOperator> additive_operator_names{
	{"+", AdditiveOperator::ADD}, {"-", AdditiveOperator::SUB}, {"||", AdditiveOperator::OR}};

const std::unordered_map<StringView, MultiplicativeOperator> multiplicative_operator_names{
	{"*", MultiplicativeOperator::MUL},
	{"/", MultiplicativeOperator::DIV},
	{"%", MultiplicativeOperator::MOD},
	{"&&", MultiplicativeOperator::AND}};
