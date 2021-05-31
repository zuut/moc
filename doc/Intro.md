# MOC
The marked-up object compiler, or Moc, takes an input file describing a
data model in terms of structs, types, enums, constants, interfaces and
classes and makes this data model available to be rendered as various types
of output files.

It was based on the same ideas as IDL compilers or RPC generators that
take a description of data and generate code. The difference with MOC is
that most types of data can be marked up with annotations. Custom
output templates can be written that can query these data types and 
generate code or DDL or any text based output file. What is generated
can be controlled thru the use of annotations.


# Running

```
moc -T {template-file.tpl} [-o {outdirectory}] [-I{include-path1}...] {input-file.moc} [{input-file2.moc} ...]
```
Moc command line arguments are:

-  -T <template>: specifies the MO compiler template
-  -I <includedir>: specifies the include path 
-  -S <sequence log file>: specifies the log file for the generated sequences 
-  -V <variable>=<value>: specifies a global variable 
-  -d <directory separator>: a single character used to separate directories

# Notes

Moc uses the C preprocessor to include any header files and expand any 
macros required. Once passed thru the preprocessor, the compiler parses
the classes and other data types into memory. Refer to 
[Input Syntax](DataModel.md) for the allowed input syntax.

Next it will load the specified template. See the [Template](Template.md) doc
for how to write your own templates.

Files:
- [Design](Design.md) - some comments about the implementation
- [Input Syntax](DataModel.md) - description of the input format
- [Template](Template.md) - description of the Moc output template

