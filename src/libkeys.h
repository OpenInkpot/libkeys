#ifndef LIBKEYS_H
#define LIBKEYS_H

/*
 * key * context -> action mapping library
 *
 * Key mappings are read from the following files:
 *   /etc/keys/$app.ini
 *   $home/.e/apps/$app/keys.ini
 *
 * keys.ini in home directory _amends_ system config, not _replaces_ it.
 */

#include <stdbool.h>

struct keys_t;
typedef struct keys_t keys_t;

keys_t* keys_init(const char* app_name);

/*
 * Returns action for specified pair (context, key)
 * or NULL if no action can be found.
 */
const char* keys_lookup(const keys_t* keys,
                        const char* context, const char* key);

void keys_shutdown(keys_t* keys);

#endif
