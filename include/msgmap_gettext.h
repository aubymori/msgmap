#include <locale.h>

EXTERN_IMPL bool mm_set_preferred_langs_from_system(void)
{
    setlocale(LC_MESSAGES, "");
    char *locale = setlocale(LC_MESSAGES, NULL);

    mm_preferred_lang_t lang;

    char *underscore = (char *)strchr(locale, '_');
    char *period     = (char *)strchr(locale, '.');

    if (underscore)
    {
        size_t lang_length = (size_t)(underscore - locale);
        if (lang_length >= sizeof(lang.lang))
            return false;

        strncpy(lang.lang, locale, lang_length);

        char *region = underscore + 1;
        size_t region_length = period ? (size_t)(period - region) : strlen(region);
        if (region_length >= sizeof(lang.region))
            return false;

        strncpy(lang.region, region, region_length);
    }
    else
    {
        size_t lang_length = period ? (size_t)(period - locale) : strlen(locale);
        if (lang_length >= sizeof(lang.lang))
            return false;

        strncpy(lang.lang, locale, lang_length);
        lang.region[0] = '\0';
    }

    mm_preferred_lang_t *new_lang = (mm_preferred_lang_t *)malloc(sizeof(mm_preferred_lang_t));
    if (!new_lang)
        return false;
    memcpy(new_lang, &lang, sizeof(mm_preferred_lang_t));

    if (g_preferred_langs)
        free(g_preferred_langs);

    g_preferred_langs = new_lang;
    g_preferred_lang_count = 1;

    return true;
}