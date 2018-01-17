%{
// Dummy parser for scanner project.

#include <cassert>

#include "lyutils.h"
#include "astree.h"

%}

%debug
%defines
%error-verbose
%token-table
%verbose

%initial-action {
   parser::root = new astree (TOK_ROOT, {0, 0, 0}, "");
}

%token TOK_VOID TOK_INT TOK_STRING TOK_INDEX
%token TOK_IF TOK_ELSE TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_NULL TOK_NEW TOK_ARRAY TOK_RETURNVOID
%token TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE 
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON

%token TOK_BLOCK TOK_CALL TOK_IFELSE TOK_INITDECL TOK_NEWSTRING
%token TOK_POS TOK_NEG TOK_NEWARRAY TOK_TYPEID TOK_FIELD
%token TOK_ORD TOK_CHR TOK_ROOT TOK_DECLID
%token TOK_FUNCTION TOK_PROTOTYPE TOK_PARAMLIST TOK_VARDECL

%right TOK_IF TOK_ELSE
%right '='
%left  TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%left  '+' '-'
%left  '*' '/' '%'
%right '!' TOK_NEW
%right TOK_POS TOK_NEG
%left  TOK_ARRAY TOK_FIELD TOK_FUNCTION

%start start

%%

start      : program                    { yyparse_astree = $1; }
           ;

program    : program structdef          { $$ = $1->adopt ($2); } 
           | program function           { $$ = $1->adopt ($2); }
           | program statement          { $$ = $1->adopt ($2); }
           | program error '}'          { $$ = $1; }
           | program error ';'          { $$ = $1; } 
           |                            { $$ = parser::root; }
           ;

structdef  : TOK_STRUCT TOK_IDENT '{' fielddecls '}' 
                                   { 
                                     $2->swap (TOK_TYPEID);
                                     $$ = $1->adopt ($2, $4);
                                     destroy ($3, $5);  
                                   }

           | TOK_STRUCT TOK_IDENT '{' '}'
                                   {
                                     $2->swap (TOK_TYPEID);
                                     $$ = $1->adopt ($2); 
                                     destroy ($3, $4); 
                                   }  
           ;

fielddecls : fielddecls fielddecl  { $$ = $1->adopt ($2); }
           | fielddecl             { $$ = $1; } 
           ; 

fielddecl  : basetype TOK_IDENT ';'  
                                   { 
                                     $2->swap (TOK_FIELD);
                                     $$ = $1->adopt ($2);
                                     destroy ($3); 
                                   }

           | basetype TOK_ARRAY TOK_IDENT ';'  
                                   { 
                                     $3->swap (TOK_FIELD);
                                     $$ = $2->adopt ($1, $3);
                                     destroy ($4); 
                                   }
           ; 

basetype   : TOK_VOID              { $$ = $1; }
           | TOK_INT               { $$ = $1; }
           | TOK_STRING            { $$ = $1; }
           | TOK_IDENT             { $$ = $1; }
           ;

function   : identdecl idendecls ')' block 
                                   { 
                                     $$ = parser::fun($1, $2, $4);
                                     destroy ($3); 
                                   }
           ; 

idendecls : idendecls ',' identdecl     
                                   {
                                     $$ = $1->adopt ($3); 
                                     destroy ($2); 
                                   }

           | '(' identdecl              
                                   { 
                                     $1->swap (TOK_PARAMLIST); 
                                     $$ = $1->adopt ($2); 
                                   }

           | '('                   { $$=$1->swap (TOK_PARAMLIST); }
           ; 

identdecl  : basetype TOK_IDENT           
                                   { 
                                     $2->swap (TOK_DECLID);
                                     $$ = $1->adopt ($2);
                                   }     

           | basetype TOK_ARRAY TOK_IDENT 
                                   { 
                                     $3->swap (TOK_DECLID);  
                                     $$ = $2->adopt ($1, $3);
                                   } 
           ; 

block      : '{' '}'                    
                                   {
                                     $$ = $1->swap(TOK_BLOCK); 
                                     destroy ($2);  
                                   }

           | statements '}'             
                                   { 
                                     $$ = $1; 
                                     destroy ($2); 
                                   }

           | ';'                   { $$ = $1; }
           ; 

statements : statements statement  { $$ = $1->adopt ($2); }
           | '{' statement           
                                   { 
                                     $1->swap (TOK_BLOCK);
                                     $$ = $1->adopt ($2);
                                   }
           ; 

statement  : block                 { $$ = $1; }
           | vardecl               { $$ = $1; }
           | while                 { $$ = $1; }
           | ifelse                { $$ = $1; }
           | return                { $$ = $1; }
           | expr ';'              
                                   { 
                                     $$ = $1;
                                     destroy ($2); 
                                   }
           ; 

vardecl    : identdecl '=' expr ';'  
                                   {
                                     $2->swap (TOK_VARDECL);
                                     $$ = $2->adopt ($1, $3);
                                     destroy ($4); 
                                   }
           ;

while      : TOK_WHILE '(' expr ')' statement   
                                   { 
                                     $$ = $1->adopt ($3, $5);
                                     destroy ($2, $4); 
                                   }
           ; 

ifelse     : TOK_IF '(' expr ')' statement  %prec TOK_ELSE 
                                   {
                                     $$ = $1->adopt ($3, $5);
                                     destroy ($2, $4); 
                                   }

           | TOK_IF '(' expr ')' statement TOK_ELSE statement
                                   { 
                                     $1->swap (TOK_IFELSE); 
                                     $1->adopt ($3); 
                                     $$ = $1->adopt ($5, $7);
                                     destroy ($2, $4); 
                                     destroy ($6); 
                                   }
           ; 

return     : TOK_RETURN ';'             
                                   { 
                                     $$=$1->swap(TOK_RETURNVOID); 
                                     destroy ($2); 
                                   }

           | TOK_RETURN expr ';'   
                                   { 
                                     $$ = $1->adopt ($2);
                                     destroy ($3);
                                   }
           ; 

exprs      : expr ',' exprs             
                                   {
                                     $$ = $1->adopt ($3); 
                                     destroy ($2);
                                   }

           | expr                  { $$ = $1; }
           ; 

expr       : expr '-' expr         { $$ = $2->adopt ($1, $3); }
           | expr '+' expr         { $$ = $2->adopt ($1, $3); }
           | expr '/' expr         { $$ = $2->adopt ($1, $3); }
           | expr '*' expr         { $$ = $2->adopt ($1, $3); }
           | expr '%' expr         { $$ = $2->adopt ($1, $3); }
           | expr '=' expr         { $$ = $2->adopt ($1, $3); }
           | expr TOK_EQ expr      { $$ = $2->adopt ($1, $3); }
           | expr TOK_NE expr      { $$ = $2->adopt ($1, $3); }
           | expr TOK_LT expr      { $$ = $2->adopt ($1, $3); }
           | expr TOK_LE expr      { $$ = $2->adopt ($1, $3); } 
           | expr TOK_GT expr      { $$ = $2->adopt ($1, $3); }
           | expr TOK_GE expr      { $$ = $2->adopt ($1, $3); }

           | '-' expr %prec TOK_NEG        
                                   {
                                     $1->swap (TOK_NEG); 
                                     $$ = $1->adopt ($2); 
                                   } 

           | '+' expr %prec TOK_POS        
                                   { 
                                     $1->swap (TOK_POS);
                                     $$ = $1->adopt ($2);
                                   }

           | '!' expr              { $$ = $1->adopt ($2); }
           | '(' expr ')'          
                                   {
                                     $$ = $2; 
                                     destroy ($1, $3);
                                   }
           | allocator             { $$ = $1; }
           | call                  { $$ = $1; }
           | variable              { $$ = $1; }
           | constant              { $$ = $1; }
           ; 

allocator  : TOK_NEW TOK_IDENT '(' ')'     
                                   { 
                                     $2->swap (TOK_TYPEID);
                                     $$ = $1->adopt ($2); 
                                     destroy ($3, $4); 
                                   }

           | TOK_NEW TOK_STRING '(' expr ')' 
                                   { 
                                     $1->swap (TOK_NEWSTRING); 
                                     $$ = $1->adopt ($2, $4);
                                     destroy ($3); 
                                   }

           | TOK_NEW basetype '[' expr ']'  
                                   { 
                                     $1->swap (TOK_NEWARRAY); 
                                     $$ = $1->adopt ($2, $4); 
                                     destroy ($5);
                                   }
           ;

call       : TOK_IDENT '(' ')'             
                                   { 
                                     $2->swap (TOK_CALL); 
                                     $$ = $2->adopt ($1); 
                                     destroy ($3); 
                                   }

           | TOK_IDENT '(' exprs ')'       
                                  { 
                                    $2->swap (TOK_CALL);
                                    $$ = $2->adopt ($1, $3); 
                                    destroy ($4);
                                  }
           ; 

variable   : TOK_IDENT            { $$ = $1; }
           | expr '[' expr ']'       
                                  {
                                    $2->swap (TOK_INDEX);
                                    $$ = $2->adopt ($1, $3); 
                                    destroy ($4); 
                                  }
           | expr '.' TOK_IDENT      
                                  { 
                                    $3->swap (TOK_FIELD); 
                                    $$ = $1->adopt ($3); 
                                    destroy ($2); 
                                  }
           ;

constant   : TOK_INTCON           { $$ = $1; }
           | TOK_CHARCON          { $$ = $1; }
           | TOK_STRINGCON        { $$ = $1; }
           | TOK_NULL             { $$ = $1; }
           ;

%%


const char *parser::get_tname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}


bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}


/*
static void* yycalloc (size_t size) {
   void* result = calloc (1, size);
   assert (result != nullptr);
   return result;
}
*/

