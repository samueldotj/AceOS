/*! 	\file	enum.h
	\brief	Converts enum values to corresponding string
	Copied from http://lists.pilot-link.org/pipermail/pilot-link-devel/2005-April/001344.html
*/

#ifndef _ENUM_H
#define _ENUM_H

#define ENUM_BODY(name, value)           \
    name value,
#define AS_STRING_CASE(name, value)      \
    case name: return #name;
#define FROM_STRING_CASE(name, value)    \
    if (strcmp(str, #name) == 0) {       \
        return name;                     \
    }
#define DEFINE_ENUM(name, list)          \
    typedef enum {                       \
        list(ENUM_BODY)                  \
    }name;
#define AS_STRING_DEC(name, list)        \
    const char* name##_AS_STRING(name n);
#define AS_STRING_FUNC(name, list)       \
    const char* name##_AS_STRING(name n) { \
        switch (n) {                     \
            list(AS_STRING_CASE)         \
            default: return "";          \
        }                                \
    }
#define FROM_STRING_DEC(name, list)      \
    name name##FromString(const char* str);
#define FROM_STRING_FUNC(name, list)     \
    name name##FromString(const char* str) {   \
        list(FROM_STRING_CASE)           \
        return 0;                        \
    }
	
#endif
