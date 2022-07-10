#include "dict.h"

#include "df_common.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>


static char *g_dictFile;
static unsigned g_dictFileNumBytes;


static char *seek_to_newline(char *s) {
    while (*s != '\n') {
        s++;
    }

    return s;
}


int dict_load(dict_t *dict, char const *filename) {
    FILE *in = fopen(filename, "r");
    if (!in) return 0;

    fseek(in, 0, SEEK_END); 
    g_dictFileNumBytes = ftell(in);

    fseek(in, 0, SEEK_SET);

    g_dictFile = (char *)malloc(g_dictFileNumBytes);
    unsigned bytesRead = fread(g_dictFile, 1, g_dictFileNumBytes, in);
    if (bytesRead != g_dictFileNumBytes)
        return 0;

    // Count num definitions.
    dict->num_defs = 0;
    for (unsigned i = 0; i < g_dictFileNumBytes - 1; i++) {
        if (g_dictFile[i] == '\n') {
            dict->num_defs++;
            if (g_dictFile[i + 1] != '<') break;
        }
    }

    // Create array of pointers to definitions.
    dict->defs = (char **)malloc(sizeof(char *) * dict->num_defs);
    char *c = g_dictFile;
    for (unsigned i = 0; i < dict->num_defs; i++) {
        dict->defs[i] = c;
        c = seek_to_newline(c);
        *c = '\0';
        c++;
    }

    // Count num words.
    dict->num_word_def_indices = 0;
    for (unsigned i = c - g_dictFile; i < g_dictFileNumBytes; i++) {
        if (g_dictFile[i] == '\n') {
            dict->num_word_def_indices++;
        }
    }

    dict->word_def_indices = (word_def_idx_t *)malloc(sizeof(word_def_idx_t) * 
                                                      dict->num_word_def_indices);

    // Read indices of definitions for each word.
    for (unsigned i = 0; i < dict->num_word_def_indices; i++) {
        char *end_of_line = seek_to_newline(c);
        char *index_str = end_of_line;
        while (isdigit(index_str[-1])) {
            index_str--;
        }

        dict->word_def_indices[i].idx = strtoul(index_str, NULL, 10);

        index_str--;
        ReleaseAssert(*index_str == ' ', "Missing space in dict");
        *index_str = '\0';
        dict->word_def_indices[i].word = c;

        c = end_of_line + 1;
    }

    return 1;
}
