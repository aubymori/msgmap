#ifndef _MSGMAP_H
#define _MSGMAP_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

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

typedef struct _mm_preferred_lang_t
{
    char   locale[64];
    // No lang_index is needed since it will always
    // be 0
    size_t lang_length;
    size_t region_index;
    size_t region_length;
} mm_preferred_lang_t;

// User preferred languages in order from most to least
// preferred.
mm_preferred_lang_t *g_preferred_langs = NULL;
size_t g_preferred_lang_count = 0;

//
// Sets the preferred languages.
//
// Arguments:
//   preferred_langs: Pointer to array of string pointers that includes
//     the new languages.
//
//   preferred_lang_count: Number of elements in the preferred_langs
//     array.
//
// Return value:
//   true if succeeded, false if failed.
//
inline bool mm_set_preferred_langs(
    const char **preferred_langs,
    size_t preferred_lang_count)
{
    // TODO(aubymori): Implement.
    return false;
}

inline void mm_clear_preferred_langs(void)
{
    if (g_preferred_langs)
    {
        free(g_preferred_langs);
        g_preferred_langs = NULL;
        g_preferred_lang_count = 0;
    }
}

typedef struct _mm_translation_entry_t
{
    void *table;
    const char *lang;
    const char *region
} mm_translation_entry_t;

inline void *mm_get_translation_table(
    mm_translation_entry_t *map,
    const char *default_lang,
    const char *default_region)
{
    // TODO(aubymori): Implement.
    return NULL;
}

#endif