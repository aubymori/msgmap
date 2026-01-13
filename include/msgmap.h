#ifndef _MSGMAP_H
#define _MSGMAP_H

#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#ifdef _MSC_VER
    #define mm_snprintf(buffer, count, format, ...) \
        _snprintf(buffer, count, format, __VA_ARGS__)
    #define mm_snwprintf(buffer, count, format, ...) \
        _snwprintf(buffer, count, format, __VA_ARGS__)
#else
    #define mm_snprintf(buffer, count, format, ...) \
        snprintf(buffer, count, format, __VA_ARGS__)
#endif

#ifdef __cplusplus
    #define MM_DEC   extern "C"
    #define MM_IMPL  extern "C"
#else
    #define MM_DEC   extern
    #define MM_IMPL  
#endif

#define MM_FORMATTED_STRING_BODY(format_str, ...) \
    static const char *format = format_str; \
    size_t length = mm_snprintf(NULL, 0, format, __VA_ARGS__); \
    char *buffer = (char *)malloc(length + 1); \
    if (!buffer) \
        return NULL; \
    buffer[length] = '\0'; \
    mm_snprintf(buffer, length, format, __VA_ARGS__); \
    return buffer;

#define MM_FORMATTED_STRING_BODY_W(format_str, ...) \
    static const wchar_t *format = format_str; \
    size_t length = mm_snwprintf(NULL, 0, format, __VA_ARGS__); \
    wchar_t *buffer = (wchar_t *)malloc((length + 1) * sizeof(wchar_t)); \
    if (!buffer) \
        return NULL; \
    buffer[length] = L'\0'; \
    mm_snwprintf(buffer, length, format, __VA_ARGS__); \
    return buffer;

typedef struct _mm_preferred_lang_t
{
    char lang[32];
    char region[32];
} mm_preferred_lang_t;

typedef struct _mm_translation_mapping_t
{
    void *translations;
    const char *lang;
    const char *region;
} mm_translation_mapping_t;

//
// Sets the list preferred languages.
//
// Arguments:
//   preferred_langs: Pointer to array of string pointers that includes
//     the new languages, in order from most to least preferred.
//
//     Each language must consist of a language indicator with only
//     lowercase Latin letters optionally followed by an underscore
//     followed by a region indicator with only uppercase Latin letters.
//     The language and region must both be 31 characters or less.
//     
//     Examples: en, en_US
//
//   preferred_lang_count: Number of elements in the preferred_langs
//     array.
//
// Return value:
//   true if succeeded, false if failed.
//
MM_DEC bool mm_set_preferred_langs(const char **preferred_langs, size_t preferred_lang_count);

//
// Clears the list of preferred languages.
//
MM_DEC void mm_clear_preferred_langs(void);

//
// Retrieves the list of preferred languages from
// the system.
// 
// Return value:
//   true if succeeded, false if failed.
//
MM_DEC bool mm_set_preferred_langs_from_system(void);

#ifdef __cplusplus

#include <string>

//
// Converts a formatted string into a C++ string
// and frees the original buffer.
//
// Arguments:
//   str: Pointer to the formatted string.
//
// Return value:
//   The converted C++ string
//
inline std::string mm_cpp_string(char *str)
{
    std::string cppstr = str;
    free(str);
    return cppstr;
}

#endif

#ifdef MSGMAP_IMPL

// User preferred languages in order from most to least
// preferred.
mm_preferred_lang_t *g_preferred_langs = NULL;
size_t g_preferred_lang_count = 0;

inline void mm_strncpy(char *buffer, const char *source, size_t count)
{
    strncpy(buffer, source, count);
    buffer[count] = '\0';
}

MM_IMPL bool mm_set_preferred_langs(
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

MM_IMPL void mm_clear_preferred_langs(void)
{
    if (g_preferred_langs)
    {
        free(g_preferred_langs);
        g_preferred_langs = NULL;
        g_preferred_lang_count = 0;
    }
}


inline void *mm_get_translations(
    const mm_translation_mapping_t *map,
    size_t map_length)
{
    if (!map || !map_length)
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
            if (!strcmp(entry->lang, lang->lang))
            {
                lang_match = entry;
                if (entry->region && !strcmp(entry->region, lang->region))
                {
                    return entry->translations;
                }

                if ((!entry->region || !entry->region[0]) && !lang->region[0])
                {
                    return entry->translations;
                }
            }
        }

        if (lang_match)
            return lang_match->translations;
    }

    return map[0].translations;
}

#if defined(_WIN32) || defined (_WIN64)
    #include "msgmap_win.h"
#elif defined(__APPLE__) && defined(__MACH__)
    #include "msgmap_darwin.h"
#else 
    // If not Windows or macOS, assume we are on Linux or FreeBSD
    // and try to get the language the way GNU gettext does.
    #include <locale.h>

    #ifndef LC_MESSAGES
        #error "No suitable mm_set_preferred_langs_from_system implementation"
    #endif

    #include "msgmap_gettext.h"
#endif

#endif // MSGMAP_IMPL
    
#endif // _MSGMAP_H