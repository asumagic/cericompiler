# CERIcompiler

A simple compiler.
From : Pascal-like imperative LL(k) langage
To : 64 bit 80x86 assembly langage (AT&T)

**Download the repository :**

> git clone git@framagit.org:jourlin/cericompiler.git

**Build the compiler with debug symbols :**

> g++ -ggdb compilateur.cpp -o compilateur

**Compile the test program :**

> cat test.p | ./compilateur > test.s

**Have a look at the output :**

> gedit test.s

**Produce the executable (with debug info) :**

> gcc -ggdb test.s -o test :

**Debug the executable :**

> ddd ./test

**Commit the new version :**

> git commit -a -m "What's new..."

**Send to your framagit :**

> git push -u origin master

**Get from your framagit :**

> git pull -u origin master

