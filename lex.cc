/* lex.cc -- This file is part of SOCC.
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
#include <unordered_map>
#include "context.hh"

using namespace socc;

static std::unordered_map <std::string, TokenType> keywords = {
  {"auto", TokenType::KeywordAuto},
  {"break", TokenType::KeywordBreak},
  {"case", TokenType::KeywordCase},
  {"char", TokenType::KeywordChar},
  {"const", TokenType::KeywordConst},
  {"continue", TokenType::KeywordContinue},
  {"default", TokenType::KeywordDefault},
  {"do", TokenType::KeywordDo},
  {"double", TokenType::KeywordDouble},
  {"else", TokenType::KeywordElse},
  {"enum", TokenType::KeywordEnum},
  {"extern", TokenType::KeywordExtern},
  {"float", TokenType::KeywordFloat},
  {"for", TokenType::KeywordFor},
  {"goto", TokenType::KeywordGoto},
  {"if", TokenType::KeywordIf},
  {"inline", TokenType::KeywordInline},
  {"int", TokenType::KeywordInt},
  {"long", TokenType::KeywordLong},
  {"register", TokenType::KeywordRegister},
  {"restrict", TokenType::KeywordRestrict},
  {"return", TokenType::KeywordReturn},
  {"short", TokenType::KeywordShort},
  {"signed", TokenType::KeywordSigned},
  {"sizeof", TokenType::KeywordSizeof},
  {"static", TokenType::KeywordStatic},
  {"switch", TokenType::KeywordSwitch},
  {"typedef", TokenType::KeywordTypedef},
  {"union", TokenType::KeywordUnion},
  {"unsigned", TokenType::KeywordUnsigned},
  {"void", TokenType::KeywordVoid},
  {"volatile", TokenType::KeywordVolatile},
  {"while", TokenType::KeywordWhile}
};

char
Context::next_char (void)
{
  if (!char_stack.empty ())
    {
      char c = char_stack.top ();
      char_stack.pop ();
      return c;
    }
  else
    {
      char c = stream.get ();
      switch (c)
	{
	case '\n':
	  currloc.line++;
	  currloc.col = 0;
	  break;
	case '\t':
	  currloc.col = ((currloc.col - 2) | 7) + 2;
	  break;
	default:
	  currloc.col++;
	}
      return c;
    }
}

bool
Context::next_char_escaped (char &c)
{
  c = next_char ();
  if (c != '\\')
    return false;
  c = next_char ();
  switch (c)
    {
    case 'n':
      c = '\n';
      break;
    case 't':
      c = '\t';
      break;
    case 'r':
      c = '\r';
      break;
    case 'a':
      c = '\a';
      break;
    case 'b':
      c = '\b';
      break;
    case 'f':
      c = '\f';
      break;
    case 'v':
      c = '\v';
      break;
    case '\\':
      c = '\\';
      break;
    case '\'':
      c = '\'';
      break;
    case '"':
      c = '"';
      break;
    case '?':
      c = '?';
      break;
    default:
      warning (currloc, "unrecognized escape sequence " +
	       bold (std::string ("\\" + c)));
      return false;
    }
  return true;
}

TokenPtr
Context::scan_word (char c)
{
  Location loc = currloc;
  std::string str;
  while (isalnum (c) || c == '_')
    {
      str += c;
      c = next_char ();
    }
  char_stack.push (c);

  if (keywords.count (str))
    return std::make_unique <Token> (keywords[str], loc);
  else
    return std::make_unique <Token> (TokenType::Identifier, loc, str);
}

TokenPtr
Context::scan_number (char c)
{
  Location loc = currloc;
  unsigned long long value = 0;
  IntLiteralWidth width = IntLiteralWidth::Int;
  while (isdigit (c))
    {
      value *= 10;
      value += c - '0';
      c = next_char ();
    }
  while (1)
    {
      switch (c)
	{
	case 'l':
	case 'L':
	  switch (width)
	    {
	    case IntLiteralWidth::Int:
	      width = IntLiteralWidth::Long;
	      break;
	    case IntLiteralWidth::Long:
	      width = IntLiteralWidth::LongLong;
	      break;
	    default:
	      error (loc, "invalid integer literal");
	    }
	  break;
	case 'u':
	case 'U':
	  break;
	default:
	  char_stack.push (c);
	  return std::make_unique <Token> (TokenType::Integer, loc, value,
					   width);
	}
      c = next_char ();
    }
}

TokenPtr
Context::scan_char (void)
{
  Location loc = currloc;
  unsigned int val;
  char c;
  bool escape;
  escape = next_char_escaped (c);
  if (c == '\'' && !escape)
    {
      error (loc, "invalid empty character literal");
      return std::make_unique <Token> (TokenType::Character, loc, 0,
				       IntLiteralWidth::Int);
    }
  val = c;
  escape = next_char_escaped (c);
  if (c == '\'' && !escape)
    return std::make_unique <Token> (TokenType::Character, loc, val,
				     IntLiteralWidth::Int);
  warning (loc, "multi-character literal");
  while (c != '\'' || escape)
    {
      val <<= 8;
      val |= c;
      escape = next_char_escaped (c);
    }
  return std::make_unique <Token> (TokenType::Character, loc, val,
				   IntLiteralWidth::Int);
}

TokenPtr
Context::scan_string (void)
{
  Location loc = currloc;
  std::string str;
  char c;
  bool escape = next_char_escaped (c);
  while (c != '"' || escape)
    {
      if (!escape)
	{
	  if (c == EOF)
	    {
	      error (currloc, "unexpected end of input in string literal");
	      return nullptr;
	    }
	  else if (c == '\n')
	    {
	      error (currloc, "unexpected newline in string literal");
	      break;
	    }
	}
      str += c;
      escape = next_char_escaped (c);
    }
  return std::make_unique <Token> (TokenType::String, loc, str);
}

TokenPtr
Context::next_token (void)
{
  if (!token_stack.empty ())
    {
      TokenPtr token = std::move (token_stack.top ());
      token_stack.pop ();
      return token;
    }
  while (1)
    {
      char c = next_char ();
      while (isspace (c))
	c = next_char ();
      if (c == EOF)
	return nullptr;

      if (isalpha (c) || c == '_')
	return scan_word (c);
      else if (isdigit (c))
	return scan_number (c);
      else if (c == '\'')
	return scan_char ();
      else if (c == '"')
	return scan_string ();

      Location loc = currloc;
      switch (c)
	{
	case '+':
	  c = next_char ();
	  switch (c)
	    {
	    case '+':
	      return std::make_unique <Token> (TokenType::Inc, loc);
	    case '=':
	      return std::make_unique <Token> (TokenType::AssignPlus, loc);
	    default:
	      char_stack.push (c);
	      return std::make_unique <Token> (TokenType::Plus, loc);
	    }
	case '-':
	  c = next_char ();
	  switch (c)
	    {
	    case '-':
	      return std::make_unique <Token> (TokenType::Dec, loc);
	    case '>':
	      return std::make_unique <Token> (TokenType::Arrow, loc);
	    case '=':
	      return std::make_unique <Token> (TokenType::AssignMinus, loc);
	    default:
	      char_stack.push (c);
	      return std::make_unique <Token> (TokenType::Minus, loc);
	    }
	case '<':
	  c = next_char ();
	  switch (c)
	    {
	    case '<':
	      c = next_char ();
	      if (c == '=')
		return std::make_unique <Token> (TokenType::AssignShl, loc);
	      else
		{
		  char_stack.push (c);
		  return std::make_unique <Token> (TokenType::Shl, loc);
		}
	      break;
	    case '=':
	      return std::make_unique <Token> (TokenType::Le, loc);
	    default:
	      char_stack.push (c);
	      return std::make_unique <Token> (TokenType::Lt, loc);
	    }
	case '>':
	  c = next_char ();
	  switch (c)
	    {
	    case '>':
	      c = next_char ();
	      if (c == '=')
		return std::make_unique <Token> (TokenType::AssignShr, loc);
	      else
		{
		  char_stack.push (c);
		  return std::make_unique <Token> (TokenType::Shr, loc);
		}
	      break;
	    case '=':
	      return std::make_unique <Token> (TokenType::Ge, loc);
	    default:
	      char_stack.push (c);
	      return std::make_unique <Token> (TokenType::Gt, loc);
	    }
	case '&':
	  c = next_char ();
	  switch (c)
	    {
	    case '&':
	      return std::make_unique <Token> (TokenType::LogicalAnd, loc);
	    case '=':
	      return std::make_unique <Token> (TokenType::AssignAnd, loc);
	    default:
	      char_stack.push (c);
	      return std::make_unique <Token> (TokenType::And, loc);
	    }
	case '|':
	  c = next_char ();
	  switch (c)
	    {
	    case '|':
	      return std::make_unique <Token> (TokenType::LogicalOr, loc);
	    case '=':
	      return std::make_unique <Token> (TokenType::AssignOr, loc);
	    default:
	      char_stack.push (c);
	      return std::make_unique <Token> (TokenType::Or, loc);
	    }
	case '^':
	  c = next_char ();
	  if (c == '=')
	    return std::make_unique <Token> (TokenType::AssignXor, loc);
	  else
	    {
	      char_stack.push (c);
	      return std::make_unique <Token> (TokenType::Xor, loc);
	    }
	case '=':
	  c = next_char ();
	  if (c == '=')
	    return std::make_unique <Token> (TokenType::Eq, loc);
	  else
	    {
	      char_stack.push (c);
	      return std::make_unique <Token> (TokenType::Assign, loc);
	    }
	case '!':
	  c = next_char ();
	  if (c == '=')
	    return std::make_unique <Token> (TokenType::Ne, loc);
	  else
	    {
	      char_stack.push (c);
	      return std::make_unique <Token> (TokenType::LogicalNot, loc);
	    }
	case '*':
	  c = next_char ();
	  if (c == '=')
	    return std::make_unique <Token> (TokenType::AssignMul, loc);
	  else
	    {
	      char_stack.push (c);
	      return std::make_unique <Token> (TokenType::Mul, loc);
	    }
	case '/':
	  c = next_char ();
	  switch (c)
	    {
	    case '/':
	      do
		c = next_char ();
	      while (c != '\n' && c != EOF);
	      char_stack.push (c);
	      continue;
	    case '=':
	      return std::make_unique <Token> (TokenType::AssignDiv, loc);
	    default:
	      char_stack.push (c);
	      return std::make_unique <Token> (TokenType::Div, loc);
	    }
	case '%':
	  c = next_char ();
	  if (c == '=')
	    return std::make_unique <Token> (TokenType::AssignMod, loc);
	  else
	    {
	      char_stack.push (c);
	      return std::make_unique <Token> (TokenType::Mod, loc);
	    }
	case '~':
	  return std::make_unique <Token> (TokenType::Not, loc);
	case '(':
	  return std::make_unique <Token> (TokenType::LeftParen, loc);
	case ')':
	  return std::make_unique <Token> (TokenType::RightParen, loc);
	case '[':
	  return std::make_unique <Token> (TokenType::LeftBracket, loc);
	case ']':
	  return std::make_unique <Token> (TokenType::RightBracket, loc);
	case '{':
	  return std::make_unique <Token> (TokenType::LeftBrace, loc);
	case '}':
	  return std::make_unique <Token> (TokenType::RightBrace, loc);
	case ';':
	  return std::make_unique <Token> (TokenType::Semicolon, loc);
	case ',':
	  return std::make_unique <Token> (TokenType::Comma, loc);
	case '.':
	  return std::make_unique <Token> (TokenType::Dot, loc);
	default:
	  error (loc, "unexpected character " + bold (std::string (1, c)));
	}
    }
}
