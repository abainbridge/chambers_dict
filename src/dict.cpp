#include "dict.h"

#include "df_common.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static char *g_dictFile;
static unsigned g_dictFileNumBytes;


int dict_load(dict_t *dict, char const *filename) {
    // Read the entire file into memory.
    FILE *in = fopen(filename, "r");
    if (!in) return 0;
    fseek(in, 0, SEEK_END); 
    g_dictFileNumBytes = ftell(in);
    fseek(in, 0, SEEK_SET);
    g_dictFile = (char *)malloc(g_dictFileNumBytes);
    unsigned bytesRead = fread(g_dictFile, 1, g_dictFileNumBytes, in);
    fclose(in);
    if (bytesRead != g_dictFileNumBytes)
        return 0;

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
        c = strchr(c, '\n');
        *c = '\0';
        c++;
    }

    // Count words.
    dict->num_words = 0;
    for (unsigned i = c - g_dictFile; i < g_dictFileNumBytes; i++) {
        if (g_dictFile[i] == '\n') {
            dict->num_words++;
        }
    }

    dict->words = (char **)malloc(sizeof(char *) * dict->num_words);

    // Read words table.
    for (unsigned i = 0; i < dict->num_words; i++) {
        dict->words[i] = c;

        // Swap the first colon for a NULL char so we can use words[x] as a
        // NULL terminated string.
        c = strchr(c, ':');
        *c = '\0';

        c = strchr(c + 1, '\n') + 1;
    }

    return 1;
}


int dict_get_def_indices(dict_t *dict, char const *word, int indices[6]) {
    for (unsigned i = 0; i < dict->num_words; i++) {
        if (strcmp(dict->words[i], word) == 0) {
            char *c = dict->words[i] + strlen(word) + 1;
            for (int j = 0; j < 6; j++) {
                indices[j] = strtoul(c, &c, 16);
                if (*c == '\n') {
                    return j + 1;
                }
                ReleaseAssert(*c == ':', "Bad word line");
                c++;
            }
        }
    }

    return 0;
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
            while (*c == ' ') c++;
            c += 6;
        }
        if (starts_with(c, "<LV2>") ||
            starts_with(c, "<LV3>") || 
            starts_with(c, "<LV6")) {
            buf[out_pos] = '\n';
            out_pos++;
            while (*c == ' ') c++;
            c += 5;
        }
        else if (starts_with(c, "<EXA>")) {
            buf[out_pos++] = '-';
            buf[out_pos++] = ' ';
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
