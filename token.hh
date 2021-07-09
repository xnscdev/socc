/* token.hh -- This file is part of SOCC.
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

#ifndef _TOKEN_HH
#define _TOKEN_HH

#include <memory>
#include "location.hh"

namespace socc
{
  enum class TokenType
  {
    Character,
    String,
    Integer,
    Identifier,
    LeftParen,
    RightParen,
    LeftBracket,
    RightBracket,
    LeftBrace,
    RightBrace,
    Semicolon,
    Comma,

    /* Operators */
    Assign,
    AssignPlus,
    AssignMinus,
    AssignMul,
    AssignDiv,
    AssignMod,
    AssignShl,
    AssignShr,
    AssignAnd,
    AssignXor,
    AssignOr,
    LogicalAnd,
    LogicalOr,
    LogicalNot,
    And,
    Xor,
    Or,
    Not,
    Eq,
    Ne,
    Lt,
    Le,
    Gt,
    Ge,
    Shl,
    Shr,
    Plus,
    Minus,
    Mul,
    Div,
    Mod,
    Inc,
    Dec,
    Dot,
    Arrow,

    /* Keywords */
    KeywordAuto,
    KeywordBreak,
    KeywordCase,
    KeywordChar,
    KeywordConst,
    KeywordContinue,
    KeywordDefault,
    KeywordDo,
    KeywordDouble,
    KeywordElse,
    KeywordEnum,
    KeywordExtern,
    KeywordFloat,
    KeywordFor,
    KeywordGoto,
    KeywordIf,
    KeywordInline,
    KeywordInt,
    KeywordLong,
    KeywordRegister,
    KeywordRestrict,
    KeywordReturn,
    KeywordShort,
    KeywordSigned,
    KeywordSizeof,
    KeywordStatic,
    KeywordStruct,
    KeywordSwitch,
    KeywordTypedef,
    KeywordUnion,
    KeywordUnsigned,
    KeywordVoid,
    KeywordVolatile,
    KeywordWhile
  };

  enum class IntLiteralWidth
  {
    Int,
    Long,
    LongLong
  };

  class Token
  {
  public:
    TokenType type;
    Location loc;
    std::string str;
    unsigned long long num;
    IntLiteralWidth num_width;

    Token (TokenType type, Location loc) : type (type), loc (loc) {}
    Token (TokenType type, Location loc, std::string str) :
      type (type), loc (loc), str (str) {}
    Token (TokenType type, Location loc, unsigned long long num,
	   IntLiteralWidth num_width) :
      type (type), loc (loc), num (num), num_width (num_width) {}
  };

  typedef std::unique_ptr <Token> TokenPtr;
}

#endif
