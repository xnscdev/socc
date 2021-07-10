/* parse-expr.cc -- This file is part of SOCC.
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

#include <cctype>
#include <iomanip>
#include <unordered_map>
#include "context.hh"

using namespace socc;

static std::unordered_map <TokenType, UnaryOperator> unary_op_map = {
  {TokenType::Inc, UnaryOperator::IncPrefix},
  {TokenType::Dec, UnaryOperator::DecPrefix},
  {TokenType::Plus, UnaryOperator::Plus},
  {TokenType::Minus, UnaryOperator::Minus},
  {TokenType::Not, UnaryOperator::Not},
  {TokenType::LogicalNot, UnaryOperator::LogicalNot},
  {TokenType::Mul, UnaryOperator::Dereference},
  {TokenType::And, UnaryOperator::Address}
};

static std::unordered_map <TokenType, BinaryOperator> binary_op_map = {
  {TokenType::Mul, BinaryOperator::Mul},
  {TokenType::Div, BinaryOperator::Div},
  {TokenType::Mod, BinaryOperator::Mod},
  {TokenType::Plus, BinaryOperator::Add},
  {TokenType::Minus, BinaryOperator::Sub},
  {TokenType::Shl, BinaryOperator::Shl},
  {TokenType::Shr, BinaryOperator::Shr},
  {TokenType::Lt, BinaryOperator::Lt},
  {TokenType::Le, BinaryOperator::Le},
  {TokenType::Gt, BinaryOperator::Gt},
  {TokenType::Ge, BinaryOperator::Ge},
  {TokenType::Eq, BinaryOperator::Eq},
  {TokenType::Ne, BinaryOperator::Ne},
  {TokenType::And, BinaryOperator::And},
  {TokenType::Xor, BinaryOperator::Xor},
  {TokenType::Or, BinaryOperator::Or},
  {TokenType::LogicalAnd, BinaryOperator::LogicalAnd},
  {TokenType::LogicalOr, BinaryOperator::LogicalOr},
  {TokenType::Assign, BinaryOperator::Assign},
  {TokenType::AssignPlus, BinaryOperator::AssignAdd},
  {TokenType::AssignMinus, BinaryOperator::AssignSub},
  {TokenType::AssignMul, BinaryOperator::AssignMul},
  {TokenType::AssignDiv, BinaryOperator::AssignDiv},
  {TokenType::AssignMod, BinaryOperator::AssignMod},
  {TokenType::AssignShl, BinaryOperator::AssignShl},
  {TokenType::AssignShr, BinaryOperator::AssignShr},
  {TokenType::AssignAnd, BinaryOperator::AssignAnd},
  {TokenType::AssignXor, BinaryOperator::AssignXor},
  {TokenType::AssignOr, BinaryOperator::AssignOr}
};

static std::unordered_map <BinaryOperator, unsigned int> binary_prec_map = {
  {BinaryOperator::Mul, 11},
  {BinaryOperator::Div, 11},
  {BinaryOperator::Mod, 11},
  {BinaryOperator::Add, 10},
  {BinaryOperator::Sub, 10},
  {BinaryOperator::Shl, 9},
  {BinaryOperator::Shr, 9},
  {BinaryOperator::Lt, 8},
  {BinaryOperator::Le, 8},
  {BinaryOperator::Gt, 8},
  {BinaryOperator::Ge, 8},
  {BinaryOperator::Eq, 7},
  {BinaryOperator::Ne, 7},
  {BinaryOperator::And, 6},
  {BinaryOperator::Xor, 5},
  {BinaryOperator::Or, 4},
  {BinaryOperator::LogicalAnd, 3},
  {BinaryOperator::LogicalOr, 2},
  {BinaryOperator::Assign, 1},
  {BinaryOperator::AssignAdd, 1},
  {BinaryOperator::AssignSub, 1},
  {BinaryOperator::AssignMul, 1},
  {BinaryOperator::AssignDiv, 1},
  {BinaryOperator::AssignMod, 1},
  {BinaryOperator::AssignShl, 1},
  {BinaryOperator::AssignShr, 1},
  {BinaryOperator::AssignAnd, 1},
  {BinaryOperator::AssignXor, 1},
  {BinaryOperator::AssignOr, 1}
};

bool
Context::expr_get_unary_op (TokenType type, UnaryOperator &op)
{
  if (unary_op_map.count (type))
    {
      op = unary_op_map[type];
      return true;
    }
  else
    return false;
}

bool
Context::expr_get_binary_op (TokenType type, BinaryOperator &op)
{
  if (binary_op_map.count (type))
    {
      op = binary_op_map[type];
      return true;
    }
  else
    return false;
}

unsigned int
Context::expr_get_binary_prec (BinaryOperator op)
{
  if (binary_prec_map.count (op))
    return binary_prec_map[op];
  else
    return 0;
}

void
Context::expr_call_build_params (std::vector <ExprPtr> &params)
{
  TokenPtr token = next_token ();
  if (token == nullptr)
    {
      error (currloc, "unexpected end of input, expected argument list");
      return;
    }
  else if (token->type == TokenType::RightParen)
    return; /* Empty argument list */

  token_stack.push (std::move (token));
  while (1)
    {
      ExprPtr param = next_expr ();
      if (param != nullptr)
	params.push_back (std::move (param));

      token = next_token ();
      if (token == nullptr)
	{
	  error (currloc, "unexpected end of input, expected " + bold (")"));
	  return;
	}
      if (token->type == TokenType::RightParen)
	break;
      else if (token->type != TokenType::Comma)
	{
	  error (token->loc, "expected " + bold (")") + " or " + bold (",") +
		 " in argument list");
	  return;
	}
    }
}

ExprPtr
Context::parse_expr_atomic (void)
{
  while (1)
    {
      TokenPtr token = next_token ();
      if (token == nullptr)
	return nullptr;
      switch (token->type)
	{
	case TokenType::Integer:
	  return std::make_unique <IntegerAST> (token->loc, token->num,
						token->num_width);
	case TokenType::String:
	  return std::make_unique <StringAST> (token->loc, token->str);
	case TokenType::Identifier:
	  return std::make_unique <VariableAST> (token->loc, token->str);
	case TokenType::LeftParen:
	  {
	    ExprPtr expr = next_expr ();
	    token = next_token ();
	    if (token == nullptr)
	      error (currloc, "unexpected end of input, expected " +
		     bold (")"));
	    else if (token->type != TokenType::RightParen)
	      {
		error (token->loc, "expected " + bold (")") +
		       " to match previous " + bold ("("));
		token_stack.push (std::move (token));
	      }
	    return expr;
	  }
	default:
	  break;
	}
    }
}

ExprPtr
Context::parse_expr_basic (void)
{
  while (1)
    {
      TokenPtr token = next_token ();
      if (token == nullptr)
	return nullptr;
      UnaryOperator op;
      Location loc = token->loc;
      if (expr_get_unary_op (token->type, op))
	{
	  ExprPtr operand = parse_expr_basic ();
	  if (operand == nullptr)
	    {
	      error (loc, "invalid token, expected an expression");
	      continue;
	    }
	  operand = parse_expr_suffix (std::move (operand));
	  return std::make_unique <UnaryAST> (loc, op, std::move (operand));
	}
      else
	{
	  token_stack.push (std::move (token));
	  ExprPtr expr = parse_expr_atomic ();
	  if (expr == nullptr)
	    return nullptr;
	  return parse_expr_suffix (std::move (expr));
	}
    }
}

ExprPtr
Context::parse_expr_suffix (ExprPtr expr)
{
  TokenPtr token = next_token ();
  std::vector <ExprPtr> params;
  if (token != nullptr)
    {
      switch (token->type)
	{
	case TokenType::Dot:
	case TokenType::Arrow:
	  expr = parse_expr_member_access (std::move (expr),
					   token->type == TokenType::Arrow);
	  return parse_expr_suffix (std::move (expr));
	case TokenType::LeftParen:
	  expr_call_build_params (params);
	  expr = std::make_unique <CallAST> (expr->location (),
					     std::move (expr),
					     std::move (params));
	  return parse_expr_suffix (std::move (expr));
	case TokenType::LeftBracket:
	  expr = parse_expr_array_index (std::move (expr));
	  return parse_expr_suffix (std::move (expr));
	case TokenType::Inc:
	case TokenType::Dec:
	  return std::make_unique <UnaryAST> (expr->location (),
					      token->type == TokenType::Inc ?
					      UnaryOperator::IncSuffix :
					      UnaryOperator::DecSuffix,
					      std::move (expr));
	default:
	  token_stack.push (std::move (token));
	}
    }
  return expr;
}

ExprPtr
Context::parse_expr_member_access (ExprPtr expr, bool deref)
{
  TokenPtr token = next_token ();
  if (token == nullptr)
    {
      error (expr->location (), "unexpected end of input");
      return expr;
    }
  if (token->type != TokenType::Identifier)
    {
      error (expr->location (),
	     "expected an identifier after member access operator");
      token_stack.push (std::move (token));
      return expr;
    }
  return std::make_unique <MemberAccessAST> (expr->location (),
					     std::move (expr), token->str,
					     deref);
}

ExprPtr
Context::parse_expr_array_index (ExprPtr expr)
{
  ExprPtr index = next_expr ();
  expr = std::make_unique <ArrayIndexAST> (expr->location (), std::move (expr),
					   std::move (index));
  TokenPtr token = next_token ();
  if (token == nullptr)
    error (currloc, "unexpected end of input, expected " + bold ("]"));
  else if (token->type != TokenType::RightBracket)
    {
      error (currloc, "unexpected token, expected " + bold ("]"));
      token_stack.push (std::move (token));
    }
  return expr;
}

ExprPtr
Context::parse_expr_binary (ExprPtr lhs, unsigned int minprec)
{
  while (1)
    {
      BinaryOperator op;
      TokenPtr token = next_token ();
      if (token == nullptr)
	return lhs;
      if (!expr_get_binary_op (token->type, op))
        {
	  token_stack.push (std::move (token));
	  return lhs;
	}

      unsigned int prec = expr_get_binary_prec (op);
      if (prec < minprec)
	{
	  token_stack.push (std::move (token));
	  return lhs;
	}

      ExprPtr rhs = parse_expr_basic ();
      if (rhs == nullptr)
	{
	  error (currloc, "unexpected end of input, expected an expression");
	  return lhs;
	}
      token = next_token ();
      if (token == nullptr || !binary_op_map.count (token->type))
	{
	  lhs = std::make_unique <BinaryAST> (lhs->location (), op,
					      std::move (lhs), std::move (rhs));
	  token_stack.push (std::move (token));
	  continue;
	}

      /* Peek ahead at next token to see if it has higher precedence */
      BinaryOperator new_op;
      expr_get_binary_op (token->type, new_op);
      unsigned int nextprec = expr_get_binary_prec (new_op);
      token_stack.push (std::move (token));
      if (prec < nextprec)
	rhs = parse_expr_binary (std::move (rhs), prec + 1);
      lhs = std::make_unique <BinaryAST> (lhs->location (), op,
					  std::move (lhs), std::move (rhs));
    }
}

ExprPtr
Context::next_expr (void)
{
  ExprPtr expr = parse_expr_basic ();
  if (expr == nullptr)
    return nullptr;
  return parse_expr_binary (std::move (expr), 0);
}

void
StringAST::print (std::ostream &os) const
{
  print_escaped_string (os, str);
}

void
IntegerAST::print (std::ostream &os) const
{
  os << value;
  if (width == IntLiteralWidth::Long)
    os << 'L';
  else if (width == IntLiteralWidth::LongLong)
    os << "LL";
}

void
CallAST::print (std::ostream &os) const
{
  os << '(' << *func << ") (";
  if (!params.empty ())
    {
      os << *params[0];
      for (size_t i = 1; i < params.size (); i++)
	os << ", " << *params[i];
    }
  os << ')';
}

void
ArrayIndexAST::print (std::ostream &os) const
{
  os << '(' << *array << ")[" << *index << ']';
}

void
MemberAccessAST::print (std::ostream &os) const
{
  os << '(' << *operand << ')';
  if (deref)
    os << "->";
  else
    os << '.';
  os << member;
}

void
VariableAST::print (std::ostream &os) const
{
  os << name;
}

void
UnaryAST::print (std::ostream &os) const
{
  if (op == UnaryOperator::IncSuffix || op == UnaryOperator::DecSuffix)
    {
      os << '(' << *operand << ')';
      if (op == UnaryOperator::IncSuffix)
	os << "++";
      else
	os << "--";
    }
  else
    {
      switch (op)
	{
	case UnaryOperator::IncPrefix:
	  os << "++";
	  break;
	case UnaryOperator::DecPrefix:
	  os << "--";
	  break;
	case UnaryOperator::Plus:
	  os << '+';
	  break;
	case UnaryOperator::Minus:
	  os << '-';
	  break;
	case UnaryOperator::Not:
	  os << '~';
	  break;
	case UnaryOperator::LogicalNot:
	  os << '!';
	  break;
	case UnaryOperator::Dereference:
	  os << '*';
	  break;
	case UnaryOperator::Address:
	  os << '&';
	  break;
	default:
	  return;
	}
      os << '(' << *operand << ')';
    }
}

void
BinaryAST::print (std::ostream &os) const
{
  os << '(' << *lhs;
  switch (op)
    {
    case BinaryOperator::Add:
      os << " + ";
      break;
    case BinaryOperator::Sub:
      os << " - ";
      break;
    case BinaryOperator::Mul:
      os << " * ";
      break;
    case BinaryOperator::Div:
      os << " / ";
      break;
    case BinaryOperator::Mod:
      os << " % ";
      break;
    case BinaryOperator::Shl:
      os << " << ";
      break;
    case BinaryOperator::Shr:
      os << " >> ";
      break;
    case BinaryOperator::Lt:
      os << " < ";
      break;
    case BinaryOperator::Le:
      os << " <= ";
      break;
    case BinaryOperator::Gt:
      os << " > ";
      break;
    case BinaryOperator::Ge:
      os << " >= ";
      break;
    case BinaryOperator::Eq:
      os << " == ";
      break;
    case BinaryOperator::Ne:
      os << " != ";
      break;
    case BinaryOperator::And:
      os << " & ";
      break;
    case BinaryOperator::Xor:
      os << " ^ ";
      break;
    case BinaryOperator::Or:
      os << " | ";
      break;
    case BinaryOperator::LogicalAnd:
      os << " && ";
      break;
    case BinaryOperator::LogicalOr:
      os << " || ";
      break;
    case BinaryOperator::Assign:
      os << " = ";
      break;
    case BinaryOperator::AssignAdd:
      os << " += ";
      break;
    case BinaryOperator::AssignSub:
      os << " -= ";
      break;
    case BinaryOperator::AssignMul:
      os << " *= ";
      break;
    case BinaryOperator::AssignDiv:
      os << " /= ";
      break;
    case BinaryOperator::AssignMod:
      os << " %= ";
      break;
    case BinaryOperator::AssignShl:
      os << " <<= ";
      break;
    case BinaryOperator::AssignShr:
      os << " >>= ";
      break;
    case BinaryOperator::AssignAnd:
      os << " &= ";
      break;
    case BinaryOperator::AssignXor:
      os << " ^= ";
      break;
    case BinaryOperator::AssignOr:
      os << " |= ";
      break;
    }
  os << *rhs << ')';
}

std::ostream &
operator<< (std::ostream &os, const AST &ast)
{
  ast.print (os);
  return os;
}

void
socc::print_escaped_string (std::ostream &os, std::string str)
{
  os << '"';
  for (char c : str)
    {
      if (isprint (c))
	os << c;
      else
	os << '\\' << std::oct << (int) c << std::dec;
    }
  os << '"';
}
