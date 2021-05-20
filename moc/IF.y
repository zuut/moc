
%{
#include "MOCInternal.h"
/*#define YYDEBUG 1*/
extern int gIfResult;
extern int yylex(void);
extern int yywrap(void);

%}

%union {
    int             reservedt;
    int             integert;
    char*           stringt;
};

%token <reservedt>       kIF_VAR
%token <reservedt>       kIF_STREQ
%token <reservedt>       kIF_EXISTS
%token <reservedt>       kIF_EMPTY
%token <reservedt>       kIF_IS_FROM_CURRENT_MODULE
%token <reservedt>       kIF_IS_A
%token <reservedt>       kIF_FIRST
%token <reservedt>       kIF_LAST
%token <integert>        kIF_INTEGER
%token <stringt>         kIF_VARIABLE

%type <integert>         expr
%type <integert>         basicExpr
%type <stringt>          inputBlock

%%

top
    : expr { gIfResult = ( $1 ? 1 : 0 ) ; }
    ;

expr
    : basicExpr { $$ = $1 ; }
    | expr '<' expr { $$ = ( $1 < $3 ? 1 : 0 ) ; }
    | expr '<' '=' expr { $$ = ( $1 <= $4 ? 1 : 0 ) ; }
    | expr '=' '=' expr { $$ = ( $1 == $4 ? 1 : 0 ) ; }
    | expr '!' '=' expr { $$ = ( $1 != $4 ? 1 : 0 ) ; }
    | expr '>' expr { $$ = ( $1 > $3 ? 1 : 0 ) ; }
    | expr '>' '=' expr { $$ = ( $1 >= $4 ? 1 : 0 ) ; }
    | expr '&' expr { $$ =  $1 & $3 ; }
    | expr '|' expr { $$ =  $1 | $3 ; }
    | expr '+' expr { $$ = $1 + $3 ; }
    | expr '-' expr { $$ = $1 - $3 ; }
    | expr '/' expr { $$ = $1 / $3 ; }
    | expr '*' expr { $$ = $1 * $3 ; }
    | expr '%' expr { $$ = $1 % $3 ; }
    | expr '^' expr { $$ = $1 ^ $3 ; }
    | expr '&' '&' expr { $$ = $1 && $4 ; }
    | expr '|' '|' expr { $$ = $1 || $4 ; }
    | '(' expr ')' { $$ = $2 ; }
    | '!' expr { $$ = $2 ? 0 : 1; }
    | '+' expr { $$ = $2 ; }
    | '-' expr { $$ = -1 * $2 ; }
    | '~' expr { $$ = ~ $2 ; }
    ;

basicExpr
    : inputBlock kIF_EXISTS kIF_VARIABLE
            { $$= IfEvalExists( $1, $3 ); }
    | inputBlock kIF_EMPTY  kIF_VARIABLE
            { $$= IfEvalEmpty( $1, $3 ); }
    | inputBlock kIF_IS_FROM_CURRENT_MODULE
            { $$= IfEvalIsFromCurrentModule( $1, 0 ); }
    | inputBlock kIF_IS_A kIF_VARIABLE
            { $$= IfEvalIsA( $1, $3 ); }
    | inputBlock kIF_FIRST kIF_VARIABLE
            { $$= IfEvalFirst( $1, $3 ); }
    | inputBlock kIF_LAST kIF_VARIABLE
            { $$= IfEvalLast( $1, $3 ); }
    | inputBlock kIF_STREQ kIF_VARIABLE  kIF_VARIABLE
            { $$= IfEvalStrequal( $1, $3, $4 ); }
    | kIF_INTEGER
            { $$= $1; }
    ;

inputBlock
    :  { $$ = ufStrDup(""); }
    | kIF_VAR kIF_VARIABLE { $$ = $2 ; }
    ;

%%

struct TokenString
{
    const char* fString;
    int fToken;
};

static struct TokenString gTokens[] =
{
    { " the \"var\" keyword ", kIF_VAR },
    { " the \"exists\" keyword ", kIF_EXISTS },
    { " the \"is-empty\" keyword ", kIF_EMPTY },
    { " the \"is-from-current-module\" keyword",
              kIF_IS_FROM_CURRENT_MODULE},
    { " the \"is-a\" keyword ", kIF_IS_A },
    { " the \"first\" keyword ", kIF_FIRST },
    { " the \"last\" keyword ", kIF_LAST },
    { " an integer with the value ", kIF_INTEGER },
    { " a string/symbol-name with the value ", kIF_VARIABLE },
    0
};

const char*
mocPrintIfToken( int token )
{
    struct TokenString* s;

    for (s = gTokens ; s->fString ; s++)
    {
        if (s->fToken == token)
        {
            return s->fString;
        }
    }
    return " {UNKNOWN IF TOKEN: either a punctuation character or "
            "some unknown token}";
}

extern YYSTYPE yylval;
static char lastTokenValue[100];
static void _mocPrintIfTokenValue( int token );

static void
_mocPrintIfTokenValue( int token ) 
{
    switch (token) {
    case kIF_INTEGER:
        sprintf( lastTokenValue, "%d", yylval.integert );
        break;
    case kIF_VARIABLE:
        {
            int i;
            for (i = 0 ; i < 80 &&
                 yylval.stringt[i] != '\0';
                 i++ ) {
                lastTokenValue[i] = yylval.stringt[i];
            }
            lastTokenValue[i] = '\0';
        }
        break;
    default:
        lastTokenValue[0] = '\0';
    }
}

const char*
mocPrintIfTokenValue( int token )
{
    return lastTokenValue;
}

int
yylex()
{
#if YYDEBUG
    mocIfdebug = 1;
    printf("yylex: getting token ...");
    gLastToken=mocIflex();
    printf("yylex: %d\n", gLastToken );
    _mocPrintIfTokenValue( gLastToken );
    printf("yylex: returning %d\n", gLastToken );
    return gLastToken;
#else
    gLastToken=mocIflex();
    _mocPrintIfTokenValue( gLastToken );
    return gLastToken;
#endif
}

int gIfResult           = 0;
const char* gIfString   = 0;

int
mocParseIf( const char* string, const char* file, int line )
{
    int exitOnError = gExitOnError;
    const char* oldName = gFilename;
    int oldLine = lineno;

    gFilename = file;
    lineno = line;

    gIfResult = 0;
    mocIfReset();

    gIfString = string;

    gExitOnError = 1;

    yyparse();

    gExitOnError = exitOnError;

    gFilename = oldName;
    lineno = oldLine;

    return gIfResult;
}

int
yywrap()
{
    int doWrap = 1;
    return doWrap;
}

