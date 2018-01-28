%{

#include "ecss/scanner_aux.h"
#include "ecss/tokens.h"

#define POS_NEWLINE do { yyextra.newline();       } while (0)
#define POS_ADVANCE do { yyextra.advance(yyleng); } while (0)

%}

%option reentrant case-insensitive noyywrap yylineno

h               [0-9a-f]
nonascii        [\200-\377]
unicode         \\{h}{1,6}(\r\n|[ \t\r\n\f])?
escape          {unicode}|\\[^\r\n\f0-9a-f]
nmstart         [_a-z]|{nonascii}|{escape}
nmchar          [_a-z0-9-]|{nonascii}|{escape}
string1         \"([^\n\r\f\\"]|\\{nl}|{escape})*\"
string2         \'([^\n\r\f\\']|\\{nl}|{escape})*\'
invalid1        \"([^\n\r\f\\"]|\\{nl}|{escape})*
invalid2        \'([^\n\r\f\\']|\\{nl}|{escape})*

ident           -?{nmstart}{nmchar}*
name            {nmchar}+
num             [0-9]+|[0-9]*"."[0-9]+
string          {string1}|{string2}
invalid         {invalid1}|{invalid2}
uri             ([!#$%&*-~]|{nonascii}|{escape})*
s               [ \t]+
w               {s}?
nl              \n|\r\n|\r|\f

A               a|\\0{0,4}(41|61)(\r\n|[ \t\r\n\f])?
C               c|\\0{0,4}(43|63)(\r\n|[ \t\r\n\f])?
D               d|\\0{0,4}(44|64)(\r\n|[ \t\r\n\f])?
E               e|\\0{0,4}(45|65)(\r\n|[ \t\r\n\f])?
G               g|\\0{0,4}(47|67)(\r\n|[ \t\r\n\f])?|\\g
H               h|\\0{0,4}(48|68)(\r\n|[ \t\r\n\f])?|\\h
I               i|\\0{0,4}(49|69)(\r\n|[ \t\r\n\f])?|\\i
K               k|\\0{0,4}(4b|6b)(\r\n|[ \t\r\n\f])?|\\k
M               m|\\0{0,4}(4d|6d)(\r\n|[ \t\r\n\f])?|\\m
N               n|\\0{0,4}(4e|6e)(\r\n|[ \t\r\n\f])?|\\n
P               p|\\0{0,4}(50|70)(\r\n|[ \t\r\n\f])?|\\p
R               r|\\0{0,4}(52|72)(\r\n|[ \t\r\n\f])?|\\r
S               s|\\0{0,4}(53|73)(\r\n|[ \t\r\n\f])?|\\s
T               t|\\0{0,4}(54|74)(\r\n|[ \t\r\n\f])?|\\t
X               x|\\0{0,4}(58|78)(\r\n|[ \t\r\n\f])?|\\x
Z               z|\\0{0,4}(5a|7a)(\r\n|[ \t\r\n\f])?|\\z

%x st_comment
%x st_url
%%

<INITIAL>{
    "/*"            POS_ADVANCE; BEGIN(st_comment);
}
<st_comment>{
    "*/"            POS_ADVANCE; BEGIN(0);
    {nl}            POS_NEWLINE;
    [^*\n]+         POS_ADVANCE;
    "*"             POS_ADVANCE;
}

<INITIAL>{
    "url"           POS_ADVANCE; BEGIN(st_url); return TOKEN_URL_SYM;
}
<st_url>{
    "("             POS_ADVANCE; return TOKEN_LPAREN;
    {uri}           POS_ADVANCE; BEGIN(0); return TOKEN_URI;
    {string}        POS_ADVANCE; BEGIN(0); return TOKEN_STRING;
    {nl}            POS_NEWLINE;
    {s}             POS_ADVANCE;
    .               POS_ADVANCE; return TOKEN_CHARACTER;
}

{nl}                POS_NEWLINE; return TOKEN_S;
{s}                 POS_ADVANCE; return TOKEN_S;
"<!--"              POS_ADVANCE; return TOKEN_CDO;
"-->"               POS_ADVANCE; return TOKEN_CDC;
"="                 POS_ADVANCE; return TOKEN_EQUALS;
"~="                POS_ADVANCE; return TOKEN_INCLUDES;
"|="                POS_ADVANCE; return TOKEN_DASH_MATCH;
"^="                POS_ADVANCE; return TOKEN_PREFIX_MATCH;
"*="                POS_ADVANCE; return TOKEN_SUBSTRING_MATCH;
"$="                POS_ADVANCE; return TOKEN_SUFFIX_MATCH;
"@"                 POS_ADVANCE; return TOKEN_AT;
"{"                 POS_ADVANCE; return TOKEN_LBRACE;
"}"                 POS_ADVANCE; return TOKEN_RBRACE;
"["                 POS_ADVANCE; return TOKEN_LBRACKET;
"]"                 POS_ADVANCE; return TOKEN_RBRACKET;
"("                 POS_ADVANCE; return TOKEN_LPAREN;
")"                 POS_ADVANCE; return TOKEN_RPAREN;
"+"                 POS_ADVANCE; return TOKEN_PLUS;
"-"                 POS_ADVANCE; return TOKEN_MINUS;
"*"                 POS_ADVANCE; return TOKEN_STAR;
"/"                 POS_ADVANCE; return TOKEN_SLASH;
"_"                 POS_ADVANCE; return TOKEN_CONCAT;
">"                 POS_ADVANCE; return TOKEN_GREATER;
"<"                 POS_ADVANCE; return TOKEN_LESS;
","                 POS_ADVANCE; return TOKEN_COMMA;
":"                 POS_ADVANCE; return TOKEN_COLON;
"::"                POS_ADVANCE; return TOKEN_DOUBLE_COLON;
";"                 POS_ADVANCE; return TOKEN_SEMICOLON;
"~"                 POS_ADVANCE; return TOKEN_TILDE;
"readonly"          POS_ADVANCE; return TOKEN_READONLY;
{string}            POS_ADVANCE; return TOKEN_STRING;
{invalid}           POS_ADVANCE; return TOKEN_UNCLOSED_STRING;
"#"{name}           POS_ADVANCE; return TOKEN_HASH;
"."{ident}          POS_ADVANCE; return TOKEN_CLASS;
"$"{ident}          POS_ADVANCE; return TOKEN_VARIABLE;
"@theme"            POS_ADVANCE; return TOKEN_ADD_FLAVOR_SYM;
"@charset"          POS_ADVANCE; return TOKEN_CHARSET_SYM;
"@define-theme"     POS_ADVANCE; return TOKEN_FLAVOR_SYM;
"@import"           POS_ADVANCE; return TOKEN_IMPORT_SYM;
"@include"          POS_ADVANCE; return TOKEN_INCLUDE_SYM;
"@media"            POS_ADVANCE; return TOKEN_MEDIA_SYM;
"@page"             POS_ADVANCE; return TOKEN_PAGE_SYM;
"@pragma"           POS_ADVANCE; return TOKEN_PRAGMA_SYM;
"!"{s}*"important"  POS_ADVANCE; return TOKEN_IMPORTANT_SYM;
{ident}             POS_ADVANCE; return TOKEN_IDENT;
{num}{ident}        POS_ADVANCE; return TOKEN_DIMENSION;
{num}%              POS_ADVANCE; return TOKEN_PERCENTAGE;
{num}               POS_ADVANCE; return TOKEN_NUMBER;
.                   POS_ADVANCE; return TOKEN_CHARACTER;

%%
