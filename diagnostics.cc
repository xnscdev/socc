/* diagnostics.cc -- This file is part of SOCC.
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
#include <unistd.h>
#include "context.hh"

static bool use_color;

std::string
socc::Context::bold (std::string str)
{
  return use_color ? "\033[1m" + str + "\033[0m" : "\"" + str + "\"";
}

void
socc::Context::warning (socc::Location loc, std::string msg, std::string option)
{
  if (use_color)
    std::cerr << "\033[35;1mwarning: \033[39m";
  else
    std::cerr << "warning: ";
  std::cerr << loc;
  if (use_color)
    std::cerr << ":\033[0m ";
  else
    std::cerr << ": ";
  std::cerr << msg;
  if (option.empty ())
    std::cerr << std::endl;
  else if (use_color)
    std::cerr << " [\033[35;1m" << option << "\033[0m]" << std::endl;
  else
    std::cerr << " [" << option << ']' << std::endl;
}

void
socc::Context::error (socc::Location loc, std::string msg)
{
  if (use_color)
    std::cerr << "\033[31;1merror: \033[39m";
  else
    std::cerr << "error: ";
  std::cerr << loc;
  if (use_color)
    std::cerr << ":\033[0m ";
  else
    std::cerr << ": ";
  std::cerr << msg << std::endl;
  errors++;
}

void
socc::init_console (void)
{
  if (isatty (STDERR_FILENO))
    use_color = true;
}

void
socc::fatal_error (std::string msg, std::string option)
{
  if (use_color)
    std::cerr << "\033[31;1mfatal error: \033[0m";
  else
    std::cerr << "fatal error: ";
  std::cerr << msg;
  if (option.empty ())
    std::cerr << std::endl;
  else if (use_color)
    std::cerr << " [\033[31;1m" << option << "\033[0m]" << std::endl;
  else
    std::cerr << " [" << option << ']' << std::endl;
  exit (1);
}

std::ostream &
operator<< (std::ostream &os, const socc::Location &loc)
{
  return os << loc.name << ':' << loc.line << '.' << loc.col;
}
