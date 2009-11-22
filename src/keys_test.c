#include <stdio.h>
#include <locale.h>
#include "libkeys.h"

int main(int ac, char **av)
{
    setlocale(LC_ALL, "");
    char *test = "space";
    printf("translate \"%s\" -> \"%s\"\n", test, keys_get_key_name(test));
}
