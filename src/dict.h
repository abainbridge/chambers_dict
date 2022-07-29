#pragma once


// There are 49236 definitions and 86279 words that point to those definitions.
// This is because, for example, "clean", "cleaning" and "cleaned" all point to
// the same definition.
//
// To look up a word, first you look in the def_indices table for "cleaning"
// and find that the definition has index 0x1d4b. Then you look in the defs 
// array at index 0x1d4b.
//
// Some words have multiple definition indices. eg "running" has indices 0x459c
// and 0x90ec.


typedef struct {
    char const *word;   // NULL terminated.
    unsigned short idx;
} def_idx_t;


typedef struct {
    // A table that maps words to definition indices.
    def_idx_t *def_indices;
    unsigned num_def_indices;

    // An array of definitions.
    char **defs;    // Array of pointers to NULL terminated strings.
    unsigned num_defs;
} dict_t;


int dict_load(dict_t *dict, char const *filename);
int dict_get_def_idx(dict_t *dict, char const *word);
char const *dict_get_clean_def_text(dict_t *dict, unsigned idx);
