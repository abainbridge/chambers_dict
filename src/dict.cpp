#include "dict.h"

#include "df_common.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static char *g_dictFile;
static unsigned g_dictFileNumBytes;


static int is_hex_digit(char c) {
    return strchr("0123456789abcdef", c) != NULL;
}

static char *seek_to_newline(char *s) {
    while (*s != '\n') {
        s++;
    }

    return s;
}


int dict_load(dict_t *dict, char const *filename) {
    // Read the entire file into memory.
    FILE *in = fopen(filename, "r");
    if (!in) return 0;
    fseek(in, 0, SEEK_END); 
    g_dictFileNumBytes = ftell(in);
    fseek(in, 0, SEEK_SET);
    g_dictFile = (char *)malloc(g_dictFileNumBytes);
    unsigned bytesRead = fread(g_dictFile, 1, g_dictFileNumBytes, in);
    if (bytesRead != g_dictFileNumBytes)
        return 0;
    fclose(in);

    // Count definitions.
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

    // Count words.
    dict->num_def_indices = 0;
    for (unsigned i = c - g_dictFile; i < g_dictFileNumBytes; i++) {
        if (g_dictFile[i] == '\n') {
            dict->num_def_indices++;
        }
    }

    dict->def_indices = (def_idx_t *)malloc(sizeof(def_idx_t) * 
                                                      dict->num_def_indices);

    // Read definition indices.
    for (unsigned i = 0; i < dict->num_def_indices; i++) {
        char *end_of_line = seek_to_newline(c);
        char *index_str = end_of_line;
        while (is_hex_digit(index_str[-1])) {
            index_str--;
        }

        dict->def_indices[i].idx = strtoul(index_str, NULL, 16);

        index_str--;
        ReleaseAssert(*index_str == ' ', "Missing space in dict");
        *index_str = '\0';
        dict->def_indices[i].word = c;

        c = end_of_line + 1;
    }

    return 1;
}


int dict_get_def_idx(dict_t *dict, char const *word) {
    for (unsigned i = 0; i < dict->num_def_indices; i++) {
        if (strcmp(dict->def_indices[i].word, word) == 0) {
            return dict->def_indices[i].idx;
        }
    }

    return -1;
}


static int starts_with(char const *haystack, char const *needle) {
    while (*haystack != '\0') {
        if (*needle == '\0')
            return 1;
        if (*haystack != *needle)
            return 0;
        haystack++;
        needle++;
    }

    return 0;
}


char const *dict_get_clean_def_text(dict_t *dict, unsigned idx) {
    static unsigned buf_len = 32768;
    static char *buf = (char *)malloc(buf_len);
    
    char *c = dict->defs[idx];
    if (strlen(c) > buf_len) {
        free(buf);
        buf_len *= 1.5;
        buf = (char *)malloc(buf_len);
    }

    unsigned out_pos = 0;
    while (*c != '\0') {
        if (starts_with(c, "</LV0>") ||
            starts_with(c, "</LV2>") ||
            starts_with(c, "</LV3>") ||
            starts_with(c, "</LV4>") ||
            starts_with(c, "</LV5>")) {
            buf[out_pos] = '\n';
            out_pos++;
            c += 6;
        }
        if (starts_with(c, "<LV2>")) {
            buf[out_pos] = '\n';
            out_pos++;
            c += 5;
        }
        else if (*c == '<') {
            while (*c != '>') c++;
            c++;
        }
        else {
            buf[out_pos] = *c;
            out_pos++;
            c++;
        }
    }

    buf[out_pos] = '\0';
    return buf;
}
