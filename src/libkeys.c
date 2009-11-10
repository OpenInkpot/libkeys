#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/param.h>
#include <unistd.h>

#include <Eina.h>
#include <Efreet.h>

#include "libkeys.h"

/* Debugging */

#define LIBKEYS_DEBUG_LOOKUP 1 /* Keys lookup */
#define LIBKEYS_DEBUG_CONFIGS 2 /* Configuration files */

static int _libkeys_debug_level;

/* Config files */

#define SHARE_DIR DATADIR "/keys/"
#define CONFIG_DIR SYSCONFDIR "/keys/"


typedef struct keys_binding_t keys_binding_t;
struct keys_binding_t
{
    const char* keysym;
    const char* action;
    const char* where;
};

typedef struct keys_reverse_binding_t keys_reverse_binding_t;
struct keys_reverse_binding_t
{
    const char* action;
    Eina_List* keysyms;
};

typedef struct keys_context_t keys_context_t;
struct keys_context_t
{
    Eina_List* map;
    Eina_List* reverse_map;
    const char* name;
};

struct keys_t
{
    const char* app_name;
    Eina_List* contexts;
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


static keys_binding_t*
_find_binding(Eina_List* lst, const char* key)
{
    Eina_List* l;
    keys_binding_t* node;
    EINA_LIST_FOREACH(lst, l, node)
    {
        if(!strcmp(node->keysym, key))
            return node;
    }
    return NULL;
}

static keys_reverse_binding_t*
_find_reverse_binding(Eina_List* lst, const char* key)
{
    Eina_List* l;
    keys_reverse_binding_t* node;
    EINA_LIST_FOREACH(lst, l, node)
    {
        if(!strcmp(node->action, key))
            return node;
    }
    return NULL;
}

static keys_context_t*
_find_context(const keys_t* keys, const char* key)
{
    Eina_List* l;
    keys_context_t* node;
    if(!keys->contexts)
        return NULL;
    EINA_LIST_FOREACH(keys->contexts, l, node)
    {
        if(!strcmp(node->name, key))
            return node;
    }
    return NULL;
}

static void
keys_context_add(keys_t* keys, const char* name)
{
    if(!_find_context(keys, name))
    {
        const char* shared_name = eina_stringshare_add(name);
        keys_context_t* context = calloc(1, sizeof(keys_context_t));
        context->map = NULL;
        context->reverse_map = NULL;
        context->name = shared_name;
        keys->contexts = eina_list_append(keys->contexts, context);
    }
}

#if 0
static void
keys_context_del(keys_t* keys, const char* name)
{
    keys_context_t* context = _find_context(keys, name);
    if(context)
    {
        keys_binding_t* binding;
        EINA_LIST_FREE(context->map, binding)
        {
            eina_stringshare_del(binding->keysym);
            eina_stringshare_del(binding->action);
            if(binding->where)
                eina_stringshare_del(binding->where);
        }
        keys_reverse_binding_t* reverse_binding;
        EINA_LIST_FREE(context->reverse_map, reverse_binding)
        {
            eina_stringshare_del(reverse_binding->action);
            char* ptr;
            EINA_LIST_FREE(reverse_binding->keysyms, ptr)
            {
                eina_stringshare_del(ptr);
            }
        }
        eina_stringshare_del(context->name);
        keys->contexts = eina_list_remove(keys->contexts, context);
        free(context);
    }
}

#endif

static void
_unmap_reverse(keys_context_t* ctx, const char* key, const char* action)
{
    keys_reverse_binding_t* reverse = _find_reverse_binding(ctx->reverse_map,
        action);

    const char* data;
    Eina_List* l, *l_next;
    EINA_LIST_FOREACH_SAFE(reverse->keysyms, l, l_next, data)
        if(!strcmp(data, key))
        {
            reverse->keysyms = eina_list_remove_list(reverse->keysyms, l);
            eina_stringshare_del(data);
        }

    if(!reverse->keysyms)
    {
        eina_stringshare_del(reverse->action);
        ctx->reverse_map = eina_list_remove(ctx->reverse_map, reverse);
        free(reverse);
    }
}

static void
_map_reverse(keys_context_t* ctx, const char* key, const char* action)
{
    keys_reverse_binding_t* reverse = _find_reverse_binding(ctx->reverse_map,
        action);
    if(!reverse)
    {
        reverse = calloc(1, sizeof(keys_reverse_binding_t));
        reverse->action = eina_stringshare_add(action);
        ctx->reverse_map = eina_list_append(ctx->reverse_map, reverse);
    }
    reverse->keysyms = eina_list_append(reverse->keysyms,
        eina_stringshare_add(key));
}

static void
_keys_map_key(keys_t* keys, const char* context, const char* key,
         const char* action, const char* where)
{
    log(LIBKEYS_DEBUG_CONFIGS, "map key %s %s %s", context,
        key, action);
    keys_context_t* ctx = _find_context(keys, context);
    if(!ctx)
        return;
    keys_binding_t* binding = _find_binding(ctx->map, key);
    if(binding)
    {
        _unmap_reverse(ctx, key, binding->action);
        eina_stringshare_del(binding->action);
        if(binding->where)
            eina_stringshare_del(binding->where);
    }
    else
    {
        binding = calloc(1, sizeof(keys_binding_t));
        binding->keysym = eina_stringshare_add(key);
        ctx->map = eina_list_append(ctx->map, binding);
    }
    binding->action = eina_stringshare_add(action);
    if(where)
        binding->where = eina_stringshare_add(where);
    _map_reverse(ctx, key, action);
}

#if 0
void
keys_map_key(keys_t* keys, const char* context, const char* key,
         const char* action)
{
    _keys_map_key(keys, context, key, action, NULL);
}

static void
keys_unmap_key(keys_t* keys, const char* context, const char* key,
         const char* action)
{

    keys_context_t* ctx = _find_context(keys, context);
    if(!ctx)
        return;
    keys_binding_t* binding = _find_binding(ctx->map, key);
    if(binding)
    {
        _unmap_reverse(ctx, key, action);
        eina_stringshare_del(binding->action);
        eina_stringshare_del(binding->keysym);
        if(binding->where)
            eina_stringshare_del(binding->where);
    }
}
#endif

static void
keys_dump(keys_t* keys)
{
    log(LIBKEYS_DEBUG_CONFIGS, "; dump %s.ini\n", keys->app_name);
    Eina_List* list;
    keys_context_t* context;
    EINA_LIST_FOREACH(keys->contexts, list, context)
    {
        log(LIBKEYS_DEBUG_CONFIGS, "[%s]", context->name);
        Eina_List* items;
        keys_binding_t* binding;
        EINA_LIST_FOREACH(context->map, items, binding)
        {
            log(LIBKEYS_DEBUG_CONFIGS, "%s = %s ; defined at %s",
                binding->keysym, binding->action,
                binding->where ? binding->where : "<local>");
        }
        keys_reverse_binding_t* reverse;
        log(LIBKEYS_DEBUG_CONFIGS, "; Reverse mappings:");
        EINA_LIST_FOREACH(context->reverse_map, items, reverse)
        {
            log(LIBKEYS_DEBUG_CONFIGS, "; %s =>", reverse->action);
            char* keysym;
            Eina_List* keysym_list;
            EINA_LIST_FOREACH(reverse->keysyms, keysym_list, keysym)
                log(LIBKEYS_DEBUG_CONFIGS, "; * => %s", keysym);
        }
    }
}

static keys_t *
keys_load(keys_t* keys, const char* filename)
{
    if(!efreet_init())
        return NULL;

    log(LIBKEYS_DEBUG_CONFIGS, "%s: loading %s", keys->app_name, filename);

    Efreet_Ini* ini = efreet_ini_new(filename);
    if(!ini || !ini->data)
    {
        log(LIBKEYS_DEBUG_CONFIGS, "%s: can't load %s",
            keys->app_name, filename);
        if(ini)
            efreet_ini_free(ini);
        return NULL;
    };

    Eina_Iterator* sections = eina_hash_iterator_tuple_new(ini->data);
    Eina_Hash_Tuple* section;
    EINA_ITERATOR_FOREACH(sections, section)
    {
        if(!section)
            continue;
        keys_context_add(keys, section->key);
        Eina_Iterator* values =
            eina_hash_iterator_tuple_new((Eina_Hash*)section->data);
        Eina_Hash_Tuple* value;
        EINA_ITERATOR_FOREACH(values, value)
        {
            _keys_map_key(keys, section->key, value->key, value->data,
                filename);
        }
        eina_iterator_free(values);
    }
    eina_iterator_free(sections);

    efreet_ini_free(ini);
    efreet_shutdown();
    return keys;
}

static keys_t*
_load_one_config(keys_t* keys, const char* template, const char* app_name)
{
    char configs[PATH_MAX];

    int r = snprintf(configs, PATH_MAX, template, app_name);
    if(r < 0 || r >= PATH_MAX)
        return NULL;
    if(access(configs, R_OK))
    {
        log(LIBKEYS_DEBUG_CONFIGS, "%s: %s [not found]", app_name, configs);
        return NULL;
    }
    return keys_load(keys, configs);
}

keys_t* keys_alloc(const char* app_name)
{
    eina_init();

    if(getenv("LIBKEYS_DEBUG"))
        _libkeys_debug_level = atoi(getenv("LIBKEYS_DEBUG"));

    log(LIBKEYS_DEBUG_CONFIGS, "%s: init", app_name);

    keys_t* keys = calloc(1, sizeof(keys_t));
    if(!keys)
        return NULL;

    keys->app_name = strdup(app_name);

    char home[PATH_MAX];
    int r = snprintf(home, PATH_MAX, "%s/.keys/%%s.ini", getenv("HOME"));
    log(LIBKEYS_DEBUG_CONFIGS, "%s: HOME=%s", app_name, home);
    if(r < 0 || r >= PATH_MAX)
        return NULL;

    bool loaded = false;
    if(_load_one_config(keys, SHARE_DIR "%s.ini", app_name))
        loaded = true;
    if(_load_one_config(keys, CONFIG_DIR "%s.ini", app_name))
        loaded = true;
    if(_load_one_config(keys, home, app_name))
        loaded = true;
    if(!loaded)
        log(LIBKEYS_DEBUG_CONFIGS, "%s: no keymaps", app_name);
    keys_dump(keys);
    return keys;
}


const char* keys_lookup(const keys_t* keys,
                        const char* context, const char* key)
{
    log(LIBKEYS_DEBUG_LOOKUP, "%s: lookup %s/%s", keys->app_name, context, key);

    keys_context_t* ctx = _find_context(keys, context);
    if(!ctx)
    {
        log(LIBKEYS_DEBUG_LOOKUP, "[%s] -> <no context>", context);
        return "";
    }

    keys_binding_t* binding = _find_binding(ctx->map, key);
    if(!binding)
    {
        log(LIBKEYS_DEBUG_LOOKUP, "[%s] -> <no match>", key);
        return "";
    }
    log(LIBKEYS_DEBUG_LOOKUP, "[%s] -> %s", key, binding->action);
    return binding->action;
}

Eina_List* keys_reverse_lookup(const keys_t* keys,
                                const char* context,
                                const char* action)
{

    keys_context_t* ctx = _find_context(keys, context);
    if(!ctx)
    {
        log(LIBKEYS_DEBUG_LOOKUP, "[%s] -> <no context>", context);
        return NULL;
    }
    keys_reverse_binding_t* reverse = _find_reverse_binding(ctx->reverse_map,
        action);
    if(!reverse)
    {
        log(LIBKEYS_DEBUG_LOOKUP, "[%s] -> <no reverse map>", action);
        return NULL;
    }
    return reverse->keysyms;
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

    keys_context_t* context;
    EINA_LIST_FREE(keys->contexts,  context)
    {
        Eina_List* items;
        keys_binding_t* binding;
        EINA_LIST_FOREACH(context->map, items, binding)
        {
            _unmap_reverse(context, binding->keysym, binding->action);
            eina_stringshare_del(binding->keysym);
            eina_stringshare_del(binding->action);
            if(binding->where)
                eina_stringshare_del(binding->where);
            free(binding);
        }
        eina_stringshare_del(context->name);
        free(context);
    }

    free(keys->app_name);
    free(keys);

    eina_shutdown();
}

const char*
keys_get_key_name(const char* keysym)
{
    return dgettext("libkeys", keysym);
}
