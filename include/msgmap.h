#ifndef _MSGMAP_H
#define _MSGMAP_H

#include <stdlib.h>
#include <stdio.h>

#ifdef _MSC_VER
    #define WSTR_ARG  L"%s"
#else
    #define WSTR_ARG  L"%S"
#endif

#ifdef _MSC_VER
    #ifdef _CRT_SECURE_NO_WARNINGS
        #define mm_snprintf(buffer, count, format, ...) \
            _snprintf(buffer, count, format, __VA_ARGS__)
    #else
        #define mm_snprintf(buffer, count, format, ...) \
            _snprintf_s(buffer, count, count, format, __VA_ARGS__)
    #endif
#else
    #define mm_snprintf(buffer, count, format, ...) \
        snprintf(buffer, count, format, __VA_ARGS__)
#endif