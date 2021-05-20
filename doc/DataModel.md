
# Input Syntax

Check out the file [sample.moc](../examples/sample/sample.moc) for how
to code up the various types.

Allowed data structures:

- types, structs, enums and typedefs
- classes and interfaces
- constants
- top-level code blocks: header_code, header_end_code, source_code

# Basic Data Structures

The following are the types of data structures are modeled by Moc:

- Modules - the *.moc input file. You can specify multiple files on the
command line, in which case you'll probably want to interate over them
to spit all cllasses and types organized by their module name.
- Classes or Interfaces. These can have Properties (AKA fields) or Methods (AKA operations).
- Structs. These can have Properties (Fields).
- Properties - these represent fields on Classes, Structs and Interfaces.
- Methods - this represent operations on Classes and Interfaces.
- Arguments - Methods can have arguments. There are in Arguments and
error Arguments (Exceptions).
- Types, Enums, Typedefs and Constants
- Annotations - these are Key-Value pairs that can be attached to most of 
the above data structures.

Each of the above data structures can have attributes that describe aspects
of those data structures. Nearly all of the above have a 'Name' attribute
that can be queried to get the name.

## Types, structs, enums and typedefs

Structs, opaque-types, typedefs and enums are all various kinds of 
a 'Type'. 

You declare a type with one of the following
### Opaque Type
An opaque type is basically a symbol name. You can set a Header attribute
and attach annotations to the opaque type. Examples are:

```
type $Name;
// or
type $Name {
   Header $HeaderToIncludeForThisType;
};
```

For instance:

```
type Type1;

type Type2 {
    Header "headerName";
};
```

Examples of opaque (to MOC) types are the built-in types:
- integer
- boolean
- real
- string

```
struct BuiltInTypes {
     integer f1;
     boolean f2;
     real f3;
     string f4;
};
```
These built-in types can be used without declaring them .

Other opaque types can be declared with as:
```
type String { Header "<string>"; };
type int64_t;
```

Types may be annotated with key value pairs. Annotated
types may then be filtered and/or queried in the template
during code generation to control what is generated.

Here is a String that has additional annotations that can
control how database code will be generated for this type.

```
type $Name {
  [[ annotation1OnType {va2} ]];
  ...
  [[ annotationNOnType {va2} ]];
};

// example string type
type String {
    Header "<string>";
    [[ namespace {std} ]];
    [[ type {std::string} ]];
    [[ header_name {string}]];

    [[ dbGetResult {rs.getString(col++)} ]];
    [[ dbcoltype {VARCHAR(255)}]];
    [[ objectArrayTypePref {}]];
    [[ objectArrayTypeSuf {}]];
};
```

### Typedefs

Typedefs are just aliases for an existing types (or for
classes and interfaces).

```
type SomeType;

// An alias to SomeType
typedef SomeType AnAliasToSomeType;

// this is the long form of the typedef. Use this
// form to set the Header attribute and to add annotations
// to the typedef.
type AnotherAliasToSomeType {
    typedef SomeType @;
};
```
To add annotations to typedefs, you must use the following
form:
```
type AliasOfAnotherStruct {
    [[ annotationOnAliasType { v1 } ]];
    typedef AnotherStruct @;
};
```

### Structs

Structs are aggregates, thus they can only have properties
with the allowed property types being any valid types e.g.

opaque types, structs, built-in, typedefs or enums.

Structs look like:

```
struct $Name {
  $PropertyType $Name;
};

struct AStruct {
    integer id;
};

struct AStructWithoutAnyPropertiesOrMethods {};

// this is the long form of the typedef. Use this
// form to set the Header attribute on the struct.
type YetAnotherStruct {
    Header "TheHeader";
    struct @ {
        integer f1;
        string f2;
    };
};
```

Annotations can be added to the struct as well as to 
its properties.

```
struct AStructWithAnnotations {
   [[ annotatedStruct { oh yeah } ]]
};

struct AStructWithAnnotatedProperty {
   EnumWithAnnotations enum1,
       [[ annotation1OnEnum1 { v3 } ]],
       [[ annotation2OnEnum1 { v4 } ]];
};
```

### Enums
Enums are a simple type consisting of a fix set of
values. Each value may be assigned an integer constant.

```
enum $Name {
    $Entry1[=$Value], ... $EntryN[=$ValueN];
};
// or
type $Name {
   enum @ { $Entry1[=$Value], ... $EntryN[=$ValueN] };
};
```

For instance:

```
enum Color {
    Red, White, Blue
};

type AnotherEnum {
    enum @ { Red, White, Blue };
};

```

If you wish to add annotations to enums, you must
use the long form below:

```
type EnumWithAnnotations {
  Header "someHeader";
  [[ annotation1OnEnum {va2} ]];
  [[ annotation2OnEnum {va2} ]];
  enum @ {
     One, OrNone
  };
};
```

## Class and Interface

Classes and interfaces may have properties and/or methods
on them. Classes, interfaces and their messages are 
automatically assigned typecodes to assist in interprocess
communication however if you're not using Moc to create
RPC/IDL like message structures, the generated type codes
can be safely ignored.


```
class $Name : [public] $SuperClass1, ... [public] $SuperClassN {
  $PropertyType $Name;
  $ReturnType $MethodName ( $ArgType $argName, ... );
};

interface $Name : [public] $SuperClass1, ... [public] $SuperClassN {
  $PropertyType $Name;
  method $ReturnType $MethodName ( $ArgType $argName, ... )
     throws $Exception1, ... $ExceptionN;
};
```

Only classes, interfaces and methods on classes and
interfaces are given typecodes by default. 

To Annotate a class:
```
class AnnotationOnClass {
      [[ annotationOnClass {this class has an annotation}]];
      [[ annotation2OnClass {this class has another annotation}]];
};
```
To annotate a property/field of a class:
```
class AnnotationOnClassProperty {
      integer id, 
            [[ annotationOnFieldId {the field 'id' has an annotation}]],
            [[ annotation2OnFieldId {and another annotation}]];
};
```
To annotate a method of a class:
```
class AnnotationOnClassMethod {
      method SomeType aMethod1()
         [[ annotationOnAMethod1 { what to do } ]];

      method SomeType aMethod2(integer arg1)
         [[ annotationOnAMethod2 { what to do } ]],
         [[ annotationOnAMethod2 { what to do } ]];

      method SomeType aMethod3(integer arg1 )
         [[ annotationOnAMethod3 { what to do } ]],
         [[ annotationOnAMethod3 { what to do } ]]
         throws AClass;
};
```
To annotate a method argument of method on a class:
```

class AnnotationOnClassMethodArgument {
      // NOTE: there are no commas between the annotations
      // and the arguments. Commas separate the arguments.
      // spaces separate annotations on 1 argument
      method SomeType aMethod1(
              integer arg1
                [[ annotation1OnArg1 { v2 }]]
                [[ annotation2OnArg1 { v3 }]])
         [[ annotationOnAMethod1 { what to do } ]];

      method SomeType
         aMethod2(integer arg1
                    [[ annotation1OnArg1 { v2 }]]
                    [[ annotation2OnArg1 { v3 }]]  , // comma sep args
                  SomeType arg2
                    [[ annotation1OnArg2 { v2 }]]
                    [[ annotation2OnArg2 { v3 }]] )
         [[ annotationOnAMethod2 { what to do } ]];
};
```

## Constants

Integer constants are allowed. The integral value of
a constant is an integer expression of integers with
the following operations 

   +, -, *, /, |, &, ||, %, <<, >> 
   
and groupd using parentheses.

```
constant $Name = $integer;
```

## Top level Code Blocks

There can be 3 top level code blocks. header_code,
header_end_code and source_code. The last string
associated with each is what is used.

Warning MOC uses the C preprocessor (CPP) to 
preprocess the include files and macros. However this 
has its own issues such as the CPP handling of single 
quotes. You'll get warnings from the CPP about missing
terminated quotes if they aren't paired up on a line. 

Those errors you can generally ignore if your output 
matches your expectations.  Instead of using the quote
or double quote directly, you can use the sequence \x27 
for a single quote or \x22 for double quotes inside a 
code block and the many preprocessors won't complain.


```
header_code {
// a pass-thru code block that can be inserted at the 
// start of a header file.
static char header_code= "\
   typically it\\\x27s code inserted at the beginning \
   of a header file. Comments are removed.";
};

header_end_code {
// a pass-thru code block that can be inserted at the 
// end of a header file.
static char header_code= "\
   typically it\\\x27s code inserted at the beginning \
   of a header file. Comments are removed.";
};

source_code {
// a pass-thru code block that can be inserted somewhere
// in a source file.
static char source_code= " hi there";
};
```
  
# Next
See how a Template can use the meta data from these
data structures to generate output files. [Template](Template.md).

Files:
- [Design](Design.md) - some comments about the implementation
- [Input Syntax](DataModel.md) - description of the input format
- [Template](Template.md) - description of the Moc output template

