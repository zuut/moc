/*
 * See Readme, Notice and License files.
 */
%{
#include "MOCInternal.h"
#include "IFTokens.h"
#include <string.h>

#define ECHO

#ifdef __cplusplus
extern "C" {
#endif
static int              gPosition;


#define YY_DECL int mocIflex(void)
extern int yylex( void );

//#define isatty( x ) 0
static int mocIfGetc( char* buf );


#define YY_INPUT(buf,result,max_size)                   \
{                                                       \
    result = mocIfGetc( buf );                          \
} 

#define YY_USER_INIT                                    \
{                                                       \
    gParser = kIF_PARSER;                               \
    gPosition = 0;                                      \
    yyin = stdin;                                       \
    yyout = stderr;                                     \
}

static int
mocIfGetc( char* buf )
{
    int c;
    gParser = kIF_PARSER;
    {
        char safebuf[210];
        int len = sizeof(safebuf) - 10;
        if (gIfString != 0) {
            strncpy(safebuf, gIfString, len);
        } else {
            strncpy(safebuf, "(null)", len);
        }
        safebuf[len]='\0';
    }

    c = gIfString != 0 ?
          (gIfString[ gPosition ] != '\0' ? gIfString[gPosition++] : 0) : 0;
    buf[0] = (char) c;
    if ( buf[0] == '\0' )
    {
        buf[0] = EOF;
    }
    return (c == '\0' ? 0 : 1);
}

void
mocIfReset()
{
    yy_init = 0;
    gPosition = 0;
}

#ifdef __cplusplus
};
#endif

%}
ws                      [ \t\n\r]+
string                  \"[^"]*\"
symbolname              [a-z/A-Z_]+[a-zA-Z:/\.0-9_]*
integer                 [0-9]+
real                    [0-9]*\.[0-9]+
bool                    [TF]
punctuation             [[{}();=,!|&+-/\*%\^~><&\]]
comment                 \/\/.*$

%%
{ws}                    ;

var {
    yylval.reservedt = kIF_VAR;
    return yylval.reservedt;
    }

string-equal {
    yylval.reservedt = kIF_STREQ;
    return yylval.reservedt;
    }

exists {
    yylval.reservedt = kIF_EXISTS;
    return yylval.reservedt;
    }

empty {
    yylval.reservedt = kIF_EMPTY;
    return yylval.reservedt;
    }

is-from-current-module  {
    yylval.reservedt = kIF_IS_FROM_CURRENT_MODULE;
    return yylval.reservedt;
    }

is-a    {
    yylval.reservedt = kIF_IS_A;
    return yylval.reservedt;
    }

first   {
    yylval.reservedt = kIF_FIRST;
    return yylval.reservedt;
    }

last    {
    yylval.reservedt = kIF_LAST;
    return yylval.reservedt;
    }

{integer} {
    yylval.integert = atoi( yytext );
    return kIF_INTEGER;
    }

{punctuation} {
    return yytext[0];
    }

{symbolname} {
    yylval.stringt = ufStrDup(yytext);
    return kIF_VARIABLE;
    }

{string} {
    int l;
    yylval.stringt = ufStrDup(&yytext[1]);
    l = strlen( yylval.stringt );
    if ( l > 0 ) {
            yylval.stringt[ l - 1] = '\0';
    }
    return kIF_VARIABLE;
    }

.   {
    {
        MOCERROR2((errmsg, "IF Parser: can not interpret \"%s\"\n", yytext ));
        yyterminate();
    }
    }


