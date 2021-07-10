/* main.cc -- This file is part of SOCC.
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

#include <iostream>
#include "context.hh"

int
main (int argc, char **argv)
{
  socc::init_console ();
  socc::Context ctx ("<stdin>", std::cin);
  while (1)
    {
      socc::StatementPtr st = ctx.next_statement ();
      if (st == nullptr)
	break;
      std::cout << st->location () << ": " << *st << std::endl;
    }
  return 0;
}
