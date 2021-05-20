
%{
#include "MOCInternal.h"
/*#define YYDEBUG 1*/
extern int gControlResult;
extern int yylex(void);
extern int yywrap(void);

%}

%union {
    int             reserved;
    int             integer;
    char*           string;
};

%token <reserved>       kCONTROL_SET
%token <integer>        kCONTROL_INTEGER
%token <string>         kCONTROL_VARIABLE

%type <integer>         expr
%type <integer>         basicExpr

%%

top
    : set_stmt
    ;

set_stmt
	: kCONTROL_SET kCONTROL_VARIABLE '=' expr
     {
		ControlEvalSetInteger( $2, $4 );
     }
	| kCONTROL_SET kCONTROL_VARIABLE '=' kCONTROL_VARIABLE
     {
		ControlEvalSetString( $2, $4 );
     }
	;
expr
    : basicExpr { $$ = $1 ; }
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
    : kCONTROL_INTEGER
     { $$ = $1 ; }
    ;

%%

struct TokenString
{
    const char* fString;
    int fToken;
};

static struct TokenString gTokens[] =
{
    { " the \"set\" keyword ", kCONTROL_SET },
    { " an integer with the value ", kCONTROL_INTEGER },
    { " a string/symbol-name with the value ", kCONTROL_VARIABLE },
    0
};

const char*
mocPrintControlToken( int token )
{
    struct TokenString* s;

    for (s = gTokens ; s->fString ; s++)
    {
        if (s->fToken == token)
        {
            return s->fString;
        }
    }
    return " {UNOWN CONTROL TOKEN: either a punctuation character or "
            "some unknown letter} ";
}

extern YYSTYPE yylval;
static char lastTokenValue[100];
static void _mocPrintControlTokenValue( int token );

static void
_mocPrintControlTokenValue( int token ) 
{
    switch (token) {
    case kCONTROL_INTEGER:
        sprintf( lastTokenValue, "%d", yylval.integer );
        break;
    case kCONTROL_VARIABLE:
        {
            int i;
            for (i = 0 ; i < 80 &&
                 yylval.string[i] != '\0';
                 i++ ) {
                lastTokenValue[i] = yylval.string[i];
            }
            lastTokenValue[i] = '\0';
        }
        break;
    default:
        lastTokenValue[0] = '\0';
    }
}

const char*
mocPrintControlTokenValue( int token )
{
    return lastTokenValue;
}

int
yylex()
{
#if 0
    printf("getting token ...");
    int token = mocControllex();
    printf("%d\n", token );
    return (gLastToken=token);
#else
    gLastToken=mocControllex();
    _mocPrintControlTokenValue( gLastToken );
    return gLastToken;
#endif
}

int gControlResult           = 0;
const char* gControlString   = 0;

int
mocParseControl( const char* string, const char* file, int line )
{
    int exitOnError = gExitOnError;
    const char* oldName = gFilename;
    int oldLine = lineno;

    gFilename = file;
    lineno = line;

    gControlResult = 0;
    mocControlReset();

    gControlString = string;

    gExitOnError = 1;
    yyparse();
    gExitOnError = exitOnError;

    gFilename = oldName;
    lineno = oldLine;

    return gControlResult;
}

int
yywrap()
{
    int doWrap = 1;
    return doWrap;
}

