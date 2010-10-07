/* HornetsEye - Computer Vision with Ruby
   Copyright (C) 2006, 2007, 2008, 2009, 2010   Jan Wedekind

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */
#ifndef ERROR_HH
#define ERROR_HH

#include <exception>
#include <sstream>

class Error: public std::exception
{
public:
  Error(void) {}
  Error( Error &e ): std::exception( e )
    { m_message << e.m_message.str(); }
  virtual ~Error(void) throw() {}
  template< typename T >
  std::ostream &operator<<( const T &t )
    { m_message << t; return m_message; }
  std::ostream &operator<<( std::ostream& (*__pf)(std::ostream&) )
    { (*__pf)( m_message ); return m_message; }
  virtual const char* what(void) const throw() {
    temp = m_message.str();
    return temp.c_str();
  }
protected:
  std::ostringstream m_message;
  mutable std::string temp;
};

#define ERRORMACRO( condition, class, params, message ) \
  if ( !( condition ) ) {                               \
    class _e params;                                    \
    _e << message;                                      \
    throw _e;                                           \
  };

#endif
