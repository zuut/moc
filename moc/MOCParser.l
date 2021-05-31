/*
 * See Readme, Notice and License files.
 */
%{
#include "MOCInternal.h"
#include "MOCTokens.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int yylex( void );

#define ECHO
//#define isatty( x ) 0

#define YY_INPUT(buf,result,max_size)                   \
{                                                       \
    int c;                                              \
    gParser = kINPUT_PARSER;                            \
    c = mocGetc( yyin );                                \
    result = (c == EOF ? 0 : 1);                        \
    buf[0] = (char) c;                                  \
} 

#define YY_USER_INIT                                    \
{                                                       \
    gParser = kINPUT_PARSER;                            \
    yyin = gInputFile;                                  \
    yyout = stderr;                                     \
}

void mocParseReset() {
    yy_flush_buffer(YY_CURRENT_BUFFER);
    yyin = gInputFile;
}

#ifdef __cplusplus
};
#endif

%}
ws                      [ \t\n\r]+
string                  \"[^"]*\"
key_value               \[\[[ \t]*[a-z/A-Z_]+[a-zA-Z/\.0-9_]*
symbolname              [a-z/A-Z_]+[a-zA-Z/\.0-9_]*
hex                             0x[0-9a-fA-F]+
integer                 [-]?[0-9]+
real                    [0-9]*\.[0-9]+
bool                    [TF]
punctuation             [[{}()@;:,=\+\*\-/\]|&><%]
comment                 \/\/.*$

%%
{ws}                    ;

string {
    yylval.reservedt = kSTRINGTYPE;
    return yylval.reservedt;
    }

integer {
    yylval.reservedt = kINTEGERTYPE;
    return yylval.reservedt;
    }

boolean {
    yylval.reservedt = kBOOLEANTYPE;
    return yylval.reservedt;
    }

real    {
    yylval.reservedt = kREALTYPE;
    return yylval.reservedt;
    }

code    {
    yylval.uct = ReadCodeString( yyin );
    yy_flush_buffer(YY_CURRENT_BUFFER);
    return CODE;
    }


class   {
    yylval.reservedt = CLASS;
    return yylval.reservedt;
    }

interface       {
    yylval.reservedt = INTERFACE;
    return yylval.reservedt;
    }

public  {
    yylval.reservedt = PUBLIC;
    return yylval.reservedt;
    }

protected {
    yylval.reservedt = PROTECTED;
    return yylval.reservedt;
    }

private {
    yylval.reservedt = PRIVATE;
    return yylval.reservedt;
    }

header_code {
    yylval.uct = ReadCodeString( yyin );
    yy_flush_buffer(YY_CURRENT_BUFFER);
    return HEADER_CODE;
    }

header_end_code {
    yylval.uct = ReadCodeString( yyin );
    yy_flush_buffer(YY_CURRENT_BUFFER);
    return HEADER_END_CODE;
    }

source_code {
    yylval.uct = ReadCodeString( yyin );
    yy_flush_buffer(YY_CURRENT_BUFFER);
    return SOURCE_CODE;
    }

method   {
    yylval.reservedt = METHOD;
    return yylval.reservedt;
    }

throws   {
    yylval.reservedt = THROWS;
    return yylval.reservedt;
    }

struct {
    yylval.reservedt = STRUCT;
    return yylval.reservedt;
    }

typedef {
    yylval.reservedt = TYPEDEF;
    return yylval.reservedt;
    }

constant {
    yylval.reservedt = CONSTANT;
    return yylval.reservedt;
    }

type    {
    yylval.reservedt = TYPE;
    return yylval.reservedt;
    }

enum    {
    yylval.reservedt = ENUM;
    return yylval.reservedt;
    }

Header {
    yylval.reservedt = HEADER;
    return yylval.reservedt;
    }

key     {
    yylval.reservedt = KEY;
    return yylval.reservedt;
    }

value   {
    yylval.reservedt = VALUE;
    return yylval.reservedt;
    }

{key_value}   {
    if (ReadKeyValue(&yylval.kvt, yytext, yyin) != 0) {
        yy_flush_buffer(YY_CURRENT_BUFFER);
        return KEY_VALUE;
    }
    REJECT;
    }

{string} {
    yylval.stringt = yytext[0] == '"' ? ufStrDup(&yytext[1]) :
            ufStrDup(yytext) ;
    {
        int len = strlen( yylval.stringt );
        if (len > 0 && yylval.stringt[len-1]=='"' )
        {
            yylval.stringt[len-1] = '\0';
        }
    }
    return kSTRING;
    }

{hex} {
    yylval.integert = strtol( yytext, 0, 16 );
    return kINTEGER;
    }

{integer} {
    yylval.integert = atoi( yytext );
    return kINTEGER;
    }

{real} {
    sscanf( yytext, "%lf", & yylval.realt );
    return kREAL;
    }

{punctuation} {
    return yytext[0];
    }

{symbolname} {
    if ( mocIsTypeDefined( yytext ) ) {
        yylval.stringt = ufStrDup(yytext);
        return kUSERTYPE;
    }
    if ( mocIsClassDefined( yytext ) ) {
        yylval.stringt = ufStrDup(yytext);
        return kUSERTYPE;
    }
    if ( mocIsConstant( yytext ) ) {
        yylval.integert = mocGetConstant( yytext );
        return kINTEGER;
    }
    yylval.stringt = ufStrDup(yytext);
    return kSYMBOLNAME;
    }

{comment} ;

.   {
    {
        MOCERROR((errmsg, "MOC Parser: can not interpret \"%s\"\n",
                  yytext ));
    }
    }


