# Introduction

Moc was written in 1993 on cfront, g++ and Microsoft's C++ compilers prior to
the common existence of templates, exceptions, namespaces, the initial 
standard and the STL library. Further It used Stroustroup's original 
CPP macros declare() and implement() to do generic programming. 

This has its own limited smart pointers, its own collection classes, string
class etc. 

I've made some updates to get this to compile with the latest C++ compilers
I've also removed the old makefiles for the various platforms and used
cmake 3.x to setup a multiplatfrom build system which is now much simpler than
before.

The changes required to get Moc to compile with C++17 included:

1) scoped the cout and cerr classes with std:: .
2) \#define'd register keyword to be empty.



# Class Hierarchy Overview

The uf library (useful library) provides smart pointers and basic collection
classes.

Moc uses flex and bison to create its parsers for the Moc input files and
for the template files.

- MOC.y & MOCParser.l := these define the input parser that loads the *.moc 
files into various Input.h classes.

- IF.y & IFParser.l := defines the parser for the if control in the output template
- CONTROL.y, CONTROLParser.l := defines the parser for the set directive in the output template.

*.moc & *.moh files are parsed into the various Input classes found in Input.h

Moc parses the template into a set of Output classes found in Output.h

It then uses the Output classes to render the input classes into 1 or more
text files.

In order to support having multiple *.y grammars and *.l lexers in 1 program,
the build uses sed to rename some of the generated functions.

Files:
- [Design](Design.md) - some comments about the implementation
- [Input Syntax](DataModel.md) - description of the input format
- [Template](Template.md) - description of the Moc output template



