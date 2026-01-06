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

// MSVC does not like the POSIX name for this function.
#ifdef _MSC_VER
    #define strnicmp _strnicmp
#endif

// region_index will be set to this value when
// the locale string contains no region code
#define MM_NO_REGION ((size_t)-1)

typedef struct _mm_preferred_lang_t
{
    char   locale[64];
    // No lang_index is needed since it will always
    // be 0
    size_t lang_length;
    size_t region_index;
    // No region_length is needed since it's always
    // the last part of the string
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

    mm_preferred_lang_t *new_langs = malloc(sizeof(mm_preferred_lang_t) * preferred_lang_count);
    if (!new_langs)
        return false;

    for (size_t i = 0; i < preferred_lang_count; i++)
    {
        char *underscore;
        const char *lang = preferred_langs[i];
        size_t j;
        mm_preferred_lang_t *pref_lang = &new_langs[i];

        if (!lang[0])
            goto fail;

        if (strlen(lang) >= sizeof(pref_lang->locale))
            goto fail;

        underscore = (char *)strchr(lang, '_');
        if (!underscore)
        {
            pref_lang->region_index = MM_NO_REGION;
            pref_lang->lang_length  = strlen(lang);
        }
        else
        {
            pref_lang->region_index = (size_t)((underscore + 1) - lang);
            pref_lang->lang_length  = (size_t)(underscore - lang - 1);
        }

        for (j = 0; j < pref_lang->lang_length; j++)
        {
            if (lang[i] < 'a' || lang[i] > 'z')
                goto fail;
        }

        if (underscore)
        {
            for (j = pref_lang->region_index; lang[i] != '\0'; j++)
            {
                if (lang[i] < 'A' || lang[i] > 'Z')
                    goto fail;
            }
        }
    }

    if (g_preferred_langs)
    {
        free(g_preferred_langs);
    }

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

typedef struct _mm_translation_entry_t
{
    void *table;
    const char *lang;
    const char *region
} mm_translation_entry_t;

inline void *mm_get_translation_table(
    mm_translation_entry_t *map,
    size_t default_entry_index)
{
    if (!map)
        return NULL;

    mm_translation_entry_t *default_lang_entry = NULL;
    mm_translation_entry_t *default_full_entry = NULL;

    for (size_t i = 0; i < g_preferred_lang_count; i++)
    {
        mm_preferred_lang_t    *lang       = &g_preferred_langs[i];
        mm_translation_entry_t *entry      = map;
        mm_translation_entry_t *lang_match = NULL;

        const char *region = NULL;
        size_t region_length = 0;
        if (lang->region_index != MM_NO_REGION)
        {
            region = &lang->locale[lang->region_index];
            region_length = strlen(region);
        }

        while (entry->table)
        {
            if (!strnicmp(entry->lang, lang->locale, lang->lang_length))
            {
                lang_match = entry;
                if (region
                && !strnicmp(entry->lang, region, region_length))
                {
                    return entry->table;
                }
            }

            if (lang_match)
                return lang_match->table;

            entry++;
        }
    }

    return NULL;
}

#endif