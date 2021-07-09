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

#include "config.h"
#include "type.hh"

using namespace socc;

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
Type::width (void)
{
  switch (type)
    {
    case TypeType::Primitive:
      return primitive_width ();
    case TypeType::Pointer:
      return LP_WIDTH;
    default:
      return 0;
    }
}
