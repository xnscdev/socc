/* context.hh -- This file is part of SOCC.
   Copyright (C) 2021 XNSC

   SOCC is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   SOCC is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with SOCC. If not, see <https://www.gnu.org/licenses/>. */

#ifndef _CONTEXT_HH
#define _CONTEXT_HH

#include <istream>
#include <stack>
#include "ast.hh"

namespace socc
{
  class Context
  {
    std::stack <char> char_stack;
    std::stack <TokenPtr> token_stack;
    unsigned int errors;
    unsigned int indent;

    char next_char (void);
    bool next_char_escaped (char &c);
    TokenPtr scan_word (char c);
    TokenPtr scan_number (char c);
    TokenPtr scan_char (void);
    TokenPtr scan_string (void);
    bool expr_get_unary_op (TokenType type, UnaryOperator &op);
    bool expr_get_binary_op (TokenType type, BinaryOperator &op);
    unsigned int expr_get_binary_prec (BinaryOperator op);
    void expr_call_build_params (std::vector <ExprPtr> &params);
    StatementPtr stmt_handle_parse_error (void);
    ExprPtr parse_expr_atomic (void);
    ExprPtr parse_expr_basic (void);
    ExprPtr parse_expr_suffix (ExprPtr expr);
    ExprPtr parse_expr_member_access (ExprPtr expr, bool deref);
    ExprPtr parse_expr_array_index (ExprPtr expr);
    ExprPtr parse_expr_binary (ExprPtr lhs, unsigned int minprec);
    StatementPtr parse_stmt_return_expr (Location loc, bool ret);
    std::unique_ptr <BlockAST> parse_stmt_block (Location loc);
    StatementPtr parse_stmt_variable_declaration (Location loc, TypePtr type);
    FileScopeDeclPtr parse_decl_func (Location loc, TypePtr type,
				      std::string name);

  public:
    Location currloc;
    std::istream &stream;

    Context (std::string name, std::istream &stream) :
      indent (0), currloc (name), stream (stream) {}
    std::string bold (std::string str);
    void warning (Location loc, std::string msg, std::string option = "");
    void error (Location loc, std::string msg);
    TokenPtr next_token (void);
    ExprPtr next_expr (void);
    StatementPtr next_statement (void);
    FileScopeDeclPtr next_decl (void);
    TypePtr parse_type (Location loc, TypeContext tctx);
  };

  void init_console (void);
  void fatal_error (std::string msg, std::string option = "");
  void print_escaped_string (std::ostream &os, std::string str);
}

#endif
