#ifndef LIBKEYS_H
#define LIBKEYS_H

/*
 * key * context -> action mapping library
 *
 * Key mappings are read from the following files:
 *   /usr/share/keys/$app.ini
 *   /etc/keys/$app.ini
 *   $home/.keys/$app.ini
 *
 */

#include <stdbool.h>
#include <Eina.h>
#include <Evas.h>

struct keys_t;
typedef struct keys_t keys_t;

keys_t* keys_alloc(const char* app_name);

/*
 * Returns action for specified pair (context, key)
 * or NULL if no action can be found.
 */
const char* keys_lookup(const keys_t* keys,
                        const char* context, const char* key);

const char* keys_lookup_by_event(const keys_t* keys,
                                 const char* context,
                                 const Evas_Event_Key_Up* event);


/*
 * Return list of keys mapped to action
 *
 */

Eina_List* keys_reverse_lookup(const keys_t* keys,
                               const char* context,
                               const char* action);

void keys_free(keys_t* keys);


/*
 * Return localized key naming as referred in printed device manual
 */
const char* keys_get_key_name(const char* keysym);

#endif
