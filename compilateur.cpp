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

// Build with "make compilateur"


#include <string>
#include <iostream>
#include <cstdlib>
#include <set>

using namespace std;

enum OPREL {equ, diff, infe, supe, inf, sup, unknown};
char current, lookedAhead;				// Current char	
int NLookedAhead=0;
set<string> DeclaredVariables;
unsigned long TagNumber=0;

bool IsDeclared(char c){
	return DeclaredVariables.find(string(1,c))!=DeclaredVariables.end();
}

void ReadChar(void){
	if(NLookedAhead>0){
		current=lookedAhead;	// Char has already been read
		NLookedAhead--;
	}
	else
		// Read character and skip spaces until 
		// non space character is read
		while(cin.get(current) && (current==' '||current=='\t'||current=='\n'));
}

void LookAhead(void){
	while(cin.get(lookedAhead) && (lookedAhead==' '||lookedAhead=='\t'||lookedAhead=='\n'));
	NLookedAhead++;
}

void Error(string s){
	cerr << "Lu : '"<<current<<"' , mais ";
	cerr<< s << endl;
	exit(-1);
}

// Program := [DeclarationPart] StatementPart
// DeclarationPart := "[" Letter {"," Letter} "]"
// StatementPart := Statement {";" Statement} "."
// Statement := AssignementStatement
// AssignementStatement := Letter "=" Expression

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
// SimpleExpression := Term {AdditiveOperator Term}
// Term := Factor {MultiplicativeOperator Factor}
// Factor := Number | Letter | "(" Expression ")"| "!" Factor
// Number := Digit{Digit}

// AdditiveOperator := "+" | "-" | "||"
// MultiplicativeOperator := "*" | "/" | "%" | "&&"
// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
// Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"
// Letter := "a"|...|"z"
	
void AdditiveOperator(void){
	if(current=='+'||current=='-')
		ReadChar();
	else
		if(current=='|'){
			ReadChar();
			if(current!='|')
				Error("l'opérateur de comparaison s'écrit '||'");
			else
				ReadChar();
		}
		else
			Error("Opérateur additif attendu");	   // Additive operator expected
}
		
void Letter(void){
	if((current<'a')||(current>'z'))
		Error("lettre attendue");		   // Digit expected
	else{
		cout << "\tpush "<<current<<endl;
		ReadChar();
	}
}

void Number(void){
	unsigned long long number;
	if((current<'0')||(current>'9'))
		Error("chiffre attendu");		   // Digit expected
	else
		number=current-'0';
	ReadChar();
	while(current>='0' && current <='9'){
		number*=10;
		number+=current-'0';
		ReadChar();
	}
	cout <<"\tpush $"<<number<<endl;
}

void Expression(void);			// Called by Term() and calls Term()

void Factor(void){
	if(current=='('){
		ReadChar();
		Expression();
		if(current!=')')
			Error("')' était attendu");		// ")" expected
		else
			ReadChar();
	}
	else 
		if (current>='0' && current <='9')
			Number();
	     	else
				if(current>='a' && current <='z')
					Letter();
				else
					Error("'(' ou chiffre ou lettre attendue");
}

// MultiplicativeOperator := "*" | "/" | "%" | "&&"
void MultiplicativeOperator(void){
	if(current=='*'||current=='/'||current=='%')
		ReadChar();
	else
		if(current=='&'){
			ReadChar();
			if(current!='&')
				Error("l'opérateur ET s'écrit '&&'");
			else
				ReadChar();
		}
		else
			Error("Opérateur multiplicatif attendu");	   // Additive operator expected
}

// Term := Factor {MultiplicativeOperator Factor}
void Term(void){
	char mulop;
	Factor();
	while(current=='*'||current=='/'||current=='%'||current=='&'){
		mulop=current;		// Save operator in local variable
		MultiplicativeOperator();
		Factor();
		cout << "\tpop %rbx"<<endl;	// get first operand
		cout << "\tpop %rax"<<endl;	// get second operand
		switch(mulop){
			case '*':
			case '&':
				cout << "\tmulq	%rbx"<<endl;	// a * b -> %rdx:%rax
				cout << "\tpush %rax"<<endl;	// store result
				break;
			case '/':
				cout << "\tmovq $0, %rdx"<<endl; 	// Higher part of numerator  
				cout << "\tdiv %rbx"<<endl;			// quotient goes to %rax
				cout << "\tpush %rax"<<endl;		// store result
				break;
			case '%':
				cout << "\tmovq $0, %rdx"<<endl; 	// Higher part of numerator  
				cout << "\tdiv %rbx"<<endl;			// remainder goes to %rdx
				cout << "\tpush %rdx"<<endl;		// store result
				break;
			default:
				Error("opérateur additif attendu");
		}
	}
}

// SimpleExpression := Term {AdditiveOperator Term}
void SimpleExpression(void){
	char adop;
	Term();
	while(current=='+'||current=='-'||current=='|'){
		adop=current;		// Save operator in local variable
		AdditiveOperator();
		Term();
		cout << "\tpop %rbx"<<endl;	// get first operand
		cout << "\tpop %rax"<<endl;	// get second operand
		if(adop=='+')
			cout << "\taddq	%rbx, %rax"<<endl;	// add both operands
		else
			cout << "\tsubq	%rbx, %rax"<<endl;	// substract both operands
		cout << "\tpush %rax"<<endl;			// store result
	}

}

// DeclarationPart := "[" Letter {"," Letter} "]"
void DeclarationPart(void){
	if(current!='[')
		Error("caractère '[' attendu");
	cout << "\t.data"<<endl;
	cout << "\t.align 8"<<endl;
	
	ReadChar();
	if(current<'a'||current>'z')
		Error("Une lettre minuscule était attendue");
	cout << current << ":\t.quad 0"<<endl;
	DeclaredVariables.insert(string(1,current));
	ReadChar();
	while(current==','){
		ReadChar();
		if(current<'a'||current>'z')
			Error("Une lettre minuscule était attendue");
		cout << current << ":\t.quad 0"<<endl;
		DeclaredVariables.insert(string(1,current));
		ReadChar();
	}
	if(current!=']')
		Error("Caractère ']' attendu");
	ReadChar();
}

// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
OPREL RelationalOperator(void){
	if(current!='<'&&current!='>'&&current!='!'&&current!='=')
		return unknown;
	LookAhead();
	if(lookedAhead=='=')
		switch(current){
			case '=':
				ReadChar(); ReadChar(); 
				return equ;
			case '!': 
				ReadChar(); ReadChar(); 
				return diff;
			case '<': 
				ReadChar(); ReadChar(); 
				return infe;
			case '>': 
				ReadChar(); ReadChar(); 
				return supe;
		}
	switch(current){
		case '=':	// Next is not '='
				Error("utilisez '==' comme opérateur d'égalité");
		case '<':	ReadChar();
				return inf;
		case '>':	ReadChar();
				return sup;
		default:	Error("opérateur relationnel inconnu");
		
	}
	return unknown;	
}

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
void Expression(void){
	OPREL oprel;
	SimpleExpression();
	if(current=='='||current=='!'||current=='<'||current=='>'){
		oprel=RelationalOperator();
		SimpleExpression();
		cout << "\tpop %rax"<<endl;
		cout << "\tpop %rbx"<<endl;
		cout << "\tcmpq %rax, %rbx"<<endl;
		switch(oprel){
			case equ:
				cout << "\tje Vrai"<<++TagNumber<<"\t# If equal"<<endl;
				break;
			case diff:
				cout << "\tjne Vrai"<<++TagNumber<<"\t# If different"<<endl;
				break;
			case supe:
				cout << "\tjae Vrai"<<++TagNumber<<"\t# If above or equal"<<endl;
				break;
			case infe:
				cout << "\tjbe Vrai"<<++TagNumber<<"\t# If below or equal"<<endl;
				break;
			case inf:
				cout << "\tjb Vrai"<<++TagNumber<<"\t# If below"<<endl;
				break;
			case sup:
				cout << "\tja Vrai"<<++TagNumber<<"\t# If above"<<endl;
				break;
			default:
				Error("Opérateur de comparaison inconnu");
		}
		cout << "\tpush $0\t\t# False"<<endl;
		cout << "\tjmp Suite"<<TagNumber<<endl;
		cout << "Vrai"<<TagNumber<<":\tpush $0xFFFFFFFFFFFFFFFF\t\t# True"<<endl;	
		cout << "Suite"<<TagNumber<<":"<<endl;
	}
}

// AssignementStatement := Letter "=" Expression
void AssignementStatement(void){
	char letter;
	if(current<'a'||current>'z')
		Error("lettre minuscule attendue");
	letter=current;
	if(!IsDeclared(letter)){
		cerr << "Erreur : Variable '"<<letter<<"' non déclarée"<<endl;
		exit(-1);
	}
	ReadChar();
	if(current!='=')
		Error("caractère '=' attendu");
	ReadChar();
	Expression();
	cout << "\tpop "<<letter<<endl;
}

// Statement := AssignementStatement
void Statement(void){
	AssignementStatement();
}

// StatementPart := Statement {";" Statement} "."
void StatementPart(void){
	cout << "\t.text\t\t# The following lines contain the program"<<endl;
	cout << "\t.globl main\t# The main function must be visible from outside"<<endl;
	cout << "main:\t\t\t# The main function body :"<<endl;
	cout << "\tmovq %rsp, %rbp\t# Save the position of the stack's top"<<endl;
	Statement();
	while(current==';'){
		ReadChar();
		Statement();
	}
	if(current!='.')
		Error("caractère '.' attendu");
	ReadChar();
}

// Program := [DeclarationPart] StatementPart
void Program(void){
	if(current=='[')
		DeclarationPart();
	StatementPart();	
}

int main(void){	// First version : Source code on standard input and assembly code on standard output
	// Header for gcc assembler / linker
	cout << "\t\t\t# This code was produced by the CERI Compiler"<<endl;
	// Let's proceed to the analysis and code production
	ReadChar();
	Program();
	// Trailer for the gcc assembler / linker
	cout << "\tmovq %rbp, %rsp\t\t# Restore the position of the stack's top"<<endl;
	cout << "\tret\t\t\t# Return from main function"<<endl;
	if(cin.get(current)){
		cerr <<"Caractères en trop à la fin du programme : ["<<current<<"]";
		Error("."); // unexpected characters at the end of program
	}

}
		
			





