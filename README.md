# CERIcompiler

A simple compiler.
From : Pascal-like imperative LL(k) langage
To : 64 bit 80x86 assembly langage (AT&T)

**Download the repository :**

> git clone git@framagit.org:jourlin/cericompiler.git

**Build the compiler and test it :**

> make test

**Have a look at the output :**

> gedit test.s

**Debug the executable :**

> ddd ./test

**Commit the new version :**

> git commit -a -m "What's new..."

**Send to your framagit :**

> git push -u origin master

**Get from your framagit :**

> git pull -u origin master

**This version Can handle :**

// Program := [DeclarationPart] StatementPart
// DeclarationPart := "[" Identifier {"," Identifier} "]"
// StatementPart := Statement {";" Statement} "."
// Statement := AssignementStatement
// AssignementStatement := Identifier ":=" Expression

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
// SimpleExpression := Term {AdditiveOperator Term}
// Term := Factor {MultiplicativeOperator Factor}
// Factor := Number | Letter | "(" Expression ")"| "!" Factor
// Number := Digit{Digit}
// Identifier := Letter {(Letter|Digit)}

// AdditiveOperator := "+" | "-" | "||"
// MultiplicativeOperator := "*" | "/" | "%" | "&&"
// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
// Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"
// Letter := "a"|...|"z"

