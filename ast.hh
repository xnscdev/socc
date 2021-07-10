/* ast.hh -- This file is part of SOCC.
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

#ifndef _AST_HH
#define _AST_HH

#include <memory>
#include <vector>
#include "token.hh"
#include "type.hh"

namespace socc
{
  enum class UnaryOperator
  {
    IncSuffix,
    IncPrefix,
    DecSuffix,
    DecPrefix,
    Plus,
    Minus,
    Not,
    LogicalNot,
    Dereference,
    Address
  };

  enum class BinaryOperator
  {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Shl,
    Shr,
    Lt,
    Le,
    Gt,
    Ge,
    Eq,
    Ne,
    And,
    Xor,
    Or,
    LogicalAnd,
    LogicalOr,
    Assign,
    AssignAdd,
    AssignSub,
    AssignMul,
    AssignDiv,
    AssignMod,
    AssignShl,
    AssignShr,
    AssignAnd,
    AssignXor,
    AssignOr
  };

  class AST
  {
  public:
    virtual ~AST (void) = default;
    virtual Location &location (void) = 0;
    virtual void print (std::ostream &os) const = 0;
  };

  class ExprAST : public AST
  {
  public:
    virtual bool is_lvalue (void) = 0;
  };

  typedef std::unique_ptr <ExprAST> ExprPtr;

  class StatementAST : public AST
  {
  };

  typedef std::unique_ptr <StatementAST> StatementPtr;

  class FileScopeDeclAST : public AST
  {
  };

  typedef std::unique_ptr <FileScopeDeclAST> FileScopeDeclPtr;

  class StringAST : public ExprAST
  {
  public:
    Location loc;
    std::string str;

    StringAST (Location loc, std::string str) : loc (loc), str (str) {}
    Location &location (void) { return loc; }
    void print (std::ostream &os) const;
    bool is_lvalue (void) { return false; }
  };

  class IntegerAST : public ExprAST
  {
  public:
    Location loc;
    unsigned long long value;
    IntLiteralWidth width;

    IntegerAST (Location loc, unsigned long long value, IntLiteralWidth width) :
      loc (loc), value (value), width (width) {}
    Location &location (void) { return loc; }
    void print (std::ostream &os) const;
    bool is_lvalue (void) { return false; }
  };

  class CallAST : public ExprAST
  {
  public:
    Location loc;
    ExprPtr func;
    std::vector <ExprPtr> params;

    CallAST (Location loc, ExprPtr func, std::vector <ExprPtr> params) :
      loc (loc), func (std::move (func)), params (std::move (params)) {}
    Location &location (void) { return loc; }
    void print (std::ostream &os) const;
    bool is_lvalue (void) { return false; }
  };

  class ArrayIndexAST : public ExprAST
  {
  public:
    Location loc;
    ExprPtr array;
    ExprPtr index;

    ArrayIndexAST (Location loc, ExprPtr array, ExprPtr index) :
      loc (loc), array (std::move (array)), index (std::move (index)) {}
    Location &location (void) { return loc; }
    void print (std::ostream &os) const;
    bool is_lvalue (void) { return true; }
  };

  class MemberAccessAST : public ExprAST
  {
  public:
    Location loc;
    ExprPtr operand;
    std::string member;
    bool deref;

    MemberAccessAST (Location loc, ExprPtr operand, std::string member,
		     bool deref) :
      loc (loc), operand (std::move (operand)), member (member),
      deref (deref) {}
    Location &location (void) { return loc; }
    void print (std::ostream &os) const;
    bool is_lvalue (void) { return true; }
  };

  class VariableAST : public ExprAST
  {
  public:
    Location loc;
    std::string name;

    VariableAST (Location loc, std::string name) : loc (loc), name (name) {}
    Location &location (void) { return loc; }
    void print (std::ostream &os) const;
    bool is_lvalue (void) { return true; }
  };

  class UnaryAST : public ExprAST
  {
  public:
    Location loc;
    UnaryOperator op;
    ExprPtr operand;

    UnaryAST (Location loc, UnaryOperator op, ExprPtr operand) :
      loc (loc), op (op), operand (std::move (operand)) {}
    Location &location (void) { return loc; }
    void print (std::ostream &os) const;
    bool is_lvalue (void) { return op == UnaryOperator::Dereference; }
  };

  class BinaryAST : public ExprAST
  {
  public:
    Location loc;
    BinaryOperator op;
    ExprPtr lhs;
    ExprPtr rhs;

    BinaryAST (Location loc, BinaryOperator op, ExprPtr lhs, ExprPtr rhs) :
      loc (loc), op (op), lhs (std::move (lhs)), rhs (std::move (rhs)) {}
    Location &location (void) { return loc; }
    void print (std::ostream &os) const;
    bool is_lvalue (void) { return false; }
  };

  class ExprStmtAST : public StatementAST
  {
  public:
    Location loc;
    ExprPtr expr;

    ExprStmtAST (Location loc, ExprPtr expr) :
      loc (loc), expr (std::move (expr)) {}
    Location &location (void) { return loc; }
    void print (std::ostream &os) const;
  };

  class ReturnAST : public StatementAST
  {
  public:
    Location loc;
    ExprPtr value;

    ReturnAST (Location loc, ExprPtr value) :
      loc (loc), value (std::move (value)) {}
    Location &location (void) { return loc; }
    void print (std::ostream &os) const;
  };

  class BlockAST : public StatementAST
  {
  public:
    Location loc;
    std::vector <StatementPtr> body;
    unsigned int indent;

    BlockAST (Location loc, std::vector <StatementPtr> body,
	      unsigned int indent) :
      loc (loc), body (std::move (body)), indent (indent) {}
    Location &location (void) { return loc; }
    void print (std::ostream &os) const;
  };

  class VariableDeclarationAST : public StatementAST, public FileScopeDeclAST
  {
  public:
    Location loc;
    TypePtr type;
    std::string name;
    ExprPtr initval;

    VariableDeclarationAST (Location loc, TypePtr type, std::string name) :
      loc (loc), type (std::move (type)), name (name) {}
    Location &location (void) { return loc; }
    void print (std::ostream &os) const;
  };

  class FuncDeclarationAST : public FileScopeDeclAST
  {
  public:
    Location loc;
    TypePtr rettype;
    std::string name;
    std::vector <TypePtr> params;
    bool empty_params;

    FuncDeclarationAST (Location loc, TypePtr rettype, std::string name,
			std::vector <TypePtr> params, bool empty_params) :
      loc (loc), rettype (std::move (rettype)), name (name),
      params (std::move (params)), empty_params (empty_params) {}
    Location &location (void) { return loc; }
    void print (std::ostream &os) const;
  };

  class FuncDefinitionAST : public FileScopeDeclAST
  {
  public:
    Location loc;
    TypePtr rettype;
    std::string name;
    std::vector <std::pair <TypePtr, std::string>> params;
    bool empty_params;
    std::unique_ptr <BlockAST> body;

    FuncDefinitionAST (Location loc, TypePtr rettype, std::string name,
		       std::vector <std::pair <TypePtr, std::string>> params,
		       bool empty_params, std::unique_ptr <BlockAST> body) :
      loc (loc), rettype (std::move (rettype)), name (name),
      params (std::move (params)), empty_params (empty_params),
      body (std::move (body)) {}
    Location &location (void) { return loc; }
    void print (std::ostream &os) const;
  };
}

std::ostream &operator<< (std::ostream &os, const socc::AST &ast);

#endif
