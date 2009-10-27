#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include <Efreet.h>

#include "libkeys.h"

/* Debugging */

#define LIBKEYS_DEBUG_LOOKUP 1 /* Keys lookup */
#define LIBKEYS_DEBUG_CONFIGS 2 /* Configuration files */

static int _libkeys_debug_level;

/* Config files */

#define SHARE_DIR DATADIR "/keys/"
#define CONFIG_DIR SYSCONFDIR "/keys/"

#define CONFIG_NUM 3

struct keys_t
{
    const char* app_name;
    Efreet_Ini* configs[CONFIG_NUM];
};



inline static void log(int level, const char* fmt, ...)
{
    if(level & _libkeys_debug_level)
    {
        fprintf(stderr, "libkeys: ");
        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        fprintf(stderr, "\n");
    }
}

keys_t* keys_alloc(const char* app_name)
{
    if(getenv("LIBKEYS_DEBUG"))
        _libkeys_debug_level = atoi(getenv("LIBKEYS_DEBUG"));

    log(LIBKEYS_DEBUG_CONFIGS, "%s: init", app_name);

    if(!efreet_init())
        return NULL;

    char configs[CONFIG_NUM][PATH_MAX];

    int r = snprintf(configs[0], PATH_MAX,
                     "%s/.keys/%s.ini", getenv("HOME"), app_name);
    if(r < 0 || r >= PATH_MAX)
        return NULL;
    r = snprintf(configs[1], PATH_MAX, CONFIG_DIR "%s.ini", app_name);
    if(r < 0 || r >= PATH_MAX)
        return NULL;
    r = snprintf(configs[2], PATH_MAX, SHARE_DIR "%s.ini", app_name);
    if(r < 0 || r >= PATH_MAX)
        return NULL;

    keys_t* keys = malloc(sizeof(keys_t));
    if(!keys)
        return NULL;

    if(!(keys->app_name = strdup(app_name)))
        return NULL;

    for(int i = 0; i < CONFIG_NUM; ++i)
    {
        keys->configs[i] = efreet_ini_new(configs[i]);

        log(LIBKEYS_DEBUG_CONFIGS, "%s: config[%d] -> %s%s", app_name, i,
            keys->configs[i] ? "" : "(not found) ", configs[i]);
    }

    for(int i = 0; i < CONFIG_NUM; ++i)
        if(keys->configs[i])
            return keys;

    free(keys);
    return NULL;
}

static const char* _keys_lookup_ini(Efreet_Ini* ini,
                                    const char* context, const char* key)
{
    if(!ini) return NULL;
    Eina_Hash* section = eina_hash_find(ini->data, context);
    if(!section)
        return NULL;
    const char* action = eina_hash_find(section, key);
    if(!action)
        return NULL;
    return action;
}

const char* keys_lookup(const keys_t* keys,
                        const char* context, const char* key)
{
    log(LIBKEYS_DEBUG_LOOKUP, "%s: lookup %s/%s", keys->app_name, context, key);

    for(int i = 0; i < CONFIG_NUM; ++i)
    {
        const char* k = _keys_lookup_ini(keys->configs[i], context, key);
        if(k)
        {
            if(!*k)
            {
                log(LIBKEYS_DEBUG_LOOKUP, "[%d] -> <no action>", i);
                return NULL; /* Explicit 'no action' */
            }
            else
            {
                log(LIBKEYS_DEBUG_LOOKUP, "[%d] -> %s", i, k);
                return k;
            }
        }
        else
            log(LIBKEYS_DEBUG_LOOKUP, "[%d] -> <no match>", i);
    }
    log(LIBKEYS_DEBUG_LOOKUP, "    -> <no action>");
    return NULL;
}

const char* keys_lookup_by_event(const keys_t* keys,
                                 const char* context,
                                 const Evas_Event_Key_Up* event)
{
    const char* k = event->keyname;

    if(evas_key_modifier_is_set(event->modifiers, "Alt"))
    {
        char key[512];
        snprintf(key, 512, "Hold-%s", k);
        return keys_lookup(keys, context, key);
    }

    return keys_lookup(keys, context, k);
}

void keys_free(keys_t* keys)
{
    log(LIBKEYS_DEBUG_CONFIGS, "%s: free", keys->app_name);

    for(int i = 0; i < CONFIG_NUM; ++i)
        efreet_ini_free(keys->configs[i]);
    free(keys->app_name);
    free(keys);

    efreet_shutdown();
}
