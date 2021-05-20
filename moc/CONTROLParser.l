/*
 * See Readme, Notice and License files.
 */
%{
#include "MOCInternal.h"
#include "CONTROLTokens.h"
#include <string.h>

#define ECHO

#ifdef __cplusplus
extern "C" {
#endif
static int              gPosition;


extern int yylex( void );

//#define isatty( x ) 0
static int mocControlGetc( char* buf );

#define YY_INPUT(buf,result,max_size)                   \
{                                                       \
    result = mocControlGetc( buf );                     \
} 

#define YY_USER_INIT                                    \
{                                                       \
    gParser = kCONTROL_PARSER;                          \
    gPosition = 0;                                      \
    yyin = stdin;                                       \
    yyout = stderr;                                     \
}

static int
mocControlGetc( char* buf )
{
    int c;
    gParser = kCONTROL_PARSER;
    c = gControlString[ gPosition ] != '\0' ?
            gControlString[gPosition++] : '\0';
    buf[0] = (char) c;
    if ( buf[0] == '\0' )
    {
        buf[0] = EOF;
    }
    return (c == '\0' ? 0 : 1);
}

void
mocControlReset()
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
symbolname              [:a-z/A-Z_]+[-a-zA-Z:/\.0-9_]*
integer                 [0-9]+
real                    [0-9]*\.[0-9]+
bool                    [TF]
punctuation             [[{}();=,!|&+-/\*%\^~\]]
comment                 \/\/.*$

%%
{ws}                    ;

set {
    yylval.reserved = kCONTROL_SET;
    return yylval.reserved;
    }

{integer} {
    yylval.integer = atoi( yytext );
    return kCONTROL_INTEGER;
    }

{punctuation} {
    return yytext[0];
    }

{symbolname} {
    yylval.string = ufStrDup(yytext);
    return kCONTROL_VARIABLE;
    }

{string} {
    int l;
    yylval.string = ufStrDup(&yytext[1]);
    l = strlen( yylval.string );
    if ( l > 0 )
    {
        yylval.string[ l - 1] = '\0';
    }
    return kCONTROL_VARIABLE;
    }

.   {
        MOCERROR2((errmsg, "CONTROL Parser: can not interpret \"%s\" at character position %d of (%ld) in the output template\n", yytext, gPosition, strlen(gControlString)));
        yyterminate();
    }


