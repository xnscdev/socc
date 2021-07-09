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

#include <memory>
#include "location.hh"

namespace socc
{
  enum class TypeType
  {
    Primitive,
    Pointer
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
    Local,
    Cast
  };

  class Type;
  typedef std::unique_ptr <Type> TypePtr;

  class Type
  {
    size_t primitive_width (void);

  public:
    TypeType type;
    Location loc;
    StorageClass storage;
    TypeContext ctx;
    bool is_const;
    bool is_volatile;
    PrimitiveType primitive;
    TypePtr pointer;

    Type (Location loc, PrimitiveType type) :
      type (TypeType::Primitive), loc (loc), primitive (type) {}
    Type (Location loc, TypePtr type) :
      type (TypeType::Pointer), loc (loc), pointer (std::move (type)) {}
    size_t width (void);
  };
}

#endif
