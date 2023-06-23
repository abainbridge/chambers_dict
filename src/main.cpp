// Deadfrog lib headers
#include "df_font.h"
#include "df_time.h"
#include "df_window.h"

// Project headers
#include "dict.h"
#include "gui.h"

// Standard headers
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


static const char APPLICATION_NAME[] = "21st Century Dictionary";

static DfWindow *g_win;
static dict_t g_dict;
static edit_box_t g_edit_box = { 0 };
static list_view_t g_list_view = { 0 };
static text_view_t g_text_view = { 0 };

// Returns 1 on match, 0 otherwise.
static int wildcard_match(char const *haystack, char const *needle) {
    int do_substring_match = !strchr(needle, '?');

    while (*haystack) {
        if (*needle == '\0')
            return do_substring_match;

        if (*needle == '*') {
            unsigned remaining_haystack_len = strlen(haystack);
            for (unsigned i = 0; i < remaining_haystack_len; i++) {
                if (wildcard_match(haystack + i, needle + 1))
                    return 1;
            }
        }
        else if (tolower(*haystack) != tolower(*needle) && *needle != '?')
            return 0;

        haystack++;
        needle++;
    }

    if (*haystack == *needle || strcmp(needle, "*") == 0)
        return 1;
    return 0;
}


static void populate_list_view(list_view_t *lv, dict_t *dict, char const *filter) {
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

static void draw_frame() {
    gui_do_frame(g_win);

    int spacer = 7 * g_drawScale;
    int edit_box_y = spacer;
    int edit_box_height = g_defaultFont->charHeight + 8 * g_drawScale;
    int text_view_x = g_win->bmp->width * 0.3;
    int views_y = edit_box_y + edit_box_height + spacer;
    int text_view_width = g_win->bmp->width - text_view_x - spacer;
    int views_height = g_win->bmp->height - views_y - spacer;
    int list_view_width = text_view_x - spacer * 2;
    int v_scrollbar_width = spacer * 2;
    int text_view_scrollbar_x = text_view_x + text_view_width - v_scrollbar_width;

    RectFill(g_win->bmp, 0, 0, g_win->bmp->width, views_y, g_frameColour);
    RectFill(g_win->bmp, 0, g_win->bmp->height - spacer, g_win->bmp->width, spacer, g_frameColour);
    RectFill(g_win->bmp, 0, 0, spacer, g_win->bmp->height, g_frameColour);
    RectFill(g_win->bmp, g_win->bmp->width - spacer, 0, spacer, g_win->bmp->height, g_frameColour);
    RectFill(g_win->bmp, text_view_x - spacer, 0, spacer, g_win->bmp->height, g_frameColour);

    if (edit_box_do(g_win, &g_edit_box, spacer, edit_box_y, list_view_width, edit_box_height))
        populate_list_view(&g_list_view, &g_dict, g_edit_box.text);

    text_view_empty(&g_text_view);
    list_view_do(g_win, &g_list_view, spacer, views_y, list_view_width, views_height);
    if (g_list_view.selected_item >= 0) {
        int def_indices[6];
        int num_defs = dict_get_def_indices(&g_dict, g_list_view.items[g_list_view.selected_item], def_indices);
        for (int i = 0; i < num_defs; i++) {
            text_view_add_text(&g_text_view, dict_get_clean_def_text(&g_dict, def_indices[i]));
            text_view_add_text(&g_text_view, "\n\n");
        }
    }

    text_view_do(g_win, &g_text_view, text_view_x, views_y, text_view_width, views_height);
    
    UpdateWin(g_win);
}

void main() {
    g_win = CreateWin(800, 600, WT_WINDOWED_RESIZEABLE, APPLICATION_NAME);
    RegisterRedrawCallback(g_win, draw_frame);
    SetWindowIcon(g_win);

    if (!dict_load(&g_dict, "data/dict.txt")) 
        ReleaseAssert(0, "Couldn't open dict.txt");

    //
    // Init GUI widgets

    g_list_view.items = (const char **)malloc(sizeof(const char **) * g_dict.num_words);
    populate_list_view(&g_list_view, &g_dict, g_edit_box.text);


    //
    // Main loop

    double next_force_frame_time = GetRealTime() + 0.2;
    while (!g_win->windowClosed && !g_win->input.keyDowns[KEY_ESC]) {
        bool force_frame = GetRealTime() > next_force_frame_time;
        if (force_frame) {
            next_force_frame_time = GetRealTime() + 0.2;
        }

        if (InputPoll(g_win) || force_frame) {
            draw_frame();
        }
         
        WaitVsync();
    }
}


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
int WINAPI WinMain(HINSTANCE _hInstance, HINSTANCE /*_hPrevInstance*/,
    LPSTR cmdLine, int /*_iCmdShow*/) {
    main();
    return 0;
}
