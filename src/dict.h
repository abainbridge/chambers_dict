#pragma once


// There are 49236 definitions and 86279 words that point to those definitions.
// This is because, for example, "clean", "cleaning" and "cleaned" all point to
// the same definition.
//
// To look up a word, first you look in the words table for "cleaning" and find
// that the definition has index 0x1d4b. Then you look in the defs array at 
// index 0x1d4b.
//
// Some words have multiple definition indices. eg "running" has indices 0x459c
// and 0x90ec.


typedef struct {
    // A table that maps words to definition indices. This is needed
    // to allow multiple words to map to the same definition. eg 
    // "tenth" and "10th" map to the definition for "tenth". TODO: Make
	// it so that words defined in "defs" don't appear in this list -
	// only the _extra_ mappings need to be stored here.
    char **words;   // Array items are like <word>\0<index>[:<index>]...\n
                    // eg "running\0459c:90ec\n".
                    // This is because it is very easy to form from the
                    // file on disk and is space efficient in RAM.
    unsigned num_words;

    // An array of definitions.
    char **defs;    // Array of pointers to NULL terminated strings.
    unsigned num_defs;
} dict_t;


int dict_load(dict_t *dict, char const *filename);
int dict_get_def_indices(dict_t *dict, char const *word, int indices[6]);
char const *dict_get_clean_def_text(dict_t *dict, unsigned idx);
