/* parse-statement.cc -- This file is part of SOCC.
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

#include "context.hh"

using namespace socc;

StatementPtr
Context::stmt_handle_parse_error (void)
{
  while (1)
    {
      TokenPtr token = next_token ();
      if (token == nullptr)
	return nullptr;
      if (token->type == TokenType::Semicolon)
	break;
    }
  return next_statement ();
}

StatementPtr
Context::parse_stmt_return_expr (Location loc, bool ret)
{
  if (ret)
    {
      TokenPtr token = next_token ();
      if (token != nullptr && token->type == TokenType::Semicolon)
	return std::make_unique <ReturnAST> (loc, nullptr);
      token_stack.push (std::move (token));
    }

  ExprPtr expr = next_expr ();
  if (expr == nullptr)
    return nullptr;
  StatementPtr st;
  if (ret)
    st = std::make_unique <ReturnAST> (loc, std::move (expr));
  else
    st = std::make_unique <ExprStmtAST> (loc, std::move (expr));

  TokenPtr token = next_token ();
  if (token == nullptr)
    error (currloc, "unexpected end of input, expected " + bold (";"));
  else if (token->type != TokenType::Semicolon)
    {
      error (token->loc, "expected " + bold (";") + " at end of statement");
      token_stack.push (std::move (token));
    }
  return st;
}

std::unique_ptr <BlockAST>
Context::parse_stmt_block (Location loc)
{
  indent++;
  std::vector <StatementPtr> body;
  while (1)
    {
      TokenPtr token = next_token ();
      if (token == nullptr)
	{
	  error (currloc, "unexpected end of input, expected " + bold ("}"));
	  return std::make_unique <BlockAST> (loc, std::move (body), --indent);
	}
      else if (token->type == TokenType::RightBrace)
	return std::make_unique <BlockAST> (loc, std::move (body), --indent);

      token_stack.push (std::move (token));
      StatementPtr st = next_statement ();
      if (st == nullptr)
	{
	  error (currloc, "unexpected end of input, expected statement");
	  return std::make_unique <BlockAST> (loc, std::move (body), --indent);
	}
      body.push_back (std::move (st));
    }
}

StatementPtr
Context::parse_stmt_variable_declaration (Location loc, TypePtr type)
{
  TokenPtr token = next_token ();
  if (token == nullptr)
    {
      error (currloc, "unexpected end of input, expected identifier");
      return nullptr;
    }
  if (token->type != TokenType::Identifier)
    {
      error (token->loc, "expected identifier in variable declaration");
      return stmt_handle_parse_error ();
    }

  std::unique_ptr <VariableDeclarationAST> st =
    std::make_unique <VariableDeclarationAST> (loc, type, token->str);
  token = next_token ();
  if (token == nullptr)
    {
      error (currloc, "unexpected end of input, expected " + bold (";"));
      return st;
    }
  else if (token->type == TokenType::Assign)
    {
      ExprPtr initval = next_expr ();
      if (initval == nullptr)
	{
	  error (currloc, "unexpected end of input, expected expression");
	  return st;
	}
      token = next_token ();
      if (token == nullptr)
	{
	  error (currloc, "unexpected end of input, expected " + bold (";"));
	  return st;
	}
      st->initval = std::move (initval);
    }
  if (token->type != TokenType::Semicolon)
    {
      error (token->loc, "unexpected token, expected " + bold (";"));
      token_stack.push (std::move (token));
    }
  return st;
}

StatementPtr
Context::next_statement (void)
{
  while (1)
    {
      TokenPtr token = next_token ();
      if (token == nullptr)
	return nullptr;
      Location loc = token->loc;
      switch (token->type)
	{
	case TokenType::KeywordReturn:
	  return parse_stmt_return_expr (loc, true);
	case TokenType::LeftBrace:
	  return parse_stmt_block (loc);
	case TokenType::Semicolon:
	  break;
	default:
	  token_stack.push (std::move (token));
	  TypePtr type = parse_type (loc, TypeContext::Local);
	  if (type != nullptr)
	    return parse_stmt_variable_declaration (loc, type);
	  return parse_stmt_return_expr (loc, false);
	}
    }
}

void
ExprStmtAST::print (std::ostream &os) const
{
  os << *expr << ';';
}

void
ReturnAST::print (std::ostream &os) const
{
  if (value == nullptr)
    os << "return;";
  else
    os << "return " << *value << ';';
}

void
BlockAST::print (std::ostream &os) const
{
  os << "{\n";
  for (const StatementPtr &st : body)
    os << std::string ((indent + 1) * 2, ' ') << *st << '\n';
  os << std::string (indent * 2, ' ') << '}';
}

void
VariableDeclarationAST::print (std::ostream &os) const
{
  std::string text = type->name ();
  os << text;
  if (text.back () != '*')
    os << ' ';
  os << name;
  if (initval)
    os << " = " << *initval;
  os << ';';
}

void
FuncDeclarationAST::print (std::ostream &os) const
{
  std::string text = rettype->name ();
  os << text;
  if (text.back () != '*')
    os << ' ';
  os << name << " (";
  if (params.empty ())
    os << "void";
  else
    {
      os << params[0]->name ();
      for (size_t i = 1; i < params.size (); i++)
	os << ", " << params[i]->name ();
    }
  os << ");";
}

void
FuncDefinitionAST::print (std::ostream &os) const
{
  std::string text = rettype->name ();
  os << text << '\n' << name << " (";
  if (params.empty ())
    os << "void";
  else
    {
      std::string text = params[0].first->name ();
      os << text;
      if (text.back () != '*')
	os << ' ';
      os << params[0].second;
      for (size_t i = 1; i < params.size (); i++)
	{
	  os << ", ";
	  text = params[i].first->name ();
	  os << text;
	  if (text.back () != '*')
	    os << ' ';
	  os << params[i].second;
	}
    }
  os << ")\n" << *body;
}
