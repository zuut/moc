# Introduction

Moc was written in 1993 on cfront, g++ and Microsoft's C++ compilers prior to
the common existence of templates, exceptions, namespaces, the initial 
standard and the STL library. This was being compiled on both Windows, Solaris
and a little bit later Linux (Slackware), but many of the compilers on
the PC or Mac were more limited than those on Solaris. I'm suprised at 
how well the code translated into today's C++ language. In the late 80s
and early 90s, there weren't even bools or namespaces :) at this time. 
Only a few compilers had the template support and none had exceptions.
So for the collection classes it continued to use Stroustroup's original
CPP macros declare() and implement() to do generic programming (see the
generic.h header). I had to supply my own generic.h as it wasn't available
on most platforms.

Now in 2021, MOC if done today in C++, would look completely different and
while I'm tempted to rewrite it using todays tools, I don't know that
I'd even do a code generator today and if I did, whether flex/bison are
the right tools. So instead, I've decided to keep the code in tack as
sort of a historical piece. One major change was that I reformated the
source to be 4 space indent instead of 8 space indent. If you look thru
the code, you'll see how very much "C-like" this is.

One of the big issues in the early days, was that everyone had to (re-)
implement lots of utility classes such as collection classes and smart 
pointers prior to the STL library. This has its own limited smart pointers,
its own collection classes, a basic string class, an even more stripped
down Date class etc. 

I've made some updates to get this to compile with the latest C++ compilers
I've also removed the old makefiles for the various platforms and used
cmake 3.x to setup a multiplatfrom build system which is now much simpler than
before.

The changes required to get Moc to compile with C++17 included:

1) scoped the cout and cerr classes with std:: . and some rework on
 the member functions writing or reading the data.

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
the build uses sed to rename some of the generated functions. This was
necessary at the time of development, but I haven't investigate whether this
can be removed.

Files:
- [Design](Design.md) - some comments about the implementation
- [Input Syntax](DataModel.md) - description of the input format
- [Template](Template.md) - description of the Moc output template



