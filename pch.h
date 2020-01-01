#pragma once

#define _in
#define _out

#define trace(...)
#define set_lasterror(x) x

typedef unsigned char byte_t;
typedef wchar_t char_t;

typedef wchar_t char_t;
typedef char_t *str_t;
typedef const char_t *cstr_t;

#include <string>
typedef std::wstring string;