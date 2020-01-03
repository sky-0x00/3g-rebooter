#pragma once

#define _in
#define _out
#define _optional

#define trace(...)
#define set_lasterror(x) x

typedef unsigned char byte_t;
typedef wchar_t char_t;

typedef char char_at;
typedef unsigned char uchar_at;
typedef char_at *str_at;
typedef const char_at *cstr_at;

typedef wchar_t char_t;
typedef char_t *str_t;
typedef const char_t *cstr_t;

typedef unsigned argc_t;
typedef cstr_t argv_t[];

#include <string>
typedef std::wstring string_t;
typedef std::string string_at;

#define Winapi
typedef void *handle_t;