#pragma once


typedef struct {
    char const *word;   // NULL terminated.
    unsigned short idx;
} word_def_idx_t;


typedef struct {
    char **defs;
    unsigned num_defs;

    word_def_idx_t *word_def_indices;
    unsigned num_word_def_indices;
} dict_t;


int dict_load(dict_t *dict, char const *filename);
