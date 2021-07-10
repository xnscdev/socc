/* type.cc -- This file is part of SOCC.
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

#include <unordered_map>
#include "config.h"
#include "context.hh"
#include "type.hh"

using namespace socc;

static std::unordered_map <PrimitiveType, std::string> primitive_names = {
  {PrimitiveType::Char, "char"},
  {PrimitiveType::Short, "short"},
  {PrimitiveType::Int, "int"},
  {PrimitiveType::Long, "long"},
  {PrimitiveType::LongLong, "long long"},
  {PrimitiveType::Float, "float"},
  {PrimitiveType::Double, "double"},
  {PrimitiveType::LongDouble, "long double"},
  {PrimitiveType::Void, "void"}
};

std::map <std::string, std::vector <TypePtr>> socc::struct_types;
std::map <std::string, TypePtr> socc::typedefs;

TypePtr
Context::parse_type (Location loc, TypeContext ctx)
{
  bool seen_int = false;
  bool finish = false;
  bool is_const = false;
  bool is_volatile = false;
  StorageClass storage = StorageClass::Unspecified;
  TypePtr type = nullptr;

  int sign = 0;
  int primitive = 0;
  PrimitiveType primtype = PrimitiveType::Unspecified;

  while (!finish)
    {
      TokenPtr token = next_token ();
      if (token == nullptr)
	return nullptr;
      switch (token->type)
	{
	case TokenType::KeywordUnsigned:
	  if (primitive == -1)
	    error (token->loc, "expected type modifier or identifier");
	  else if (sign)
	    error (token->loc, "multiple sign modifiers specified");
	  else
	    {
	      sign = 1;
	      primitive = 1;
	    }
	  break;
	case TokenType::KeywordSigned:
	  if (primitive == -1)
	    error (token->loc, "expected type modifier or identifier");
	  else if (sign)
	    error (token->loc, "multiple sign modifiers specified");
	  else
	    {
	      sign = -1;
	      primitive = 1;
	    }
	  break;
	case TokenType::KeywordConst:
	  is_const = true;
	  break;
	case TokenType::KeywordVolatile:
	  is_volatile = true;
	  break;
	case TokenType::KeywordChar:
	  if (primitive == -1)
	    error (token->loc, "expected type modifier or identifier");
	  else if (primtype != PrimitiveType::Unspecified)
	    error (token->loc, "multiple base types specified");
	  else
	    {
	      primtype = PrimitiveType::Char;
	      primitive = 1;
	    }
	  break;
	case TokenType::KeywordShort:
	  if (primitive == -1)
	    error (token->loc, "expected type modifier or identifier");
	  else if (primtype != PrimitiveType::Unspecified
		   && primtype != PrimitiveType::Int)
	    error (token->loc, "multiple base types specified");
	  else
	    {
	      primtype = PrimitiveType::Short;
	      primitive = 1;
	    }
	  break;
	case TokenType::KeywordInt:
	  if (primitive == -1)
	    error (token->loc, "expected type modifier or identifier");
	  else if ((primtype != PrimitiveType::Unspecified
		    && primtype != PrimitiveType::Short
		    && primtype != PrimitiveType::Long
		    && primtype != PrimitiveType::LongLong) || seen_int)
	    error (token->loc, "multiple base types specified");
	  if (primtype == PrimitiveType::Unspecified)
	    {
	      primtype = PrimitiveType::Int;
	      primitive = 1;
	    }
	  seen_int = true;
	  break;
	case TokenType::KeywordLong:
	  if (primitive == -1)
	    error (token->loc, "expected type modifier or identifier");
	  else if (primtype == PrimitiveType::Long)
	    {
	      primtype = PrimitiveType::LongLong;
	      primitive = 1;
	    }
	  else if (primtype == PrimitiveType::Unspecified
		   || primtype == PrimitiveType::Int)
	    {
	      primtype = PrimitiveType::Long;
	      primitive = 1;
	    }
	  else
	    error (token->loc, "multiple base types specified");
	  break;
	case TokenType::KeywordVoid:
	  if (type || primitive != 0 || primtype != PrimitiveType::Unspecified)
	    error (token->loc, "expected type modifier or identifier");
	  else
	    {
	      primitive = -1;
	      if (sign != 0)
		error (token->loc, bold ("void") + "specifier with " +
		       bold (sign == 1 ? "unsigned" : "signed"));
	      type = std::make_shared <Type> (PrimitiveType::Void, false);
	      type->is_const = is_const;
	      type->is_volatile = is_volatile;
	      is_const = false;
	      is_volatile = false;
	    }
	  break;
	case TokenType::KeywordAuto:
	  if (ctx != TypeContext::Local)
	    error (token->loc, "storage class " + bold ("auto") +
		   " is invalid in this context");
	  else if (storage != StorageClass::Unspecified)
	    error (token->loc, "multiple storage classes specified");
	  else
	    storage = StorageClass::Auto;
	  break;
	case TokenType::KeywordStatic:
	  if (ctx == TypeContext::FuncParam || ctx == TypeContext::Cast)
	    error (token->loc, "storage class " + bold ("static") +
		   " is invalid in this context");
	  else if (storage != StorageClass::Unspecified)
	    error (token->loc, "multiple storage classes specified");
	  else
	    storage = StorageClass::Static;
	  break;
	case TokenType::KeywordExtern:
	  if (ctx == TypeContext::FuncParam || ctx == TypeContext::Cast)
	    error (token->loc, "storage class " + bold ("static") +
		   " is invalid in this context");
	  else if (storage != StorageClass::Unspecified)
	    error (token->loc, "multiple storage classes specified");
	  else
	    storage = StorageClass::Extern;
	  break;
	case TokenType::KeywordRegister:
	  if (ctx != TypeContext::Local)
	    error (token->loc, "storage class " + bold ("register") +
			" is invalid in this context");
	  else if (storage != StorageClass::Unspecified)
	    error (token->loc, "multiple storage classes specified");
	  else
	    storage = StorageClass::Register;
	  break;
	case TokenType::Mul:
	  if (primitive == 1)
	    type = std::make_shared <Type> (primtype, sign == 1);
	  primitive = -1;
	  type->is_const = is_const;
	  type->is_volatile = is_volatile;
	  is_const = false;
	  is_volatile = false;
	  type = std::make_shared <Type> (std::move (type));
	  break;
	default:
	  token_stack.push (std::move (token));
	  finish = true;
	}
    }
  if (primitive == 1)
    type = std::make_shared <Type> (primtype, sign == 1);
  else if (!type)
    return nullptr;
  if (type->type == TypeType::Primitive
      && type->primitive == PrimitiveType::Void
      && ctx != TypeContext::FuncReturn)
    {
      error (loc, "use of " + bold ("void") +
	     " type is invalid in this context");
      return nullptr;
    }
  type->is_const = is_const;
  type->is_volatile = is_volatile;
  type->storage = storage;
  return type;
}

size_t
Type::primitive_width (void)
{
  switch (primitive)
    {
    case PrimitiveType::Int:
      return 4;
    case PrimitiveType::Char:
      return 1;
    case PrimitiveType::Short:
      return 2;
    case PrimitiveType::Long:
      return LP_WIDTH;
    case PrimitiveType::LongLong:
      return 8;
    case PrimitiveType::Float:
      return 4;
    case PrimitiveType::Double:
      return 8;
    case PrimitiveType::LongDouble:
      return 16;
    default:
      return 0;
    }
}

size_t
Type::struct_width (void)
{
  size_t width = 0;
  if (struct_name.empty ())
    {
      for (TypePtr &type : params)
	width += type->width ();
    }
  else
    {
      if (struct_types.count (struct_name))
	{
	  for (TypePtr &type : struct_types[struct_name])
	    width += type->width ();
	}
      else
	return 0;
    }
  return width;
}

std::string
Type::primitive_name (void)
{
  std::string name;
  if (is_const)
    name += "const ";
  if (is_volatile)
    name += "volatile ";
  return name + primitive_names[primitive];
}

std::string
Type::pointer_name (void)
{
  std::string name = pointer->name ();
  if (name.back () != '*')
    name += ' ';
  name += '*';
  if (is_const)
    {
      name += "const";
      if (is_volatile)
	name += ' ';
    }
  if (is_volatile)
    name += "volatile";
  return name;
}

std::string
Type::function_name (void)
{
  std::string name = pointer->name () + "(*";
  if (is_const)
    {
      name += "const";
      if (is_volatile)
	name += ' ';
    }
  if (is_volatile)
    name += "volatile";
  name += ") (";
  if (params.empty ())
    name += "void";
  else
    {
      name += params[0]->name ();
      for (size_t i = 1; i < params.size (); i++)
	name += params[i]->name ();
    }
  return name + ')';
}

size_t
Type::width (void)
{
  switch (type)
    {
    case TypeType::Primitive:
      return primitive_width ();
    case TypeType::Pointer:
    case TypeType::Function:
      return LP_WIDTH;
    case TypeType::Struct:
      return struct_width ();
    default:
      return 0;
    }
}

std::string
Type::name (void)
{
  std::string name;
  switch (storage)
    {
    case StorageClass::Auto:
      name += "auto ";
      break;
    case StorageClass::Static:
      name += "static ";
      break;
    case StorageClass::Extern:
      name += "extern ";
      break;
    case StorageClass::Register:
      name += "register ";
      break;
    default:
      break;
    }
  switch (type)
    {
    case TypeType::Primitive:
      name += primitive_name ();
      break;
    case TypeType::Pointer:
      name += pointer_name ();
      break;
    case TypeType::Function:
      name += function_name ();
      break;
    case TypeType::Struct:
      name += "struct " + (struct_name.empty () ? "<anonymous> " : struct_name);
      break;
    default:
      return std::string ();
    }
  return name;
}
