/* context.hh -- This file is part of SOCC.
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

#ifndef _CONTEXT_HH
#define _CONTEXT_HH

#include <istream>
#include <stack>
#include "token.hh"

namespace socc
{
  class Context
  {
    std::stack <char> char_stack;
    std::stack <TokenPtr> token_stack;
    unsigned int errors;

    char next_char (void);
    bool next_char_escaped (char &c);
    TokenPtr scan_word (char c);
    TokenPtr scan_number (char c);
    TokenPtr scan_char (void);
    TokenPtr scan_string (void);

  public:
    Location currloc;
    std::istream &stream;

    Context (std::string name, std::istream &stream) :
      currloc (name), stream (stream) {}
    std::string bold (std::string str);
    void warning (Location loc, std::string msg, std::string option = "");
    void error (Location loc, std::string msg);
    TokenPtr next_token (void);
  };

  void init_console (void);
  void fatal_error (std::string msg, std::string option = "");
}

#endif
