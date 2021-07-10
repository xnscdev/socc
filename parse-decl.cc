/* parse-decl.cc -- This file is part of SOCC.
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

#include <algorithm>
#include "context.hh"

using namespace socc;

FileScopeDeclPtr
Context::parse_decl_func (Location loc, TypePtr rettype, std::string name)
{
  std::vector <std::pair <TypePtr, std::string>> params;
  TokenPtr token = next_token ();
  bool empty_params = true;
  bool try_define = false;
  if (token != nullptr)
    {
      if (token->type == TokenType::KeywordVoid)
	{
	  TokenPtr lookahead = next_token ();
	  if (lookahead == nullptr)
	    {
	      error (currloc, "unexpected end of input, expected " +
		     bold (")"));
	      return nullptr;
	    }
	  else if (lookahead->type == TokenType::RightParen)
	    {
	      empty_params = true;
	      try_define = true;
	    }
	  else
	    token_stack.push (std::move (lookahead));
	}
      else if (token->type == TokenType::RightParen)
	try_define = true;
    }

  if (!try_define)
    {
      token_stack.push (std::move (token));
      while (1)
	{
	  token = next_token ();
	  if (token == nullptr)
	    {
	      error (currloc, "unexpected end of input, expected parameter");
	      return nullptr;
	    }
	  Location loc = token->loc;
	  token_stack.push (std::move (token));

	  TypePtr type = parse_type (loc, TypeContext::FuncParam);
	  if (type == nullptr)
	    {
	      error (loc, "expected a type in parameter list");
	      do
		token = next_token ();
	      while (token != nullptr && token->type != TokenType::Comma
		     && token->type != TokenType::RightParen);
	      if (token == nullptr)
		return nullptr;
	    }
	  else
	    {
	      std::string param_name;
	      token = next_token ();
	      if (token == nullptr)
		{
		  error (currloc,
			 "unexpected end of input, expected parameter");
		  return nullptr;
		}
	      else if (token->type == TokenType::Identifier)
		{
		  param_name = token->str;
		  token = next_token ();
		  if (token == nullptr)
		    {
		      error (currloc, "unexpected end of input, expected " +
			     bold (",") + " or " + bold (")"));
		      return nullptr;
		    }
		}
	      params.push_back (std::make_pair <TypePtr, std::string>
				(std::move (type), std::move (param_name)));
	    }

	  if (token->type == TokenType::RightParen)
	    break;
	  else if (token->type != TokenType::Comma)
	    {
	      error (token->loc, "expected " + bold (",") + " or " +
		     bold (")"));
	      token_stack.push (std::move (token));
	    }
	}
    }

  token = next_token ();
  if (token == nullptr)
    {
      error (currloc, "unexpected end of input, expected " + bold (";") +
	     " or " + bold ("{"));
      return nullptr;
    }
  else if (token->type != TokenType::LeftBrace)
    {
      if (token->type != TokenType::Semicolon)
	{
	  error (token->loc, "unexpected token, expected " + bold (";") +
		 " or " + bold ("{"));
	  token_stack.push (std::move (token));
	}
      std::vector <TypePtr> types;
      for (const std::pair <TypePtr, std::string> &param : params)
	types.push_back (std::move (param.first));
      return std::make_unique <FuncDeclarationAST> (loc, std::move (rettype),
						    name, std::move (types),
						    empty_params);
    }

  /* At this point, we are parsing a function definition */
  std::unique_ptr <BlockAST> body = parse_stmt_block (token->loc);
  if (body == nullptr)
    return nullptr;
  return std::make_unique <FuncDefinitionAST> (loc, std::move (rettype),
					       name, std::move (params),
					       empty_params, std::move (body));
}

FileScopeDeclPtr
Context::next_decl (void)
{
  while (1)
    {
      TokenPtr token = next_token ();
      if (token == nullptr)
	return nullptr;
      Location loc = token->loc;
      token_stack.push (std::move (token));

      TypePtr type = parse_type (loc, TypeContext::FileScope);
      if (type == nullptr)
	{
	  token = next_token ();
	  if (token == nullptr)
	    return nullptr;
	  error (token->loc, "unexpected token, expected declaration");
	}
      else
	{
	  token = next_token ();
	  if (token == nullptr)
	    {
	      error (currloc, "unexpected end of input, expected identifier");
	      return nullptr;
	    }
	  TokenPtr lookahead = next_token ();
	  if (lookahead != nullptr && lookahead->type == TokenType::LeftParen)
	    {
	      if (token->type != TokenType::Identifier)
		{
		  error (token->loc,
			 "expected an identifier in function declaration");
		  continue;
		}
	      return parse_decl_func (token->loc, std::move (type), token->str);
	    }
	  else
	    {
	      token_stack.push (std::move (lookahead));
	      token_stack.push (std::move (token));
	      if (type->type == TypeType::Primitive
		  && type->primitive == PrimitiveType::Void)
		{
		  error (loc, "use of " + bold ("void") +
			 " type is invalid in this context");
		  continue;
		}
	      StatementPtr st =
		parse_stmt_variable_declaration (loc, std::move (type));
	      if (st == nullptr)
		return nullptr;

	      /* Convert StatementPtr to FileScopeDeclPtr */
	      VariableDeclarationAST *rst =
		dynamic_cast <VariableDeclarationAST *> (st.get ());
	      FileScopeDeclPtr decl;
	      st.release ();
	      decl.reset (rst);
	      return decl;
	    }
	}
    }
}
