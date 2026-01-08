#ifndef _MSGMAP_H
#define _MSGMAP_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#ifdef _MSC_VER
    #ifdef _CRT_SECURE_NO_WARNINGS
        #define mm_snprintf(buffer, count, format, ...) \
            _snprintf(buffer, count, format, __VA_ARGS__)
    #else
        #define mm_snprintf(buffer, count, format, ...) \
            _snprintf_s(buffer, (count + 1), count, format, __VA_ARGS__)
    #endif
#else
    #define mm_snprintf(buffer, count, format, ...) \
        snprintf(buffer, count, format, __VA_ARGS__)
#endif

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
    #define mm_strncpy(buffer, source, count) \
        strncpy_s(buffer, (count + 1), source, count)
#else
    #define mm_strncpy(buffer, source, count) \
        strncpy(buffer, source, count)
#endif

// MSVC does not like the POSIX name for this function.
#ifdef _MSC_VER
    #define stricmp _stricmp
#endif

typedef struct _mm_preferred_lang_t
{
    char lang[32];
    char region[32];
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
//     the new languages, in order from most to least preferred.
//
//     Each language must consist of a language indicator with only
//     lowercase Latin letters optionally followed by an underscore
//     followed by a region indicator with only uppercase Latin letters.
//     The entire string must be 63 characters or less. 
//     
//     Examples: en, en_US
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
    if (!preferred_langs || !preferred_lang_count)
        return false;

    mm_preferred_lang_t *new_langs = (mm_preferred_lang_t *)malloc(sizeof(mm_preferred_lang_t) * preferred_lang_count);
    if (!new_langs)
        return false;

    for (size_t i = 0; i < preferred_lang_count; i++)
    {
        char *underscore;
        const char *lang = preferred_langs[i];
        size_t j, lang_length, region_length;
        mm_preferred_lang_t *pref_lang = &new_langs[i];

        if (!lang[0])
            goto fail;

        underscore = (char *)strchr(lang, '_');
        if (!underscore)
        {
            lang_length = strlen(lang);
            if (lang_length >= sizeof(pref_lang->lang))
                goto fail;
            region_length = 0;

            mm_strncpy(pref_lang->lang, lang, lang_length);
            pref_lang->region[0] = '\0';
        }
        else
        {
            lang_length = (size_t)(underscore - lang);
            if (lang_length >= sizeof(pref_lang->lang))
                goto fail;

            region_length = strlen(underscore + 1);
            if (region_length >= sizeof(pref_lang->region))
                goto fail;

            mm_strncpy(pref_lang->lang, lang, lang_length);
            mm_strncpy(pref_lang->region, underscore + 1, region_length);
        }

        for (j = 0; j < lang_length; j++)
        {
            if (pref_lang->lang[j] < 'a' || pref_lang->lang[j] > 'z')
                goto fail;
        }

        for (j = 0; j < region_length; j++)
        {
            if (pref_lang->region[j] < 'A' || pref_lang->region[j] > 'Z')
                goto fail;
        }
    }

    if (g_preferred_langs)
        free(g_preferred_langs);

    g_preferred_langs = new_langs;
    g_preferred_lang_count = preferred_lang_count;

    return true;

fail:
    if (new_langs)
        free(new_langs);
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

typedef struct _mm_translation_mapping_t
{
    void *translations;
    const char *lang;
    const char *region;
} mm_translation_mapping_t;

//
// Not for direct consumption. Please use the functions provided by
// your generated header files.
//
inline void *mm_get_translations(
    const mm_translation_mapping_t *map,
    size_t map_length,
    size_t default_entry_index)
{
    if (!map || !map_length || default_entry_index >= map_length)
        return NULL;

    mm_translation_mapping_t *default_lang_entry = NULL;
    mm_translation_mapping_t *default_full_entry = NULL;

    for (size_t i = 0; i < g_preferred_lang_count; i++)
    {
        mm_preferred_lang_t            *lang       = &g_preferred_langs[i];
        const mm_translation_mapping_t *lang_match = NULL;

        for (size_t j = 0; j < map_length; j++)
        {
            const mm_translation_mapping_t *entry = &map[j];
            if (!stricmp(entry->lang, lang->lang))
            {
                lang_match = entry;
                if (entry->region && lang->region && !stricmp(entry->region, lang->region))
                {
                    return entry->translations;
                }

                if ((!entry->region || !entry->region[0]) && (!lang->region || !lang->region[0]))
                {
                    return entry->translations;
                }
            }
        }

        if (lang_match)
            return lang_match->translations;
    }

    return map[default_entry_index].translations;
}

//
// Sets the preferred languages from the system.
// 
// Return value:
//   true if succeeded, false if failed.
//
inline bool mm_set_preferred_langs_from_system(void);

#if defined(_WIN32) || defined (_WIN64)
    #include "msgmap_win.h"
#endif

#endif