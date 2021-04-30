%{
#include "lexer.h"
#include "module_consumer.h"
#include "syntax_error_observer.h"

#include "generated/lex.h"
#include "generated/parse.h"

namespace {

void yyerror(const Location* location,
             Lexer&,
             ModuleConsumer&,
             SyntaxErrorObserver& syntaxErrorObserver,
             const char* originalErrorMessage)
{
  std::string errorMessage(originalErrorMessage);

  size_t prefix = sizeof("syntax error, ") - 1;

  if (errorMessage.substr(0, prefix) == "syntax error, ")
    errorMessage = errorMessage.substr(prefix);

  syntaxErrorObserver.ObserveSyntaxError(*location, errorMessage.c_str());
}

int yylex(SemanticValue* value, Location* location, Lexer& lexer)
{
  auto token = lexer.Lex();

  if (!token) {
    return TOK_END;
  }

  if (token->Kind() == TOK_IDENTIFIER) {
    value->asString = new std::string(token->AsStringView());
  } else if (token->Kind() == TOK_INT_LITERAL) {
    value->asInt = token->AsInt();
  } else if (token->Kind() == TOK_FLOAT_LITERAL) {
    value->asFloat = token->AsDouble();
  }

  *location = token->GetLocation();

  return token->Kind();
}

} // namespace

%}

%code requires {

#include "semantic_value.h"

class Lexer;
class ModuleConsumer;
class SyntaxErrorObserver;
}

%locations

%define api.location.type {Location}

%define api.value.type {SemanticValue}

%define api.token.prefix {TOK_}

%define api.pure full

%lex-param {Lexer& lexer}

%parse-param {Lexer& lexer}

%parse-param {ModuleConsumer& moduleConsumer}

%parse-param {SyntaxErrorObserver& syntaxErrorObserver}

%define parse.error verbose

%left '+' '-'
%left '*' '/'

%token END 0 "end of file"

%token<asInt> INT_LITERAL "int literal"

%token<asFloat> FLOAT_LITERAL "float literal"
%token<asFloat> PI "pi"
%token<asFloat> INFINITY "infinity"

%token<asBool> TRUE "true"
%token<asBool> FALSE "false"

%token<asString> IDENTIFIER "identifier"

%token RETURN "return"
%token BREAK "break"
%token CONTINUE "continue"
%token IF "if"
%token ELSE "else"
%token FOR "for"
%token WHILE "while"

%token MODULE "module"
%token EXPORT "export"
%token IMPORT "import"

%token SCOPE_RESOLUTION_OPERATOR "::"

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

%type <asExpr> primary_expr
%type <asExpr> unary_expr
%type <asExpr> postfix_expr
%type <asExpr> multiplicative_expr
%type <asExpr> additive_expr
%type <asExpr> expr

%type <asExprList> expr_list

%type <asVarDecl> param_decl

%type <asParamList> param_list

%type <asStmtList> stmt_list

%type <asStmt> stmt
%type <asStmt> assignment_stmt
%type <asStmt> return_stmt
%type <asStmt> compound_stmt

%type <asStmt> decl_stmt

%type <asVarDecl> var_decl

%type <asFuncDecl> func

%type <asModuleName> module_name

%type <asModuleExportDecl> module_export_decl

%type <asModuleImportDecl> module_import_decl

%type <asModule> module

%destructor { delete $$; } IDENTIFIER

%destructor { delete $$; } type

%destructor { delete $$; } expr

%destructor { delete $$; } expr_list

%destructor { delete $$; } stmt

%destructor { delete $$; } stmt_list

%destructor { delete $$; } param_list

%destructor { delete $$; } func

%destructor { delete $$; } var_decl

%destructor { delete $$; } module

%destructor { delete $$; } module_name

%destructor { delete $$; } module_export_decl

%destructor { delete $$; } module_import_decl

%%

file: module END
    {
      moduleConsumer.ConsumeModule(std::unique_ptr<Module>($1));
    }
    ;

module: func
       {
         $$ = new Module();
         $$->AppendFunc($1);
       }
       | module func
       {
         $$ = $1;
         $$->AppendFunc($2);
       }
       | module var_decl
       {
         $$ = $1;
         $$->AppendGlobalVar($2);
       }
       | var_decl
       {
         $$ = new Module();
         $$->AppendGlobalVar($1);
       }
       | module_export_decl
       {
         $$ = new Module();
         $$->SetModuleExportDecl($1);
       }
       | module module_export_decl
       {
         $$ = $1;
         $$->SetModuleExportDecl($2);
       }
       | module_import_decl
       {
         $$ = new Module();
         $$->AppendModuleImportDecl($1);
       }
       | module module_import_decl
       {
         $$ = $1;
         $$->AppendModuleImportDecl($2);
       }
       ;

module_name: IDENTIFIER
           {
             $$ = new ModuleName();
             $$->Append($1, @1);
           }
           | module_name SCOPE_RESOLUTION_OPERATOR IDENTIFIER
           {
             $$ = $1;
             $$->Append($3, @3);
           }
           ;

module_import_decl: IMPORT MODULE module_name ';'
                  {
                    $$ = new ModuleImportDecl($3);
                  }

module_export_decl: EXPORT MODULE module_name ';'
                  {
                    $$ = new ModuleExportDecl($3);
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
          $$ = new VarDecl($1, DeclName($2, @2), nullptr);
        }
        | type IDENTIFIER '=' expr ';'
        {
          $$ = new VarDecl($1, DeclName($2, @2), $4);
        }
        ;

param_decl: type IDENTIFIER
          {
            $$ = new VarDecl($1, DeclName($2, @2), nullptr);
          }
          ;

param_list: param_decl
          {
            $$ = new ParamList();
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
      $$ = new FuncDecl($1, DeclName($2, @2), $4, $6);
    }
    | type IDENTIFIER '(' ')' compound_stmt
    {
      $$ = new FuncDecl($1, DeclName($2, @2), new ParamList(), $5);
    }
    ;

stmt: compound_stmt
    | return_stmt
    | decl_stmt
    | assignment_stmt
    ;

stmt_list: stmt
         {
           $$ = new StmtList();
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
           $$ = new DeclStmt($1);
         }

return_stmt: RETURN expr ';'
           {
             $$ = new ReturnStmt($2);
           }
           ;

compound_stmt: '{' stmt_list '}'
             {
               $$ = new CompoundStmt($2);
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
            | INFINITY
            {
              $$ = FloatLiteral::Infinity(@1);
            }
            | PI
            {
              $$ = FloatLiteral::Pi(@1);
            }
            | TRUE
            {
              $$ = new BoolLiteral(true, @1);
            }
            | FALSE
            {
              $$ = new BoolLiteral(false, @1);
            }
            | '(' expr ')'
            {
              $$ = new GroupExpr($2, @$);
            }
            | type_name '(' expr_list ')'
            {
              $$ = new TypeConstructor($1, $3, @$);
            }
            | IDENTIFIER
            {
              $$ = new VarRef(DeclName($1, @1));
            }
            | IDENTIFIER '(' expr_list ')'
            {
              $$ = new FuncCall(DeclName($1, @1), $3, @$);
            }
            | IDENTIFIER '(' ')'
            {
              $$ = new FuncCall(DeclName($1, @1), new ExprList(), @$);
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
            $$ = new UnaryExpr($2, UnaryExpr::Kind::Negate, @$);
          }
          | '~' unary_expr
          {
            $$ = new UnaryExpr($2, UnaryExpr::Kind::BitwiseNot, @$);
          }
          | '!' unary_expr
          {
            $$ = new UnaryExpr($2, UnaryExpr::Kind::LogicalNot, @$);
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
