#ifndef _MSGMAP_WIN_H
#define _MSGMAP_WIN_H

#include <wchar.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>

inline bool mm_set_preferred_langs_from_system(void)
{
    WCHAR szLanguages[MAX_PATH];
    LPWSTR pszLanguages = szLanguages;
    ULONG  cchLanguages = ARRAYSIZE(szLanguages);
    ULONG  ulNumLanguages;

    if (!GetThreadPreferredUILanguages(
        MUI_LANGUAGE_NAME, &ulNumLanguages, pszLanguages, &cchLanguages))
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            pszLanguages = (LPWSTR)malloc(sizeof(WCHAR) * cchLanguages);
            if (!pszLanguages)
                return false;

            if (!GetThreadPreferredUILanguages(
                MUI_LANGUAGE_NAME, &ulNumLanguages, pszLanguages, &cchLanguages))
            {
                return false;
            }
        }
        else
            return false;
    }

    bool ret = false;
    mm_preferred_lang_t *new_langs = (mm_preferred_lang_t *)malloc(sizeof(mm_preferred_lang_t) * ulNumLanguages);
    if (!new_langs)
        return false;

    LPWSTR pszLang = pszLanguages;
    for (ULONG i = 0; i < ulNumLanguages; i++)
    {
        mm_preferred_lang_t *lang = &new_langs[i];
        size_t length = wcslen(pszLang);
        size_t lang_length;

        LPWSTR pszDash = (LPWSTR)wcschr(pszLang, L'-');
        if (!pszDash)
        {
            if (!WideCharToMultiByte(CP_UTF8, 0, pszLang, -1, lang->lang, ARRAYSIZE(lang->lang), NULL, NULL))
                goto cleanup;
            lang->region[0] = '\0';
        }
        else
        {
            lang_length = (size_t)(pszDash - pszLang) ;
            if (!WideCharToMultiByte(CP_UTF8, 0, pszLang, lang_length, lang->lang, ARRAYSIZE(lang->lang), NULL, NULL))
                goto cleanup;
            // WideCharToMultiByte does not null terminate with a specified length...
            // correct it.
            lang->lang[lang_length] = L'\0';
            if (!WideCharToMultiByte(CP_UTF8, 0, pszDash + 1, -1, lang->region, ARRAYSIZE(lang->region), NULL, NULL))
                goto cleanup;
        }

        pszLang += length + 1;
    }

    ret = true;

    if (g_preferred_langs)
        free(g_preferred_langs);

    g_preferred_langs = new_langs;
    g_preferred_lang_count = ulNumLanguages;

cleanup:
    if (cchLanguages > ARRAYSIZE(szLanguages))
        free(pszLanguages);

    return ret;
}

#endif