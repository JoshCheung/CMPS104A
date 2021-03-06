/* $Id: scanner.l,v 1.1 2017-10-30 01:03:20-07 - - $ */

%{

#include "lyutils.h"

#define YY_USER_ACTION  { lexer::advance(); }

int yylval_token (int symbol) {
   yylval = new astree (symbol, lexer::lloc, yytext);
   fprintf (tok_outfile, 
      "   %lu %lu.%-2lu  %-4d %-10s (%s)\n", 
      lexer::lloc.filenr, 
      lexer::lloc.linenr, 
      lexer::lloc.offset, 
                  symbol,
      parser::get_tname(symbol), 
      yytext);
   return symbol;
}

%}


%option 8bit
%option debug
%option nodefault
%option nounput
%option noyywrap
%option verbose
%option warn


LETTER          [a-zA-Z_]
DIGIT           [0-9]
MANTISSA        ({DIGIT}+\.?{DIGIT}*|\.{DIGIT}+)
EXPONENT        ([Ee][+-]?{DIGIT}+)
NUMBER          ({MANTISSA}{EXPONENT}?)
NOTNUMBER       ({MANTISSA}[Ee][+-]?)
IDENT           ({LETTER}({LETTER}|{DIGIT})*)
CHAR            ('([^\\'\n]|\\[\\'"0nt])')
STRING          (\"([^\\"\n]|\\[\\'"0nt])*\")

%%

"#".*           { lexer::include();
                  fprintf(tok_outfile, "%s\n", yytext);    
                }

[ \t]+          { }
\n              { lexer::newline(); }

"if"            { return yylval_token (TOK_IF);}
"else"          { return yylval_token (TOK_ELSE);}
"while"         { return yylval_token (TOK_WHILE);}
"return"        { return yylval_token (TOK_RETURN); }
"int"           { return yylval_token (TOK_INT); }
"string"        { return yylval_token (TOK_STRING);}
"struct"        { return yylval_token (TOK_STRUCT);}
"void"          { return yylval_token (TOK_VOID);}
"new"           { return yylval_token (TOK_NEW);}
"null"          { return yylval_token (TOK_NULL);}


{CHAR}          { return yylval_token (TOK_CHARCON); }
{STRING}        { return yylval_token (TOK_STRINGCON); }
{NUMBER}        { return yylval_token (NUMBER); }
{IDENT}         { return yylval_token (TOK_IDENT); }
"="             { return yylval_token ('='); }
"+"             { return yylval_token ('+'); }
"-"             { return yylval_token ('-'); }
"*"             { return yylval_token ('*'); }
"/"             { return yylval_token ('/'); }
"^"             { return yylval_token ('^'); }
"("             { return yylval_token ('('); }
")"             { return yylval_token (')'); }
";"             { return yylval_token (';'); }
"*"             { return yylval_token ('*'); }
"^"             { return yylval_token ('^'); }
"!"             { return yylval_token ('!'); }
"%"             { return yylval_token ('%'); }
"."             { return yylval_token ('.'); }
"["             { return yylval_token ('['); }
"]"             { return yylval_token (']'); }
"{"             { return yylval_token ('{'); }
"}"             { return yylval_token ('}'); }
","             { return yylval_token (','); }

"[]"            { return (TOK_ARRAY); }
"=="            { return (TOK_EQ); }
"!="            { return (TOK_NE); }
"<="            { return (TOK_LE); }
">="            { return (TOK_GE); }
"<"             { return (TOK_LT); }
">"             { return (TOK_GT); }


{NOTNUMBER}     { lexer::badtoken (yytext);
                  return yylval_token (NUMBER); }
.               { lexer::badchar (*yytext); }

%%

