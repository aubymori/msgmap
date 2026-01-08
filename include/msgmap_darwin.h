#include <CoreFoundation/CoreFoundation.h>

EXTERN_IMPL bool mm_set_preferred_langs_from_system(void)
{
    bool ret = false;
    CFStringRef lang;
    char buffer[64];
    char *dash;
    int i;
    size_t locale_length;

    CFArrayRef languages = CFLocaleCopyPreferredLanguages();
    if (!languages)
        return false;

    int count = CFArrayGetCount(languages);
    if (count <= 0)
        goto cleanup;

    mm_preferred_lang_t *new_langs = (mm_preferred_lang_t *)malloc(sizeof(mm_preferred_lang_t) * count);

    for (i = 0; i < count; i++)
    {
        lang = (CFStringRef)CFArrayGetValueAtIndex(languages, i);
        if (!CFStringGetCString(lang, buffer, sizeof(buffer), kCFStringEncodingUTF8))
            goto cleanup;

        mm_preferred_lang_t *pref_lang = &new_langs[i];

        dash = (char *)strchr(buffer, '-');
        if (dash)
        {
            strncpy(pref_lang->lang, buffer, (size_t)(dash - buffer));
            strcpy(pref_lang->region, dash + 1);
        }
        else
        {
            strcpy(pref_lang->lang, buffer);
            pref_lang->region[0] = '\0';
        }
    }

    ret = true;
    if (g_preferred_langs)
        free(g_preferred_langs);

    g_preferred_langs = new_langs;
    g_preferred_lang_count = count;

cleanup:
    if (languages)
        CFRelease(languages);

    return ret;
}