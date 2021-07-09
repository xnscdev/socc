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
  switch (type)
    {
    case TypeType::Primitive:
      return primitive_name ();
    case TypeType::Pointer:
      return pointer_name ();
    case TypeType::Function:
      return function_name ();
    case TypeType::Struct:
      return "struct " + (struct_name.empty () ? "<anonymous> " : struct_name);
    default:
      return std::string ();
    }
}
