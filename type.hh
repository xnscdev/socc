/* type.hh -- This file is part of SOCC.
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

#ifndef _TYPE_HH
#define _TYPE_HH

#include <vector>
#include <map>
#include <memory>
#include "location.hh"

namespace socc
{
  enum class TypeType
  {
    Primitive,
    Pointer,
    Array,
    Function,
    Struct
  };

  enum class PrimitiveType
  {
    Unspecified,
    Char,
    Short,
    Int,
    Long,
    LongLong,
    Float,
    Double,
    LongDouble,
    Void
  };

  enum class StorageClass
  {
    Unspecified,
    Auto,
    Static,
    Extern,
    Register
  };

  enum class TypeContext
  {
    FileScope,
    FuncReturn,
    FuncParam,
    Local,
    Cast
  };

  class Type;
  typedef std::shared_ptr <Type> TypePtr;

  class Type
  {
    size_t primitive_width (void);
    size_t struct_width (void);
    std::string primitive_name (void);
    std::string pointer_name (void);
    std::string function_name (void);

  public:
    TypeType type;
    StorageClass storage;
    TypeContext ctx;
    bool is_const;
    bool is_volatile;
    bool is_unsigned;
    PrimitiveType primitive;
    TypePtr pointer; /* For pointer, array, and function return types */
    unsigned long len; /* For array size */
    std::vector <TypePtr> params; /* For function params and anonymous struct 
				     members */
    bool empty_params;
    std::string struct_name;

    Type (PrimitiveType type, bool is_unsigned) :
      type (TypeType::Primitive), is_unsigned (is_unsigned), primitive (type) {}
    Type (TypePtr type) :
      type (TypeType::Pointer), pointer (std::move (type)) {}
    Type (TypePtr type, unsigned long len) :
      type (TypeType::Array), pointer (std::move (type)), len (len) {}
    Type (TypePtr rettype, std::vector <TypePtr> params) :
      type (TypeType::Function), pointer (std::move (rettype)),
      params (std::move (params)) {}
    Type (std::vector <TypePtr> params) :
      type (TypeType::Struct), params (std::move (params)) {}
    Type (std::string struct_name) :
      type (TypeType::Struct), struct_name (struct_name) {}
    size_t width (void);
    std::string name (void);
  };

  extern std::map <std::string, std::vector <TypePtr>> struct_types;
  extern std::map <std::string, TypePtr> typedefs;
}

#endif
