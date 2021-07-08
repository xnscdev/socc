/* location.hh -- This file is part of SOCC.
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

#ifndef _LOCATION_HH
#define _LOCATION_HH

#include <ostream>
#include <string>

namespace socc
{
  class Location
  {
  public:
    std::string name;
    unsigned long line;
    unsigned long col;

    explicit Location (std::string name) : name (name), line (1), col (0) {}
    Location (std::string name, unsigned long line, unsigned long col) :
      name (name), line (line), col (col) {}
  };
}

std::ostream &operator<< (std::ostream &os, const socc::Location &loc);

#endif
