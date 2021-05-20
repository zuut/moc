/*
 * See Readme, Notice and License files.
 */

%{
#include "MOCInternal.h"
/*#define YYDEBUG 1*/
#ifdef __cplusplus
extern "C" {
#endif
#include <ctype.h>

extern int yylex(void);
extern int yywrap(void);

#ifdef __cplusplus
}
#endif
%}

%union {
    int             reservedt;
    int             integert;
    char*           stringt;
    Boolean         booleant;
    double          realt;
    UserCode*       uct;
    KeyValue        kvt;
};

%token <reservedt>       START_OF_TOKENS

%token <reservedt>       kINTEGERTYPE
%token <reservedt>       kSTRINGTYPE
%token <reservedt>       kBOOLEANTYPE
%token <reservedt>       kREALTYPE

%token <stringt>         kUSERTYPE
%token <stringt>         kSYMBOLNAME
%token <stringt>         kSTRING
%token <integert>        kINTEGER
%token <uct>             CODE
%token <booleant>        kBOOLEAN
%token <realt>           kREAL
%token <uct>             HEADER_CODE
%token <uct>             HEADER_END_CODE
%token <uct>             SOURCE_CODE
%token <kvt>             KEY_VALUE


%token <reservedt>       STRUCT
%token <reservedt>       TYPEDEF
%token <reservedt>       TYPE
%token <reservedt>       HEADER
%token <reservedt>       ENUM
%token <reservedt>       CLASSNAME
%token <reservedt>       KEY
%token <reservedt>       VALUE

%token <reservedt>       CLASS
%token <reservedt>       INTERFACE
%token <reservedt>       PUBLIC
%token <reservedt>       PROTECTED
%token <reservedt>       PRIVATE
%token <reservedt>       VIRTUAL
%token <reservedt>       OPERATOR
%token <reservedt>       CONSTANT
%token <reservedt>       METHOD
%token <reservedt>       THROWS

%token <reservedt>       END_OF_TOKENS

%type <stringt>          oneWordSymbolName
%type <uct>              stringvalue
%type <uct>              user_defined_return_type
%type <integert>         integervalue
%type <kvt>              key_value

%%

top
    : decl
    | top decl
    ;

decl
    : className opt_classParents opt_classFields classEnd
    | typeDecl
    | HEADER_CODE
       { SetHeaderCode( $1 );
         gX="expected ';' to follow header code block"; } ';'
    | HEADER_END_CODE
       { SetHeaderEndCode( $1 );
         gX="expected ';' to follow header code block"; } ';'
    | SOURCE_CODE { SetSourceCode( $1 );
         gX="expected ';' to follow source code block"; } ';'
    | error { YYACCEPT; }
    ;

typeDecl
    : typeDeclName ';'
    | typeDeclName '{' typeDeclFieldList { gX = "}"; }
      '}' { gX=";";}  ';'
    | { gX= "struct keyword"; } STRUCT
      { gX="symbolname for type";}  oneWordSymbolName
      { DeclareType( $4 ); gX="{"; } '{'
            opt_structFields
            { gX = "}"; }
      '}' { gX=";";}  ';'
    | { gX= "struct keyword"; } ENUM
      { gX="symbolname for type";}  oneWordSymbolName
      { DeclareType( $4 ); gX="{"; } '{'
            enumList
            { gX = "}"; }
      '}' { gX=";";}  ';'
    | TYPEDEF oneWordSymbolName oneWordSymbolName ';'
      { DeclareType( $3 ); SetTypeDef( $2 ) ;gX="";  }
    | CONSTANT oneWordSymbolName '=' integervalue ';'
      { DeclareConstant( $2, $4 ); gX="";  }
    ;

typeDeclName
    : { gX= "type keyword"; } TYPE
      { gX="symbolname for type";}  oneWordSymbolName
      { DeclareType( $4 ); gX="{"; }
;

typeDeclFieldList
    : typeDeclFieldList { gX="type field"; } typeDeclField
    |  { gX="type field"; } typeDeclField
    ;

typeDeclField
    : { gX="CLASSNAME"; } CLASSNAME
       { gX="CLASSNAME symbolname"; } oneWordSymbolName
       { gX = ";"; } ';' { SetTypeClassName( $4 ); }
    | { gX="header keyword";} HEADER
       { gX="header filename with no quotes";}
       stringvalue { gX="; to follow header code"; } ';'
       { SetTypeHeader( $4->fCode ); }
    | { gX="key keyword"; } key_value
       { gX="; to follow key-value code for a typename";} ';' { SetTypeKeyValue( $2.fKey, $2.fUserCode ); }
    | { gX="enum keyword"; } ENUM  { gX="@ for enum"; } '@' { gX="{ for enum"; } '{'
       { gX="enum list "; }
        enumList
       { gX="} to end enum";}
   '}' { gX="; to terminate enum list";} ';'
    | { gX="struct keyword"; } STRUCT { gX="{ for struct"; } '@'
        { gX="{ for struct";} '{'
        { gX=" struct list "; }
         opt_structFields
        { gX= "} to end struct"; }
   '}' { gX="; to terminate struct list"; } ';'
    | TYPEDEF oneWordSymbolName { gX="{ for typedef"; } '@'
     { gX="; to terminate typedef"; }  ';'
      { SetTypeDef( $2 ) ;gX="";  }
    ;

enumList 
    : enumEntry
    | enumList ',' enumEntry
    ;

enumEntry
    : oneWordSymbolName '=' integervalue
   { AddTypeEnumEntryPair( $1 ,$3 ); }
    | oneWordSymbolName { AddTypeEnumEntry( $1 ); }
    ;

opt_structFields
    :
    | structField
    | opt_structFields structField
    ;

structField
    : structFieldDecl
       { gX="; to end field declaration "; } ';'
    | { gX="key keyword"; } key_value
       { gX="; to follow key-value code for a struct";} ';' { SetTypeKeyValue( $2.fKey, $2.fUserCode ); }
    ;

structFieldDecl
    : basicStructFieldDecl opt_fieldModifiers
    | basicStructFieldDecl '[' integervalue ']' opt_fieldModifiers
      {
            AddFieldArrayIndex( $3 );
      }
    ;

basicStructFieldDecl
    : {gX="int for structField ";} kINTEGERTYPE
       {gX="int symbolname";} oneWordSymbolName
       { AddStructField( "INTEGER", $4 ); }
       {gX=";";}
    | {gX="bool for structField";} kBOOLEANTYPE
       {gX="boolean symbolname";}  oneWordSymbolName
       { AddStructField( "BOOLEAN", $4 ); }
       {gX=";";}
    | {gX="string for structField";} kSTRINGTYPE
       {gX="string symbolname";} oneWordSymbolName
       { AddStructField( "STRING",  $4 ); }
       {gX=";";}
    | {gX="real for structField";} kREALTYPE
       {gX="real symbolname";} oneWordSymbolName
       { AddStructField( "REAL",    $4 ); }
       {gX=";";}
    | {gX="type for structField";} kUSERTYPE
       {gX="name of structField";} oneWordSymbolName
       { AddStructField( $2, $4 ); }
       {gX=";";}
    | {gX="type for structField";} TYPE
       {gX="type symbolname";} oneWordSymbolName
       {gX="name of structField";} oneWordSymbolName
       { AddStructField( $4, $6 ); }
       {gX=";";}
    ;

className
    : {gX="class keyword;";} CLASS
       {gX="class symbolname";} oneWordSymbolName
       { DeclareClass( $4 ); gX=""; }
    | {gX="class keyword;";} INTERFACE
       {gX="class symbolname";} oneWordSymbolName
       { DeclareClass( $4 ); SetInterface(); gX=""; }
    ;

opt_classParents
    : {gX="{ to start class body";} '{'
    | {gX=": to start class parent list";} ':'
       classParents
       {gX="{ to start class body";} '{'
    ;

classParents
    : classParents {gX=", before next class parent";} ','
       {gX="class parent";} classParent
    | {gX="class parent";} classParent
    ;

classParent
    : {gX="public keyword";} PUBLIC
       {gX="parent name";} oneWordSymbolName
       { AddClassParent( $4 ); }
    | {gX="parent name"; } oneWordSymbolName
       { AddClassParent( $2 ); }
    ;

opt_classFields
    :
    | classField
    | opt_classFields classField
    ;

classEnd
    : { gX="} to end class declaration";} '}'
       { gX="; to end class declaration";} ';'
       { gX=""; }
    ;

classField
    : METHOD methodDecl
       { gX="; to end method declaration"; } ';'
    | fieldDecl
       { gX="; to end field declaration "; } ';'
    | { gX="key keyword"; } key_value
       { gX="; to follow key-value code for a class field";} ';' { SetClassKeyValue( $2.fKey, $2.fUserCode ); }
    ;

methodDecl
    : { gX="return int type";} kINTEGERTYPE
      {gX="method name to follow int return type";} oneWordSymbolName
      {
        DeclareOperation( $4 );
    SetOpKeyValue( "ReturnType", newUserCode("int") );
    gX="[[, key-value]+,]"; }  '(' opt_methodArguments ')'
      methodDeclEnd
    | { gX="return boolean type";} kBOOLEANTYPE
      {gX="method name to follow boolean return type";} oneWordSymbolName
      {
        DeclareOperation( $4 );
    SetOpKeyValue( "ReturnType", newUserCode("boolean") );
    gX="[[, key-value]+,]"; } '(' opt_methodArguments ')'
      methodDeclEnd
    | { gX="return String type";} kSTRINGTYPE
      {gX="method name to follow string return type";} oneWordSymbolName
      {
        DeclareOperation( $4 );
    SetOpKeyValue( "ReturnType", newUserCode("String") );
    gX="[[, key-value]+,]"; } '(' opt_methodArguments ')'
      methodDeclEnd
    | { gX="return real type";} kREALTYPE
      {gX="method name to follow real return type";} oneWordSymbolName
      {
        DeclareOperation( $4 );
    SetOpKeyValue( "ReturnType", newUserCode("double") );
    gX="[[, key-value]+,]"; } '(' opt_methodArguments ')'
      methodDeclEnd
    | { gX="return UserType";} user_defined_return_type
      {gX="method name to follow user declared return type";} oneWordSymbolName
      {
        DeclareOperation( $4 );
    SetOpKeyValue( "ReturnType", $2 );
    gX="[[, key-value]+,]"; } '(' opt_methodArguments ')'
      methodDeclEnd
    ;

user_defined_return_type 
    : kUSERTYPE '[' ']'
      { $$= newUserCode2($1, "_ARRAY"); }
    | kUSERTYPE '*'
      { $$= newUserCode2($1, "_PTR"); }
    | kUSERTYPE
      { $$= newUserCode($1); }
    | oneWordSymbolName '[' ']'
      { $$= newUserCode2($1, "_ARRAY"); }
    | oneWordSymbolName '*'
      { $$= newUserCode2($1, "_PTR"); }
    | oneWordSymbolName
      { $$= newUserCode($1); }
    ;

opt_methodArguments
    : {}
    | methodArguments {}
    ;

methodArguments
    : methodArguments {gX=",";} ',' {gX="argument";} methodArgument
    | {gX="last argument"; } methodArgument
    ;

methodArgument
	: methodArgumentNameAndType
    |  methodArgumentNameAndType {gX="last argument"; } methodArgumentModifiers
	;

methodArgumentModifiers
    : methodArgumentModifier
    | methodArgumentModifiers  methodArgumentModifier
    ;

methodArgumentModifier
    : { gX="key keyword"; } key_value
       { gX="[method argument key-value pair ,]*";} { SetOpArgKeyValue( $2.fKey, $2.fUserCode ); }
    ;
methodArgumentNameAndType
    : {gX="user defined type";} kUSERTYPE
       {gX="parameter name";} oneWordSymbolName
   { AddInArgument( $2, $4 ); }
    | {gX="type";} TYPE
       {gX="type symbolname";} oneWordSymbolName
       {gX="parameter name";} oneWordSymbolName
       { AddInArgument( $4, $6 ); }
    | {gX="int argument";} kINTEGERTYPE
       {gX="parameter name";} oneWordSymbolName
  { AddInArgument( "INTEGER", $4 ); }
    | {gX="string argument";} kSTRINGTYPE
       {gX="parameter name";} oneWordSymbolName
       { AddInArgument( "STRING", $4 ); }
    | {gX="bool argument";} kBOOLEANTYPE
       {gX="parameter name";} oneWordSymbolName
       { AddInArgument( "BOOLEAN", $4 ); }
    | {gX="real argument";} kREALTYPE
       {gX="parameter name";} oneWordSymbolName
       { AddInArgument( "REAL", $4 ); }
    ;

methodDeclEnd
	: methodModifiers {}
	| THROWS methodExceptions {}
	| methodModifiers THROWS methodExceptions {}
	| {}
    ;

methodModifiers
    : methodModifier
    | methodModifiers ',' methodModifier
    ;

methodModifier
    : { gX="key keyword"; } key_value
       { gX="[method key-value pair ,]* [throws [exceptions]+]*;";} { SetOpKeyValue( $2.fKey, $2.fUserCode ); }
    ;

methodExceptions
    : methodException
    | methodExceptions ',' methodException
    ;

methodException
    : { gX="exception"; } oneWordSymbolName
       {
            gX="[throws exception];";
            AddErrorArgument( $2, $2 );
       }
    ;

fieldDecl
    : basicFieldDecl opt_fieldModifiers
    | basicFieldDecl '[' integervalue ']' opt_fieldModifiers
      {
            AddFieldArrayIndex( $3 );
      }
    ;

basicFieldDecl
    : {gX="int for field ";} kINTEGERTYPE
       {gX="int symbolname";} oneWordSymbolName
       { AddField( "INTEGER", $4 ); }
       {gX=";";}
    | {gX="bool for field";} kBOOLEANTYPE
       {gX="boolean symbolname";}  oneWordSymbolName
       { AddField( "BOOLEAN", $4 ); }
       {gX=";";}
    | {gX="string for field";} kSTRINGTYPE
       {gX="string symbolname";} oneWordSymbolName
       { AddField( "STRING",  $4 ); }
       {gX=";";}
    | {gX="real for field";} kREALTYPE
       {gX="real symbolname";} oneWordSymbolName
       { AddField( "REAL",    $4 ); }
       {gX=";";}
    | {gX="type for field";} kUSERTYPE
       {gX="name of field";} oneWordSymbolName
       { AddField( $2, $4 ); }
       {gX=";";}
    | {gX="type for field";} TYPE
       {gX="type symbolname";} oneWordSymbolName
       {gX="name of field";} oneWordSymbolName
       { AddField( $4, $6 ); }
       {gX=";";}
    ;

opt_fieldModifiers
    :
    | ',' fieldModifiers
    ;

fieldModifiers
    : fieldModifier
    | fieldModifiers ',' fieldModifier
    ;

fieldModifier
    : { gX="key keyword"; } key_value
       { gX="[, field key-value pair ]* ;";} { SetFieldKeyValue( $2.fKey, $2.fUserCode ); }
    ;

key_value
    : KEY_VALUE { $$ = $1; }
    | { gX="key keyword"; } KEY
       { gX="symbol name";} oneWordSymbolName
       { gX="value keyword"; } VALUE
       { gX="value string";} stringvalue
       {
            $$.fUserCode = $8;
            strncpy($$.fKey, $4, sizeof($$.fKey)/sizeof($$.fKey[0]));
       }
    ;

stringvalue
    : kSTRING { $$= CreateUserCode($1);}
    | kSYMBOLNAME { $$= CreateUserCode($1);}
    | kUSERTYPE { $$= CreateUserCode($1);}
    | CODE { $$= $1;}
    ;

integervalue
    : kINTEGER { $$= $1;}
    | integervalue '+' integervalue { $$ = $1 + $3; }
    | integervalue '-' integervalue { $$ = $1 - $3; }
    | integervalue '*' integervalue { $$ = $1 * $3; }
    | integervalue '/' integervalue { $$ = $1 / $3; }
    | integervalue '|' integervalue { $$ = $1 | $3; }
    | integervalue '&' integervalue { $$ = $1 & $3; }
    | integervalue '|' '|' integervalue { $$ = $1 || $4; }
    | integervalue '&' '&' integervalue { $$ = $1 && $4; }
    | integervalue '%' integervalue { $$ = $1 % $3; }
    | integervalue '<' '<' integervalue { $$ = $1 << $4; }
    | integervalue '>' '>' integervalue { $$ = $1 >> $4; }
    | '(' integervalue ')' { $$ = $2; }
    ;

oneWordSymbolName
    : kUSERTYPE { $$=$1;}
    | kSYMBOLNAME { $$=$1;}
    ;
%%

struct TokenString
{
    const char* fString;
    int fToken;
};

static struct TokenString gTokens[] =
{
    { "START_OF_TOKENS", START_OF_TOKENS },
    { " the \"integer\" keyword", kINTEGERTYPE },
    { " the \"string\" keyword", kSTRINGTYPE },
    { " the \"boolean\" keyword", kBOOLEANTYPE },
    { " the \"real\" keyword", kREALTYPE },
    { " the user defined type", kUSERTYPE },
    { " a symbolname (i.e. not a class or type and so must be a \
variable name). It has the value ", kSYMBOLNAME },
    { " the quoted string ", kSTRING },
    { " an integer with the value ", kINTEGER },
    { " a boolean with the value ", kBOOLEAN },
    { " a real number with the value ", kREAL },
    { " a key-value pair ", KEY_VALUE },
    { " a free-form code fragment containing ", CODE },
    { " an initial header code fragment containing ",
              HEADER_CODE },
    { " a terminating header code fragment containing ",
              HEADER_END_CODE },
    { " a source code fragment containing ", SOURCE_CODE },
    { " the \"type\" keyword", TYPE },
    { " the \"Header\" keyword", HEADER },
    { " the \"classname\" keyword used for types", CLASSNAME },
    { " the \"key\" keyword", KEY },
    { " the \"value\" keyword", VALUE },
    { " the \"class\" keyword", CLASS },
    { " the \"interface\" keyword ", INTERFACE },
    { " the \"public\" keyword", PUBLIC },
    { " the \"protected\" keyword", PROTECTED },
    { " the \"private\" keyword ", PRIVATE },
    { " the \"virtual\" keyword ", VIRTUAL },
    { " the \"operator\" keyword", OPERATOR },
    { "END_OF_TOKENS", END_OF_TOKENS  },
    0
};

const char*
mocPrintInputToken( int token )
{
    struct TokenString* s;

    for (s = gTokens ; s->fString ; s++)
    {
        if (s->fToken == token)
        {
            return s->fString;
        }
    }
    if (isascii(token)) {
        return " ASCII character";
    }
    return " {UNKNOWN MOC TOKEN} ";
}

extern YYSTYPE yylval;
static char lastTokenValue[100];
const int LastTokenValueBufSize = sizeof(lastTokenValue)/sizeof(lastTokenValue[0]) - 4; // leave room for ellipsis
static void _mocPrintInputTokenValue( int token );

const char*
mocPrintInputTokenValue( int token )
{
    return lastTokenValue;
}

static void
_mocPrintInputTokenValue( int token )
{
    switch (token)
    {
    case kUSERTYPE:
    case kSYMBOLNAME:
    case kSTRING:
        strncpy(lastTokenValue, yylval.stringt, LastTokenValueBufSize);
        strcpy(lastTokenValue + LastTokenValueBufSize, "...");
        break;
    case kREAL:
        sprintf( lastTokenValue, "%f", yylval.realt );
        break;
    case kINTEGER:
    case kBOOLEAN:
        sprintf( lastTokenValue, "%d", yylval.integert );
        break;
    case KEY_VALUE:
        strncpy(lastTokenValue, yylval.kvt.fKey, LastTokenValueBufSize);
        strncat(lastTokenValue, "=", LastTokenValueBufSize);
        strncat(lastTokenValue, yylval.kvt.fUserCode->fCode, LastTokenValueBufSize);
        strcpy(lastTokenValue + LastTokenValueBufSize, "...");
        break;
    case CODE:
    case HEADER_CODE:
    case HEADER_END_CODE:
    case SOURCE_CODE:
        strncpy(lastTokenValue, yylval.uct->fCode, LastTokenValueBufSize);
        strcpy(lastTokenValue + LastTokenValueBufSize, "...");
        break;
    default:
        if (isascii(token)) {
            lastTokenValue[0] = (char) token;
            lastTokenValue[1] = '\0';
        } else {
            lastTokenValue[0] = '\0';
        }
    }
}

int
yylex()
{
#if 0
    printf("READ token");
    int token = mocInputlex();
    _mocPrintInputTokenValue( token );
    printf("%s %d ", mocPrintInputToken(token), token);
    if (isascii(token)) {
        printf("('%c') ", token);
    }
    printf("value='%s'\n", mocPrintInputTokenValue(token));
    return (gLastToken=token);
#else
    gLastToken=mocInputlex();
    _mocPrintInputTokenValue( gLastToken );
    return gLastToken;
#endif
}

int
yywrap()
{
    return 1;
}

FILE* gInputFile = 0;

void
mocParseInput( FILE* in, const char* filename )
{
    gInputFile = in;
    mocParseReset();
    gFilename = filename ? filename : "<stdin>" ;
    yyparse();

    gInputFile = 0;
    gFilename = "???";
}

