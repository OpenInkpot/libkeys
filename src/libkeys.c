#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>

#include <Efreet.h>

#include "libkeys.h"

#define SHARE_DIR DATADIR "/keys/"
#define CONFIG_DIR SYSCONFDIR "/keys/"

#define CONFIG_NUM 3

struct keys_t
{
    Efreet_Ini* configs[CONFIG_NUM];
};

keys_t* keys_alloc(const char* app_name)
{
    if(!efreet_init())
        return NULL;

    char share_config_file[PATH_MAX];
    int r = snprintf(share_config_file, PATH_MAX, SHARE_DIR "%s.ini", app_name);
    if(r < 0 || r >= PATH_MAX)
        return NULL;
    char etc_config_file[PATH_MAX];
    r = snprintf(etc_config_file, PATH_MAX, CONFIG_DIR "%s.ini", app_name);
    if(r < 0 || r >= PATH_MAX)
        return NULL;
    char user_config_file[PATH_MAX];
    r = snprintf(user_config_file, PATH_MAX,
                 "%s/.keys/%s.ini", getenv("HOME"), app_name);
    if(r < 0 || r >= PATH_MAX)
        return NULL;

    keys_t* keys = malloc(sizeof(keys_t));
    if(!keys)
        return NULL;

    keys->configs[0] = efreet_ini_new(share_config_file);
    keys->configs[1] = efreet_ini_new(etc_config_file);
    keys->configs[2] = efreet_ini_new(user_config_file);

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
    for(int i = 0; i < CONFIG_NUM; ++i)
    {
        const char* k = _keys_lookup_ini(keys->configs[i], context, key);
        if(k)
        {
            if(!*k)
                return NULL; /* Explicit 'no action' */
            else
                return k;
        }
    }
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
    for(int i = 0; i < CONFIG_NUM; ++i)
        efreet_ini_free(keys->configs[i]);
    free(keys);

    efreet_shutdown();
}
