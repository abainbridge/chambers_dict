// Deadfrog lib headers
#include "df_font.h"
#include "df_window.h"

// Project headers
#include "dict.h"
#include "gui.h"

// Standard headers
#include <ctype.h>
#include <stdlib.h>

static const char APPLICATION_NAME[] = "21st Century Dictionary";


// Returns 1 on match, 0 otherwise.
int wildcard_match(char const *haystack, char const *needle) {
    bool needleEndsWithQmark = needle[strlen(needle) - 1] == '?';
    while (*haystack) {
        if (*needle == '\0')
            return !needleEndsWithQmark;

        if (*needle == '*') {
            unsigned remaining_needle_len = strlen(needle);
            for (unsigned i = 0; i < remaining_needle_len; i++) {
                if (wildcard_match(haystack + i, needle + 1))
                    return 1;
            }
        }
        else if (tolower(*haystack) != tolower(*needle) && *needle != '?')
            return 0;

        haystack++;
        needle++;
    }

    return *haystack == *needle;
}


void populate_list_view(list_view_t *lv, dict_t *dict, char const *filter) {
    lv->num_items = 0;

    unsigned filter_len = strlen(filter);
    for (unsigned i = 0; i < dict->num_words; i++) {
        char const *word = dict->words[i];
        if (wildcard_match(word, filter)) {
            lv->items[lv->num_items] = word;
            lv->num_items++;
        }
    }
}


void main() {
    DfWindow *win = CreateWin(800, 600, WT_WINDOWED_RESIZEABLE, APPLICATION_NAME);

    dict_t dict;
    if (!dict_load(&dict, "data/dict.txt")) 
        ReleaseAssert(0, "Couldn't open dict.txt");

    //
    // Init GUI widgets

    edit_box_t edit_box = { 0 };

    list_view_t list_view = { 0 };
    list_view.items = (const char **)malloc(sizeof(const char **) * dict.num_words);
    populate_list_view(&list_view, &dict, edit_box.text);

    text_view_t text_view;
    text_view_init(&text_view);


    //
    // Main loop

    double next_force_frame_time = GetRealTime() + 0.2;
    while (!win->windowClosed && !win->input.keyDowns[KEY_ESC]) {
        bool force_frame = GetRealTime() > next_force_frame_time;
        if (force_frame) {
            next_force_frame_time = GetRealTime() + 0.2;
        }

        if (InputPoll(win) || force_frame) {
            HandleDrawScaleChange(win);

            int spacer = 7 * g_drawScale;
            int edit_box_y = spacer;
            int edit_box_height = g_defaultFont->charHeight + 8 * g_drawScale;
            int text_view_x = win->bmp->width * 0.3;
            int views_y = edit_box_y + edit_box_height + spacer;
            int text_view_width = win->bmp->width - text_view_x - spacer;
            int views_height = win->bmp->height - views_y - spacer;
            int list_view_width = text_view_x - spacer * 2;

            RectFill(win->bmp, 0, 0, win->bmp->width, views_y, g_frameColour);
            RectFill(win->bmp, 0, win->bmp->height - spacer, win->bmp->width, spacer, g_frameColour);
            RectFill(win->bmp, 0, 0, spacer, win->bmp->height, g_frameColour);
            RectFill(win->bmp, win->bmp->width - spacer, 0, spacer, win->bmp->height, g_frameColour);
            RectFill(win->bmp, text_view_x - spacer, 0, spacer, win->bmp->height, g_frameColour);

            if (edit_box_do(win, &edit_box, spacer, edit_box_y, list_view_width, edit_box_height))
                populate_list_view(&list_view, &dict, edit_box.text);

            text_view_empty(&text_view);
            list_view_do(win, &list_view, spacer, views_y, list_view_width, views_height);
            if (list_view.selected_item >= 0) {
                int def_indices[6];
                int num_defs = dict_get_def_indices(&dict, list_view.items[list_view.selected_item], def_indices);
                for (int i = 0; i < num_defs; i++) {
                    text_view_add_text(&text_view, dict_get_clean_def_text(&dict, def_indices[0]));
                }
            }

            text_view_do(win, &text_view, text_view_x, views_y, text_view_width, views_height);

            UpdateWin(win);
        }
         
        WaitVsync();
    }
}


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
int WINAPI WinMain(HINSTANCE _hInstance, HINSTANCE /*_hPrevInstance*/,
    LPSTR cmdLine, int /*_iCmdShow*/)
{
    main();
    return 0;
}
