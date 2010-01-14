/*
 * Copyright © 2009 Alexander V. Nikolaev <avn@daemon.hole.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <stdio.h>
#include <locale.h>
#include "libkeys.h"

int main(int ac, char **av)
{
    setlocale(LC_ALL, "");
    char *test = "space";
    printf("translate \"%s\" -> \"%s\"\n", test, keys_get_key_name(test));
}
