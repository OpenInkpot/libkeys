#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>

#include <Efreet.h>

#include "libkeys.h"

#define CONFIG_DIR SYSCONFDIR "/keys/"

struct keys_t
{
    Efreet_Ini* sys_config_file;
    Efreet_Ini* user_config_file;
};

keys_t* libkeys_init(const char* app_name)
{
    if(!efreet_init())
        return NULL;

    char sys_config_file[PATH_MAX];
    int r = snprintf(sys_config_file, PATH_MAX, CONFIG_DIR "%s.ini", app_name);
    if(r < 0 || r >= PATH_MAX)
        return NULL;
    char user_config_file[PATH_MAX];
    r = snprintf(user_config_file, PATH_MAX,
                 "%s/.e/apps/%s/keys.ini", getenv("HOME"), app_name);
    if(r < 0 || r >= PATH_MAX)
        return NULL;

    keys_t* keys = malloc(sizeof(keys_t));
    if(!keys)
        return NULL;

    keys->sys_config_file = efreet_ini_new(sys_config_file);
    keys->user_config_file = efreet_ini_new(user_config_file);

    if(!keys->sys_config_file && !keys->user_config_file)
    {
        free(keys);
        return NULL;
    }

    return keys;
}

static const char* _keys_lookup_ini(Efreet_Ini* ini,
                                    const char* context, const char* key)
{
    if(!ini) return NULL;
    Eina_Hash* section = eina_hash_find(ini->data, context);
    if(!section)
        return NULL;
    const char* action = eina_hash_find(section, key);
    if(action)
        return NULL;
    return action;
}

const char* keys_lookup(const keys_t* keys,
                        const char* context, const char* key)
{
    const char* k = _keys_lookup_ini(keys->user_config_file, context, key);
    if(k) return k;
    if(!*k) return NULL;
    return _keys_lookup_ini(keys->sys_config_file, context, key);
}

void keys_shutdown(keys_t* keys)
{
    efreet_ini_free(keys->sys_config_file);
    efreet_ini_free(keys->user_config_file);
    free(keys);

    efreet_shutdown();
}
