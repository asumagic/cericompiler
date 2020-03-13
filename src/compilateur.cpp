//  A compiler from a very simple Pascal-like structured language LL(k)
//  to 64-bit 80x86 Assembly langage
//  Copyright (C) 2019 Pierre Jourlin
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "compilateur.hpp"
#include "codegen.hpp"
#include "tokeniser.hpp"

#include <FlexLexer.h>

#include <array>
#include <cassert>
#include <cstring>
#include <iostream>
#include <set>
#include <string>

using std::cerr;
using std::cout;
using std::string;

using namespace std::string_literals;

TOKEN current; // Current token

FlexLexer* lexer = new yyFlexLexer; // This is the flex tokeniser
// tokens can be read using lexer->yylex()
// lexer->yylex() returns the type of the lexicon entry (see enum TOKEN in tokeniser.h)
// and lexer->YYText() returns the lexicon entry as a string

std::set<string> DeclaredVariables;
unsigned long    TagNumber = 0;

bool IsDeclared(const char* id) { return DeclaredVariables.find(id) != DeclaredVariables.end(); }

void PrintErrorPreamble() { cerr << "source:" << lexer->lineno() << ": "; }

void Error(const char* s)
{
	PrintErrorPreamble();
	cerr << "error: " << s << '\n';

	PrintErrorPreamble();
	cerr << "note: when reading token '" << lexer->YYText() << "'\n";
	exit(-1);
}

void TypeCheck(Type a, Type b)
{
	assert(int(a) < int(Type::CONCEPT_BEGIN) && "Only the second operand of TypeCheck may be a type concept");

	bool match = true;

	if (b == Type::ARITHMETIC)
	{
		if (a != Type::UNSIGNED_INT)
		{
			match = false;
		}
	}
	else if (a != b)
	{
		match = false;
	}

	if (!match)
	{
		Error(("incompatible types: "s + name(a) + ", " + name(b)).c_str());
	}
}

TOKEN read_token() { return (current = TOKEN(lexer->yylex())); }

Type Identifier()
{
	cout << "\tpush " << lexer->YYText() << '\n';
	read_token();

	return Type::UNSIGNED_INT;
}

Type Number()
{
	cout << "\tpush $" << atoi(lexer->YYText()) << '\n';
	read_token();

	return Type::UNSIGNED_INT;
}

Type Factor()
{
	if (current == RPARENT)
	{
		read_token();
		Type type = Expression();
		if (current != LPARENT)
			Error("expected ')'");
		else
			read_token();

		return type;
	}

	if (current == NUMBER)
	{
		return Number();
	}

	if (current == ID)
	{
		return Identifier();
	}

	Error("expected '(', number or identifier");
}

OPMUL MultiplicativeOperator()
{
	OPMUL opmul;
	if (strcmp(lexer->YYText(), "*") == 0)
		opmul = MUL;
	else if (strcmp(lexer->YYText(), "/") == 0)
		opmul = DIV;
	else if (strcmp(lexer->YYText(), "%") == 0)
		opmul = MOD;
	else if (strcmp(lexer->YYText(), "&&") == 0)
		opmul = AND;
	else
		opmul = WTFM;
	read_token();
	return opmul;
}

Type Term()
{
	OPMUL      mulop;
	const Type first_type = Factor();
	while (current == MULOP)
	{
		mulop               = MultiplicativeOperator(); // Save operator in local variable
		const Type nth_type = Factor();

		TypeCheck(first_type, nth_type);

		cout << "\tpop %rbx\n"; // get first operand
		cout << "\tpop %rax\n"; // get second operand
		switch (mulop)
		{
		case AND:
			TypeCheck(first_type, Type::BOOLEAN);
			cout << "\tmulq	%rbx\n";        // a * b -> %rdx:%rax
			cout << "\tpush %rax\t# AND\n"; // store result
			break;
		case MUL:
			TypeCheck(first_type, Type::ARITHMETIC);
			cout << "\tmulq	%rbx\n";        // a * b -> %rdx:%rax
			cout << "\tpush %rax\t# MUL\n"; // store result
			break;
		case DIV:
			TypeCheck(first_type, Type::ARITHMETIC);
			cout << "\tmovq $0, %rdx\n";    // Higher part of numerator
			cout << "\tdiv %rbx\n";         // quotient goes to %rax
			cout << "\tpush %rax\t# DIV\n"; // store result
			break;
		case MOD:
			TypeCheck(first_type, Type::ARITHMETIC);
			cout << "\tmovq $0, %rdx\n";    // Higher part of numerator
			cout << "\tdiv %rbx\n";         // remainder goes to %rdx
			cout << "\tpush %rdx\t# MOD\n"; // store result
			break;
		case WTFM:
		default: Error("unknown multiplicative operator");
		}
	}

	return first_type;
}

OPADD AdditiveOperator()
{
	OPADD opadd;
	if (strcmp(lexer->YYText(), "+") == 0)
		opadd = ADD;
	else if (strcmp(lexer->YYText(), "-") == 0)
		opadd = SUB;
	else if (strcmp(lexer->YYText(), "||") == 0)
		opadd = OR;
	else
		opadd = WTFA;
	read_token();
	return opadd;
}

Type SimpleExpression()
{
	OPADD      adop;
	const Type first_type = Term();
	while (current == ADDOP)
	{
		adop                = AdditiveOperator(); // Save operator in local variable
		const Type nth_type = Term();

		TypeCheck(first_type, nth_type);

		cout << "\tpop %rbx\n"; // get first operand
		cout << "\tpop %rax\n"; // get second operand
		switch (adop)
		{
		case OR:
			TypeCheck(first_type, Type::BOOLEAN);
			cout << "\taddq	%rbx, %rax\t# OR\n"; // operand1 OR operand2
			break;
		case ADD:
			TypeCheck(first_type, Type::ARITHMETIC);
			cout << "\taddq	%rbx, %rax\t# ADD\n"; // add both operands
			break;
		case SUB:
			TypeCheck(first_type, Type::ARITHMETIC);
			cout << "\tsubq	%rbx, %rax\t# SUB\n"; // substract both operands
			break;
		case WTFA:
		default: Error("unknown additive operator");
		}
		cout << "\tpush %rax\n"; // store result
	}

	return first_type;
}

void DeclarationPart()
{
	if (current != RBRACKET)
		Error("expected '[' to begin variable declaration block");

	read_token();
	if (current != ID)
		Error("expected an identifier");
	cout << lexer->YYText() << ":\t.quad 0\n";
	DeclaredVariables.insert(lexer->YYText());
	read_token();
	while (current == COMMA)
	{
		read_token();
		if (current != ID)
			Error("expected an identifier");
		cout << lexer->YYText() << ":\t.quad 0\n";
		DeclaredVariables.insert(lexer->YYText());
		read_token();
	}
	if (current != LBRACKET)
		Error("expected ']' to end variable declaration block");
	read_token();
}

OPREL RelationalOperator()
{
	OPREL oprel = OPREL::WTFR;
	if (strcmp(lexer->YYText(), "==") == 0)
		oprel = OPREL::EQU;
	else if (strcmp(lexer->YYText(), "!=") == 0)
		oprel = OPREL::DIFF;
	else if (strcmp(lexer->YYText(), "<") == 0)
		oprel = OPREL::INF;
	else if (strcmp(lexer->YYText(), ">") == 0)
		oprel = OPREL::SUP;
	else if (strcmp(lexer->YYText(), "<=") == 0)
		oprel = OPREL::INFE;
	else if (strcmp(lexer->YYText(), ">=") == 0)
		oprel = OPREL::SUPE;

	read_token();
	return oprel;
}

Type Expression()
{
	OPREL      oprel;
	const Type first_type = SimpleExpression();
	if (current == RELOP)
	{
		oprel               = RelationalOperator();
		const Type nth_type = SimpleExpression();

		TypeCheck(first_type, nth_type);

		cout << "\tpop %rax\n";
		cout << "\tpop %rbx\n";
		cout << "\tcmpq %rax, %rbx\n";
		switch (oprel)
		{
		case OPREL::EQU: cout << "\tje Vrai" << ++TagNumber << "\t# If equal\n"; break;
		case OPREL::DIFF: cout << "\tjne Vrai" << ++TagNumber << "\t# If different\n"; break;
		case OPREL::SUPE: cout << "\tjae Vrai" << ++TagNumber << "\t# If above or equal\n"; break;
		case OPREL::INFE: cout << "\tjbe Vrai" << ++TagNumber << "\t# If below or equal\n"; break;
		case OPREL::INF: cout << "\tjb Vrai" << ++TagNumber << "\t# If below\n"; break;
		case OPREL::SUP: cout << "\tja Vrai" << ++TagNumber << "\t# If above\n"; break;
		case OPREL::WTFR:
		default: Error("unknown comparison operator");
		}
		cout << "\tpush $0\t\t# False\n";
		cout << "\tjmp Suite" << TagNumber << '\n';
		cout << "Vrai" << TagNumber << ":\tpush $0xFFFFFFFFFFFFFFFF\t\t# True\n";
		cout << "Suite" << TagNumber << ":\n";

		return Type::BOOLEAN;
	}

	return first_type;
}

VariableAssignment AssignementStatement()
{
	string variable;
	if (current != ID)
		Error("expected an identifier");
	if (!IsDeclared(lexer->YYText()))
	{
		Error((std::string("variable '") + lexer->YYText() + "' not found").c_str());
	}
	variable = lexer->YYText();
	read_token();
	if (current != ASSIGN)
		Error("expected ':=' in variable assignment");
	read_token();
	Type type = Expression();
	cout << "\tpop " << variable << '\n';

	TypeCheck(type, Type::UNSIGNED_INT);

	return {variable, type};
}

void IfStatement()
{
	read_token();
	TypeCheck(Expression(), Type::BOOLEAN);

	const auto tag = ++TagNumber;

	cout << "\tpopq %rax\n";
	cout << "\ttest %rax, %rax\n";
	cout << "\tjz IfFalse" << tag << '\n';
	cout << "IfTrue" << tag << ":\n";

	if (current != KEYWORD || strcmp(lexer->YYText(), "THEN"))
	{
		Error("expected 'THEN' after conditional expression of 'IF' statement");
	}

	read_token();
	Statement();

	if (current == KEYWORD && strcmp(lexer->YYText(), "ELSE") == 0)
	{
		cout << "\tjmp Suite" << tag << '\n';
		cout << "IfFalse" << tag << ":\n";
		read_token();
		Statement();
	}
	else
	{
		cout << "IfFalse" << tag << ":\n";
	}

	cout << "Suite" << tag << ":\n";
}

void WhileStatement()
{
	const auto tag = ++TagNumber;

	cout << "WhileBegin" << tag << ":\n";

	read_token();
	const Type type = Expression();
	TypeCheck(type, Type::BOOLEAN);

	cout << "\tpop %rax\n";
	cout << "\ttest %rax, %rax\n";
	cout << "\tjz Suite" << tag << '\n';

	if (current != KEYWORD || strcmp(lexer->YYText(), "DO"))
	{
		Error("expected 'DO' after conditional expression of 'WHILE' statement");
	}

	read_token();
	Statement();

	cout << "\tjmp WhileBegin" << tag << '\n';
	cout << "Suite" << tag << ":\n";
}

void ForStatement()
{
	const auto tag = ++TagNumber;

	read_token();
	const auto assignment = AssignementStatement();
	TypeCheck(assignment.type, Type::UNSIGNED_INT);

	if (current != KEYWORD || strcmp(lexer->YYText(), "TO") != 0)
	{
		Error("expected 'TO' after assignement in 'FOR' statement");
	}

	cout << "ForBegin" << tag << ":\n";

	read_token();
	TypeCheck(Expression(), Type::UNSIGNED_INT);

	cout << "\tpop %rax\n";
	// we branch *out* if var < %rax, mind the op order in at&t
	cout << "\tcmpq " << assignment.variable << ", %rax\n";
	cout << "\tjl Suite" << tag << '\n';

	if (current != KEYWORD || strcmp(lexer->YYText(), "DO") != 0)
	{
		Error("expected 'DO' after max expression in 'FOR' statement");
	}

	read_token();
	Statement();

	cout << "\taddq $1, " << assignment.variable << '\n';
	cout << "\tjmp ForBegin" << tag << '\n';
	cout << "Suite" << tag << ":\n";
}

void BlockStatement()
{
	do
	{
		read_token();
		Statement();
	} while (current == SEMICOLON);

	if (current != KEYWORD || strcmp(lexer->YYText(), "END") != 0)
	{
		Error("expected 'END' to finish block statement");
	}

	read_token();
}

void DisplayStatement()
{
	read_token();
	const Type type = Expression();

	switch (type)
	{
	case Type::UNSIGNED_INT:
	{
		cout << "\tmovq $__cc_format_string, %rdi\n";
		break;
	}

	default:
	{
		Error("unimplemented DISPLAY statement for this type, sorry");
	}
	}

	cout << "\tpop %rsi # Value to be displayed\n";
	cout << "\tmovb $0, %al # Number of floating point parameters (varargs)\n";
	cout << "\tcall printf\n";
}

void Statement()
{
	if (strcmp(lexer->YYText(), "IF") == 0)
	{
		IfStatement();
	}
	else if (strcmp(lexer->YYText(), "WHILE") == 0)
	{
		WhileStatement();
	}
	else if (strcmp(lexer->YYText(), "FOR") == 0)
	{
		ForStatement();
	}
	else if (strcmp(lexer->YYText(), "BEGIN") == 0)
	{
		BlockStatement();
	}
	else if (strcmp(lexer->YYText(), "DISPLAY") == 0)
	{
		DisplayStatement();
	}
	else
	{
		AssignementStatement();
	}
}

void StatementPart()
{
	emit_main_preamble();

	Statement();
	while (current == SEMICOLON)
	{
		read_token();
		Statement();
	}
	if (current != DOT)
		Error("expected '.' (did you forget a ';'?)");
	read_token();
}

void Program()
{
	cout << "\t.data\n";
	cout << "\t.align 8\n";
	cout << "__cc_format_string: .string \"%llu\\n\"\n";

	if (current == RBRACKET)
		DeclarationPart();
	StatementPart();
}

int main()
{
	// Read the source from stdin, output the assembly to stdout

	cout << "# This code was produced by the CERI Compiler\n";

	// Let's proceed to the analysis and code production
	read_token();
	Program();
	// Trailer for the gcc assembler / linker
	cout << "\tmovq %rbp, %rsp\t\t# Restore the position of the stack's top\n";
	cout << "\tret\t\t\t# Return from main function\n";
	if (current != FEOF)
	{
		Error((string("extraneous characters at end of file: [") + std::to_string(current) + "]").c_str());
	}
}
