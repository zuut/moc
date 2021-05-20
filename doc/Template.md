Files:
- [Design](Design.md) - some comments about the implementation
- [Input Syntax](DataModel.md) - description of the input format
- [Template](Template.md) - description of the Moc output template

# Templates
Moc can be used to generate any text based file such as a C++ source
and header files, HTML, DDL, SQL, XML, JSON etc.

Check out the file [sample.tpl](../examples/sample/sample.tpl)
for an example of a template that walks down the data
structures. Another example is [test_app.tpl](../test_app/test_app.tpl).

Moc requires a single template file to specify what files it should generate.
This template has both control structures as well as output lines. 

Lines beginning with @ in the first column are interpreted as control 
directives for Moc's output engine. Any other lines are considered output
lines. 

SIDE NOTE: Moc will ignore blank lines before the first 'open $FILE' 
directive.

Output lines are regular text with optional embedded variable expressions
that will be expanded based on the input data structures. An example output
line looks something like:

```
      class @(Name) { 
```
The symbol '@' is a special character indicating that a Moc variable or
formatting rule is being used. The escape sequence '@@' will result in 
the regular '@' character being written to the output.

See the Output Lines section below for more info.

# Moc Template Control Structures

Any lines starting with an '@' in the first column are considered to be
Moc control unless it is "@@" which will be treated as a regular output line
with a single '@' as the first character. 

These are the available starting sequences for Moc control lines:

- "@#" comment
- "@@" :=> '@' Lines starting with this will not be a control line, but will be treated 
  as a regular output line beginning with the single character '@'.
- "@{" starts a control block e.g. if and foreach 
- "@}" ends a control block . Moc ignores everything after the @} . its treated as a comment 
  however to facilitate matching up with the start block, the command from the start block 
  should be repeated verbatitum.
- "@|" additional control directives for the current block. Doesn't change the nested level 
  of blocks
- "@>" command e.g. set


```
  an output line that must be @\
treated as a single line.
```
or

```
@>  a control line that must be @\
treated as a single line.
```
Lines can be continued by terminating them in the "@\" e.g.


Control blocks look like the following:

```
@{  $COMMAND 
...
@|  $OPTIONAL ADDITIONAL DIRECTIVES FOR THE CURRENT BLOCK
...
@|  $ENDS THE CURRENT BLOCK. NOTE: THIS WHOLE TEXT AFTER @} IS A COMMENT
```

Single line commands not associated with a block look like
```
@>  $COMMAND
```
Where command is like an error or warning or setting a key-value on an item.


## OPEN File Blocks
Before rendering output, a file block needs to begun. File blocks look like:
```
@{ open $Filename
...
@} end open $Filename
```
for instance
```
@{ open @(Name)Imp.cpp
 ...
@} end open @(Name)Imp.cpp

```
## IF Blocks

Moc IF control blocks are in actuality more like filtering expressions
being applied to the input data structures. They take boolean
expressions that are evaluated against some portion of the data structure
hierarchy.

The basic IF control block looks like 
```
@{         if $BOOLEAN_EXPRESSION
   ... output and other control lines here
@}         end if  $BOOLEAN_EXPRESSION
```

A more complicated one might look like:

```
@{         if $BOOLEAN1_EXPRESSION
   ... output and other control lines here
@|         else if $BOOLEAN2_EXPRESSION
   ... output and other control lines here
@|         else
   ... output and other control lines here
@}         end if  $BOOLEAN1_EXPRESSION
```

### Boolean Expression
The $BOOLEAN_EXPRESSION is made up of &&s and ||s of
basic expressions of the form
- exists $Name - true if there is an attribute or annotation
with the name $Name. e.g. exists MyKey 
- is-a $Name - true if the class or interface subclasses $Name or is $Name.
- is-from-current-module - true if the data structure was defined in the 
immediate file  given to Moc and not from a file included in that file. If
you have multiple input files, you must interate over the ALL_MODULES list.
- first $IteratorName - first and last take the name of the iterator are only useful 
  in a loop. First will be true if this is the first element in the loop being rendered.
  Useful to determine whether separators should be emitted in the rendered output.
- last $IteratorName - first and last take the name of the are only useful in a loop. 
  Last will be true if this is the last element in the loop being rendered. Useful to 
  determine whether separators should be emitted in the rendered output.


The NOT operator is '!'.


## Foreach Control Blocks 

Foreach control blocks apply a set of output lines and control structures
to each item in a data structure list. 

The basic foreach control block looks like 

```
@{         foreach $ITERATOR_NAME in $LIST
   ... output and other control lines here
@}         end foreach $ITERATOR_NAME in $LIST
```
The control and output lines are applied to each item in the list. What
lists of data structures are available is a function of the type of node
and whether it has any such sub-elements.

## SET Directive
The Set Directive can be used to annotate a data structure with a key-value
pair using the following:
```
@>    set  [$ITERATOR-NAME or $SCOPE::]$VariableName = $Value
```
e.g

```
@>    set  MyKey = "a long long road"
...
@>    set  Counter = @(Counter) + 1
...
@>    set  $iter-name::TagOnItem = " hi "
...
@>    set  $scope::TagOnItem = " hi "
```
Iterator names are specified in enclosing foreach blocks. Scopes are predefined. 
The available scopes are:

- file:: - this is a set of key-values associated with the current open files.
Opening a new file, starts off with no key-values.
- :: - this is the global scope.

```
@> set file::MyKey = "a value"
...
@> set ::GlobalKey = "another value"
```

## ERROR and WARNING Directives
You can cause a warning or error to be spit out when running Moc with the 
control directives:
```
@>   error  "A fatal thing happened."
...
@>   warning "Something unfortunated happened and should be looked at"
...
```

# Moc Template Output Lines
Output lines are rendered to the open file after having all variable
substituions being performed. Variable substitions may also be passed
thru a formatter to change their formating.

Any sequence @(<string>) represents a variable substitution and these
may appear in both output lines as well as in boolean expressions, set,
error and warn directives.

These subsitutions may be a formatted or unformatted variable substitution.
Both are explained below:

## Unformatted Variable Substitution

The unformatted variable substitution is of the form
```
   @( [<iterator/scope name>::] <key>)
```
Where key specifies the Name of the object, the Lineno, ...
The iterator/scope name is optional and is the name of an iterator of one 
of the enclosing foreach loops or a scope.

## Formatted Variable Substitution

The formatted substitution is an extension of the unformatted
variable substitution. Formatted substitution is similiar to 
unformatted except that before the ```[<variable name>::]<key>``` 
is a format specifier with a preceding ':' such as:

```
   @(:<format-specifier> <formatter arguments> )
```

The formatter arguments vary depending on the formatter that
is used. The standard formatters are:

- format-list - see below
- length (or len abbreviation) ```:length <list>```
- snake-case ```:snake-case <camelCase|PascalCase|snake_case_symbol>```
- camelCase ```:camelCase <camelCase|PascalCase|snake_case_symbol>```
- PascalCase ```:PascalCase <camelCase|PascalCase|snake_case_symbol>```
- uppercase-prefix (or upp abbreviation) ```:upp <word>```
- uppercase-all (or up) ```:up <word>```
- uppercase-first (or up1) ```:up1 <word>```
- lowercase-all (or low)   ```:low <word>```
- generate-sequence-number (or g)
- fill (or f)
- if - see below
- ? - ( format is ```:? <variable>:<value-if-variable-not-set> )```
- nth - the nth element in a list. format is ```:nth <index> <list> <variable>```

See existing templates for the arguments to these formatter functions.

@(:? Variable) or @(:? Variable:DefaultVauleIfMissing)

### format-list

Format-list works with foreach to format 1 or more lists 
optionally seperated by a prefix-separator
or a suffix-separator.
```
 a couple of lines to be rendered 
  and  @(:format-list : prefix-sep=','                   @\
        :foreach a in in-arguments                       @\
           :@(Type.TypeString) the@(Name):               @\
         foreach a in out-arguments                      @\
           :@(Type.TypeString) * the@(Name)) Rest the line
```

### uppercase & lowercase

The uppercase and lowercase convert either some or all
of the characters to upper or lower. up1 simply capitalizes
the first char.

#### uppercase-prefix

If the word starts with an uppercase letter, this will look for the
next uppercase letter following, and assume that this first uppercase
letter plus the immediately following lowercase letters are a prefix.
It will upper all of those and insert an _. This was used to assist the
naming conventions at the time.

```
  @(:uppercase-prefix PascalCaseWordList) 
  @(:uppercase-prefix camelCaseWordList) 
  @(:uppercase-prefix @(:up1 camelCaseWordList))
  @(:uppercase-prefix snakeCaseWordList) 
```
is rendered as 
```
    PASCAL_CaseWordList
    camelCaseWordList
    CAMEL_CaseWordList
    snake_case_word_list
```

#### uppercase-all (or up)
Changes the text to all upper case with underscores.
```
 @(:uppercase-all PascalCaseWordList) 
 @(:uppercase-all camelCaseWordList) 
 @(:uppercase-all snake_case_word_list)
```
is rendered as 
```
    PASCALCASEWORDLIST
    CAMELCASEWORDLIST
    SNAKE_CASE_WORD_LIST
```

#### uppercase-first (or up1)
```
  @(:up1 PascalCaseWordList)
  @(:up1 camelCaseWordList)
  @(:up1 snake_case_word_list)
```
is rendered as 
```
    PascalCaseWordList
    CamelCaseWordList
    Snake_case_word_list

```

#### lowercase-all

```
  @(:lowercase-all PascalCaseWordList)
  @(:lowercase-all camelCaseWordList)
  @(:lowercase-all snake_case_word_list)
```

```
   pascalcasewordlist
   camelcasewordlist
   snake_case_word_list
```

#### snake-case

```
  @(:snake-case PascalCaseWordList)
  @(:snake-case camelCaseWordList)
  @(:snake-case snake_case_word_list)
```

```
   pascal_case_word_list
   camel_case_word_list
   snake_case_word_list
```
#### camelCase

```
  @(:camelCase PascalCaseWordList)
  @(:camelCase camelCaseWordList)
  @(:camelCase snake_case_word_list)
```

```
   pascalCaseWordList
   camelCaseWordList
   snakeCaseWordList
```
#### PascalCase

```
  @(:PascalCase PascalCaseWordList)
  @(:PascalCase camelCaseWordList)
  @(:PascalCase snake_case_word_list)
```

```
   pascalcasewordlist
   camelcasewordlist
   snake_case_word_list
```

### if and ?

```
- @(:if $boolean_expression :then $output-stmt )
- @(:if $boolean_expression :then $output-stmt :else $output-stmt2 )
- @(:? VariableName)
- @(:? VariableName: $output-stmt-if-variable-not-exists)
```

### list related 
```
- @(:length $LIST) e.g. @(:length in-arguments)
- @(:nth $Index  $LIST $AttributeOrKeyName)
```

The length formatter returns the length of the list.

The nth formatter takes a 0-based index into the list and an attribute
of the nth node . The expression evaluates to the value of that attribute.
E.g if this is a list annotations and each annotation has an Name attribute and
a Value attribute, then

```
@(:nth 0 annotations Name )
```
This will grab the first item on the list of annotations if any. 
It will find the 'Name' attribute on that item and return the value of 
that attribute or grab the annotation on that item with the specified 
key name and return the annotation's value.


### misc

```
- @(:fill $COLUMN)
- @(:generate-sequence-number $SEQUENCE-NAME $ITEM ) (or g)
```

Fill simply inserts spaces until the $COLUMN on a line.

Generate sequence number is useful when you need a monotonically increasing
numerical sequence. If $ITEM has already been assigned an integer in the
sequence $SEQUENCE-NAME, that integer will be returned. Otherwise the next
available integer in the sequence will be allocated and assigned to that
$ITEM. 


# Global Attributes, Global Data Structures and Global Lists 

The top level lists are:

- ALL_MODULES - a list of all the *.moc files passed on the command line.
- ALL_CLASSES - a list of all classes and interfaces that have been parsed
- ALL_TYPES   - a list of all classes and interfaces that have been parsed
- ALL_CONSTANTS - a list of all constants that have been defined

Global attributes are attributes that don't belong to a particular data
structure. These are always available. The global attributes are:

- OUTPUT_FILENAME - the name of the current open file being rendered
- OUTPUT_LINENO   - the line number of the line currently being rendered
- OUTPUT_NEXT_LINENO   - the line number of the line immediately following the currently rendered line. Useful for emmitting C preprocessor #line statements
- MODULE - the name of the current input moc file/module

There are only 3 Global data structures - these are Annotations:

- HEADER_CODE - a top-level annotation that is passed unparsed from the moc input file to the output. Typically rendered at the top of a header file.
- HEADER_END_CODE - a top-level annotation that is passed unparsed from the moc input file to the output. Typically rendered at the bottom of a header file.
- SOURCE_CODE - a top-level annotation that is passed unparsed from the moc input file to the output. Typically rendered somewhere in a source file.

These have the following attributes
- Value
- Name or Key
- Filename
- Lineno

e.g.
```
@(HEADER_CODE.Value)
```

## Class, Interface  and Method Properties and Lists 

### Class/Interface Attributes
Classes and Interfaces have these attributes:
  @(Name)        - the name of the class
  @(TypeString)  - same as @(Name)
  @(TypeCode)    - a generated unique integer code that can be used to identify
  this class vs other classes, methods or interfaces. This is use by some
  RPC style communication libraries.
  @(Filename)    - the input filename where this was declared.
  @(Lineno)      - the input lineno where this was declared.
  @(IsType)      - 0 for classes, interfaces and methods.
  @(IsClass)     - 1 for classes and interfaces
  @(IsEnum)      - 0 for classes, interfaces and methods.
  @(IsAggregate) - 1 for classes, interfaces and methods.
  @(IsTypeDef)   - 0 for classes, interfaces and methods.

#### Lists on Classes/Interfaces
They also have the following lists suitable to be passed to the foreach control:

- parents - a list of classes and interfaces.
- all-parents - the list of all classes and interfaces that this class or any super-class inherits from.
- properties - a list of properties on this class/interface.
- inherited-properties - a list of all properties including those inherited from other classes.
- methods - a list of methods on this class/interface.
- inherited-methods - a list of all methods including inherited ones.
- annotations - a list of key-value pairs on the class/interface

### Attributes of Properties 

Classes, Interfaces and Structs can have properties. That is a class has 
properties AKA fields. These have attributes associated with them.

Attributes of a Property:

- @(Name) - the name of the property. e.g. 'id'
- @(TypeString) - the type of the property. e.g. 'integer' or 'AStruct'
- @(TypeCode) - a unique integer identifying this property name
- @(Filename) - the input Moc file where this property was declared. eg. examples/sample/sample.moc
- @(Lineno) - the line number of the Moc file where this was declared: e.g 59
- @(ArrayLength) - if the property is an array, this is its length. Otherwise it doesn't exist. 

#### Lists on Properties
They also have the following lists suitable to be passed to the foreach control:
- annotations - a list of key-value pairs on this node



### Attributes of Methods

Classes, Interfaces can have methods. Methods can have attributes and lists.

Attributes of a Method:

- @(Name) - the name of the method. e.g. 'doSomethingFantastical'
- @(TypeCode) - a unique integer identifying this method
- @(Filename) - the input Moc file where this method was declared. eg. examples/sample/sample.moc
- @(Lineno) - the line number of the Moc file where this was declared: e.g 59
- @(ReturnType) - the return type of this method.

#### Lists on Methods
They also have the following lists suitable to be passed to the foreach control:
- annotations - a list of key-value pairs on this node
- in-arguments - a list of arguments that are passed into the method.
- error-arguments - a list of error arguments (exceptions) that can be thrown by the method.

### Attributes of Method Arguments

Method Arguments have the following attributes:

- @(Name) - the name of the method. e.g. 'arg1'
  @(Name)        - the name of the structure
  @(TypeString)  - same as @(Name)
  @(TypeCode)    - a generated unique integer code that can be used to identify
  this class vs other classes, methods or interfaces. This is use by some
  RPC style communication libraries.
  @(Filename)    - the input filename where this was declared.
  @(Lineno)      - the input lineno where this was declared.
  @(IsType)      - 0 for classes, interfaces and methods.
  @(IsClass)     - 1 for classes and interfaces, 0 otherwise.
  @(IsEnum)      - 1 for enums, 0 otherwise.
  @(IsAggregate) - 1 for classes, interfaces, structs and methods.
  @(IsTypeDef)   - 1 for typedefs, 0 otherwise.
  @(IsArg)       - 1 for method argumetns, 0 otherwise.
  @(IsMethod)    - 1 for methods, 0 otherwise.

  @(Type.Name)        - the name of the type
  @(Type.TypeString)  - the type of argument
  @(Type.Filename)    - the input filename where this was declared.
  @(Type.Lineno)      - the input lineno where this was declared.
  @(Type.IsType)      - 0 for classes, interfaces and methods.
  @(Type.IsClass)     - 1 for classes and interfaces, 0 otherwise.
  @(Type.IsEnum)      - 1 for enums, 0 otherwise.
  @(Type.IsAggregate) - 1 for classes, interfaces, structs and methods.
  @(Type.IsTypeDef)   - 1 for typedefs, 0 otherwise.
  @(Type.IsArg)       - 1 for method argumetns, 0 otherwise.
  @(Type.IsMethod)    - 1 for methods, 0 otherwise.


#### Lists on Method Arguments
They also have the following lists suitable to be passed to the foreach control:
- annotations - a list of key-value pairs on this node


## Types

Structs, opaque types, typedefs and enums are all considered types and have
the same set of attributes available and all can have a list of annotations
(key-value pairs).

### Type Attributes 
Structs, opaque types, typedefs and enums all have these attributes:

  @(Name)        - the name of the structure
  @(TypeString)  - same as @(Name)
  @(TypeCode)    - a generated unique integer code that can be used to identify
  this class vs other classes, methods or interfaces. This is use by some
  RPC style communication libraries.
  @(Filename)    - the input filename where this was declared.
  @(Lineno)      - the input lineno where this was declared.
  @(IsType)      - 0 for classes, interfaces and methods.
  @(IsClass)     - 1 for classes and interfaces, 0 otherwise.
  @(IsEnum)      - 1 for enums, 0 otherwise.
  @(IsAggregate) - 1 for classes, interfaces, structs and methods.
  @(IsTypeDef)   - 1 for typedefs, 0 otherwise.
  @(IsArg)       - 1 for method argumetns, 0 otherwise.
  @(IsMethod)    - 1 for methods, 0 otherwise.

#### Lists on types
Types also have a list of annotations:

- annotations - a list of key-value pairs on the class/interface


While structs have 1 other list:

- properties - a list of properties on the struct


Files:
- [Design](Design.md) - some comments about the implementation
- [Input Syntax](DataModel.md) - description of the input format
- [Template](Template.md) - description of the Moc output template


