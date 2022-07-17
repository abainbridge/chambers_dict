// Deadfrog lib headers
#include "fonts/df_prop.h"
#include "df_font.h"
#include "df_window.h"

// Project headers
#include "dict.h"
#include "gui.h"

// Standard headers
#include <stdlib.h>

static const char APPLICATION_NAME[] = "21st Century Dictionary";


void populate_list_view(list_view_t *lv, dict_t *dict, char const *filter) {
    lv->num_items = 0;

    for (unsigned i = 0; i < dict->num_word_def_indices; i++) {
        char const *word = dict->word_def_indices[i].word;
        if (strstr(word, filter)) {
            lv->items[lv->num_items] = word;
            lv->num_items++;
        }
    }
}


void main() {
    DfWindow *win = CreateWin(800, 600, WT_WINDOWED_RESIZEABLE, APPLICATION_NAME);
    g_defaultFont = LoadFontFromMemory(df_prop_7x13, sizeof(df_prop_7x13));

    dict_t dict;
    if (!dict_load(&dict, "data/dict.txt")) 
        ReleaseAssert(0, "Couldn't open dict.txt");

    //
    // Init GUI widgets

    edit_box_t edit_box = { 0 };

    list_view_t list_view = { 0 };
    list_view.items = (const char **)malloc(sizeof(const char **) * dict.num_word_def_indices);
    populate_list_view(&list_view, &dict, edit_box.text);

    text_view_t text_view;
    text_view.text = "";


    //
    // Main loop

    while (!win->windowClosed && !win->input.keyDowns[KEY_ESC]) {
        InputPoll(win);

        int spacer = 10 * g_drawScale;
        int edit_box_y = spacer;
        int edit_box_height = g_defaultFont->charHeight + 8 * g_drawScale;
        int text_view_x = win->bmp->width * 0.3;
        int views_y = edit_box_y + edit_box_height + spacer;
        int text_view_width = win->bmp->width - text_view_x - spacer;
        int views_height = win->bmp->height - views_y - spacer;
        int list_view_width = text_view_x - spacer * 2;

        BitmapClear(win->bmp, g_backgroundColour);

        if (edit_box_do(win, &edit_box, spacer, edit_box_y, list_view_width, edit_box_height))
            populate_list_view(&list_view, &dict, edit_box.text);

        list_view_do(win, &list_view, spacer, views_y, list_view_width, views_height);
        int word_def_idx = dict_get_word_def_idx(&dict, list_view.items[list_view.selected_item]);
        if (word_def_idx >= 0)
            text_view.text = dict_get_clean_def_text(&dict, word_def_idx);

        text_view_do(win, &text_view, text_view_x, views_y, text_view_width, views_height);

        UpdateWin(win);
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
