%{
#include "common.h"
#include "lex.h"
#include "parse.h"

namespace {

void yyerror(const location* loc, ParseObserver& observer, const char* msg)
{
  observer.OnSyntaxError(*loc, msg);
}

} // namespace

%}

%code requires {
#include "common.h"
}

%locations

%define api.location.type {location}

%define api.value.type {semantic_value}

%define api.pure full

%parse-param {ParseObserver& observer}

%define parse.error verbose

%left '+' '-'
%left '*' '/'

%token END 0 "end of file"

%token<as_int> INT_LITERAL "int literal"

%token<as_float> FLOAT_LITERAL "float literal"

%token<as_string> IDENTIFIER "identifier"

%token RETURN "return"

%token INVALID_CHAR "invalid character"

%token<asTypeID> TYPE_NAME "type name"

%token<asVariability> VARIABILITY "variability"

%type <asType> type "type"

%type <as_expr> primary_expr
%type <as_expr> unary_expr
%type <as_expr> postfix_expr
%type <as_expr> multiplicative_expr
%type <as_expr> additive_expr
%type <as_expr> expr

%type <as_expr_list> expr_list

%type <as_var> param_decl

%type <as_param_list> param_list

%type <as_stmt_list> stmt_list

%type <as_stmt> stmt

%type <as_stmt> assignment_stmt

%type <as_stmt> return_stmt

%type <as_stmt> compound_stmt

%type <as_stmt> decl_stmt

%type <as_var> var_decl

%type <as_func> func

%type <asProgram> program

%destructor { delete $$; } IDENTIFIER

%destructor { delete $$; } expr

%destructor { delete $$; } expr_list

%destructor { delete $$; } stmt

%destructor { delete $$; } stmt_list

%destructor { delete $$; } param_list

%destructor { delete $$; } func

%destructor { delete $$; } var_decl

%destructor { delete $$; } program

%%

file: program END
    {
      observer.OnProgram(std::unique_ptr<Program>($1));
    }
    ;

program: func
       {
         $$ = new Program();
         $$->AppendFunc($1);
       }
       | program func
       {
         $$ = $1;
         $$->AppendFunc($2);
       }
       | program var_decl
       {
         $$ = $1;
         $$->AppendGlobalVar($2);
       }
       | var_decl
       {
         $$ = new Program();
         $$->AppendGlobalVar($1);
       }
       ;

type: TYPE_NAME
    {
      $$ = new Type($1);
    }
    | VARIABILITY TYPE_NAME
    {
      $$ = new Type($2, $1);
    }
    ;

var_decl: type IDENTIFIER ';'
        {
          $$ = new Var($1, decl_name($2, @2), nullptr);
        }
        | type IDENTIFIER '=' expr ';'
        {
          $$ = new Var($1, decl_name($2, @2), $4);
        }
        ;

param_decl: type IDENTIFIER
          {
            $$ = new Var($1, decl_name($2, @2), nullptr);
          }
          ;

param_list: param_decl
          {
            $$ = new param_list();
            $$->emplace_back($1);
          }
          | param_list param_decl
          {
            $$ = $1;
            $$->emplace_back($2);
          }
          ;

func: type IDENTIFIER '(' param_list ')' compound_stmt
    {
      $$ = new func($1, decl_name($2, @2), $4, $6);
    }
    | type IDENTIFIER '(' ')' compound_stmt
    {
      $$ = new func($1, decl_name($2, @2), new param_list(), $5);
    }
    ;

stmt: compound_stmt
    | return_stmt
    | decl_stmt
    | assignment_stmt
    ;

stmt_list: stmt
         {
           $$ = new stmt_list();
           $$->emplace_back($1);
         }
         | stmt_list stmt
         {
           $$ = $1;
           $$->emplace_back($2);
         }
         ;

assignment_stmt: unary_expr '=' expr ';'
               {
                 $$ = new AssignmentStmt($1, $3);
               }
               ;

decl_stmt: var_decl
         {
           $$ = new decl_stmt($1);
         }

return_stmt: RETURN expr ';'
           {
             $$ = new return_stmt($2);
           }
           ;

compound_stmt: '{' stmt_list '}'
             {
               $$ = new compound_stmt($2);
             }
             ;

expr_list: expr
         {
           $$ = new expr_list();

           $$->emplace_back($1);
         }
         | expr_list ',' expr
         {
           $$ = $1;

           $$->emplace_back($3);
         }
         ;

primary_expr: INT_LITERAL
            {
              $$ = new int_literal($1, @1);
            }
            | FLOAT_LITERAL
            {
              $$ = new float_literal($1, @1);
            }
            | '(' expr ')'
            {
              $$ = new group_expr($2);
            }
            | TYPE_NAME '(' expr_list ')'
            {
              $$ = new type_constructor($1, $3, @$);
            }
            | IDENTIFIER
            {
              $$ = new var_ref(decl_name($1, @1));
            }
            ;

postfix_expr: primary_expr
            | postfix_expr '.' IDENTIFIER
            {
              $$ = new member_expr($1, decl_name($3, @3), @$);
            }

unary_expr: postfix_expr
          | '-' unary_expr
          {
            $$ = new unary_expr($2, unary_expr::kind::NEGATE, @$);
          }
          | '~' unary_expr
          {
            $$ = new unary_expr($2, unary_expr::kind::BITWISE_NOT, @$);
          }
          | '!' unary_expr
          {
            $$ = new unary_expr($2, unary_expr::kind::LOGICAL_NOT, @$);
          }
          ;

multiplicative_expr: unary_expr
                   | multiplicative_expr '*' unary_expr
                   {
                     $$ = new binary_expr($1, $3, binary_expr::kind::MUL, @$);
                   }
                   | multiplicative_expr '/' unary_expr
                   {
                     $$ = new binary_expr($1, $3, binary_expr::kind::DIV, @$);
                   }
                   | multiplicative_expr '%' unary_expr
                   {
                     $$ = new binary_expr($1, $3, binary_expr::kind::MOD, @$);
                   }
                   ;

additive_expr: multiplicative_expr
             | additive_expr '+' multiplicative_expr
             {
               $$ = new binary_expr($1, $3, binary_expr::kind::ADD, @$);
             }
             | additive_expr '-' multiplicative_expr
             {
               $$ = new binary_expr($1, $3, binary_expr::kind::SUB, @$);
             }
             ;

expr: additive_expr
