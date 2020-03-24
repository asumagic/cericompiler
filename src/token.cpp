#include "token.hpp"

#include "util/enums.hpp"

bool is_token_keyword(TOKEN token) { return check_enum_range(token, TOKEN::FIRST_KEYWORD, TOKEN::LAST_KEYWORD); }
bool is_token_type(TOKEN token) { return check_enum_range(token, TOKEN::FIRST_TYPE, TOKEN::LAST_TYPE); }
bool is_token_addop(TOKEN token) { return check_enum_range(token, TOKEN::FIRST_ADDOP, TOKEN::LAST_ADDOP); }
bool is_token_mulop(TOKEN token) { return check_enum_range(token, TOKEN::FIRST_MULOP, TOKEN::LAST_MULOP); }
bool is_token_relop(TOKEN token) { return check_enum_range(token, TOKEN::FIRST_RELOP, TOKEN::LAST_RELOP); }
