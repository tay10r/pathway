%{
#include "lexer.h"
#include "program_consumer.h"
#include "syntax_error_observer.h"

#include "generated/lex.h"
#include "generated/parse.h"

namespace {

void yyerror(const Location* location,
             Lexer&,
             ProgramConsumer&,
             SyntaxErrorObserver& syntaxErrorObserver,
             const char* errorMessage)
{
  syntaxErrorObserver.ObserveSyntaxError(*location, errorMessage);
}

int yylex(semantic_value* value, Location* location, Lexer& lexer)
{
  auto token = lexer.Lex();

  if (!token) {
    return END;
  }

  if (token->Kind() == IDENTIFIER) {
    value->as_string = new std::string(token->AsStringView());
  } else if (token->Kind() == INT_LITERAL) {
    value->as_int = token->AsInt();
  } else if (token->Kind() == FLOAT_LITERAL) {
    value->as_float = token->AsDouble();
  }

  *location = token->GetLocation();

  return token->Kind();
}

} // namespace

%}

%code requires {

#include "common.h"

class Lexer;
class ProgramConsumer;
class SyntaxErrorObserver;
}

%locations

%define api.location.type {Location}

%define api.value.type {semantic_value}

%define api.pure full

%lex-param {Lexer& lexer}

%parse-param {Lexer& lexer}

%parse-param {ProgramConsumer& programConsumer}

%parse-param {SyntaxErrorObserver& syntaxErrorObserver}

%define parse.error verbose

%left '+' '-'
%left '*' '/'

%token END 0 "end of file"

%token<as_int> INT_LITERAL "int literal"

%token<as_float> FLOAT_LITERAL "float literal"

%token<asBool> BOOL_LITERAL "bool literal"

%token<as_string> IDENTIFIER "identifier"

%token RETURN "return"
%token BREAK "break"
%token CONTINUE "continue"
%token IF "if"
%token ELSE "else"
%token FOR "for"
%token WHILE "while"

%token INVALID_CHAR "invalid character"

%token VOID "void"
%token BOOL "bool"
%token INT "int"
%token FLOAT "float"
%token VEC2 "vec2"
%token VEC3 "vec3"
%token VEC4 "vec4"
%token VEC2I "vec2i"
%token VEC3I "vec3i"
%token VEC4I "vec4i"
%token MAT2 "mat2"
%token MAT3 "mat3"
%token MAT4 "mat4"

%token UNIFORM "uniform"
%token VARYING "varying"

%type <asTypeID> type_name "type name"
%type <asVariability> variability "variability"
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

%destructor { delete $$; } type

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
      programConsumer.ConsumeProgram(std::unique_ptr<Program>($1));
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

type_name: VOID  { $$ = TypeID::Void; }
         | BOOL  { $$ = TypeID::Bool; }
         | INT   { $$ = TypeID::Int; }
         | FLOAT { $$ = TypeID::Float; }
         | VEC2  { $$ = TypeID::Vec2; }
         | VEC3  { $$ = TypeID::Vec3; }
         | VEC4  { $$ = TypeID::Vec4; }
         | VEC2I { $$ = TypeID::Vec2i; }
         | VEC3I { $$ = TypeID::Vec3i; }
         | VEC4I { $$ = TypeID::Vec4i; }
         | MAT2  { $$ = TypeID::Mat2; }
         | MAT3  { $$ = TypeID::Mat3; }
         | MAT4  { $$ = TypeID::Mat4; }
         ;

variability: UNIFORM { $$ = Variability::Uniform; }
           | VARYING { $$ = Variability::Varying; }
           ;

type: type_name
    {
      $$ = new Type($1);
    }
    | variability type_name
    {
      $$ = new Type($2, $1);
    }
    ;

var_decl: type IDENTIFIER ';'
        {
          $$ = new Var($1, DeclName($2, @2), nullptr);
        }
        | type IDENTIFIER '=' expr ';'
        {
          $$ = new Var($1, DeclName($2, @2), $4);
        }
        ;

param_decl: type IDENTIFIER
          {
            $$ = new Var($1, DeclName($2, @2), nullptr);
          }
          ;

param_list: param_decl
          {
            $$ = new param_list();
            $$->emplace_back($1);
          }
          | param_list ',' param_decl
          {
            $$ = $1;
            $$->emplace_back($3);
          }
          ;

func: type IDENTIFIER '(' param_list ')' compound_stmt
    {
      $$ = new Func($1, DeclName($2, @2), $4, $6);
    }
    | type IDENTIFIER '(' ')' compound_stmt
    {
      $$ = new Func($1, DeclName($2, @2), new param_list(), $5);
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
           $$ = new ExprList();

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
              $$ = new IntLiteral($1, @1);
            }
            | FLOAT_LITERAL
            {
              $$ = new FloatLiteral($1, @1);
            }
            | BOOL_LITERAL
            {
              $$ = new BoolLiteral($1, @1);
            }
            | '(' expr ')'
            {
              $$ = new GroupExpr($2, @$);
            }
            | type_name '(' expr_list ')'
            {
              $$ = new type_constructor($1, $3, @$);
            }
            | IDENTIFIER
            {
              $$ = new VarRef(DeclName($1, @1));
            }
            | IDENTIFIER '(' expr_list ')'
            {
              $$ = new FuncCall(DeclName($1, @1), $3, @$);
            }
            ;

postfix_expr: primary_expr
            | postfix_expr '.' IDENTIFIER
            {
              $$ = new MemberExpr($1, DeclName($3, @3), @$);
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
                     $$ = new BinaryExpr($1, $3, BinaryExpr::Kind::Mul, @$);
                   }
                   | multiplicative_expr '/' unary_expr
                   {
                     $$ = new BinaryExpr($1, $3, BinaryExpr::Kind::Div, @$);
                   }
                   | multiplicative_expr '%' unary_expr
                   {
                     $$ = new BinaryExpr($1, $3, BinaryExpr::Kind::Mod, @$);
                   }
                   ;

additive_expr: multiplicative_expr
             | additive_expr '+' multiplicative_expr
             {
               $$ = new BinaryExpr($1, $3, BinaryExpr::Kind::Add, @$);
             }
             | additive_expr '-' multiplicative_expr
             {
               $$ = new BinaryExpr($1, $3, BinaryExpr::Kind::Sub, @$);
             }
             ;

expr: additive_expr
