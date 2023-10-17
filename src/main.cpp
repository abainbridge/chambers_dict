// Deadfrog lib headers
#include "df_clipboard.h"
#include "df_font.h"
#include "df_gui.h"
#include "df_time.h"
#include "df_window.h"

// Project headers
#include "dict.h"

// Standard headers
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


static const char APPLICATION_NAME[] = "21st Century Dictionary";

static dict_t g_dict;
static DfEditBox g_edit_box = { 0 };
static DfButton g_help_button = { "Help" };
static DfListView g_list_view = { 0 };
static char const *g_current_word = NULL;
static DfTextView g_text_view = { 0 };

// Returns 1 on match, 0 otherwise.
static int wildcard_match(char const *haystack, char const *needle) {
    int do_substring_match = true;
    if (strchr(needle, '?') || strchr(needle, '*'))
        do_substring_match = false;

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


static void populate_list_view(DfListView *lv, dict_t *dict, char const *filter) {
    lv->numItems = 0;

    unsigned filter_len = strlen(filter);
    for (unsigned i = 0; i < dict->num_words; i++) {
        char const *word = dict->words[i];
        if (wildcard_match(word, filter)) {
            lv->items[lv->numItems] = word;
            lv->numItems++;
        }
    }
}

static void draw_frame() {
    DfWindow *win = g_window;
    DfGuiDoFrame(win);

    int spacer = 7 * g_drawScale;
    int edit_box_y = spacer;
    int edit_box_height = g_defaultFont->charHeight + 8 * g_drawScale;
    int help_button_w = 60 * g_drawScale;
    int help_button_h = edit_box_height;
    int help_button_x = win->bmp->width - help_button_w - spacer;
    int help_button_y = edit_box_y;
    int text_view_x = win->bmp->width * 0.3;
    int views_y = edit_box_y + edit_box_height + spacer;
    int text_view_width = win->bmp->width - text_view_x - spacer;
    int views_height = win->bmp->height - views_y - spacer;
    int list_view_width = text_view_x - spacer * 2;
    int v_scrollbar_width = spacer * 2;
    int text_view_scrollbar_x = text_view_x + text_view_width - v_scrollbar_width;

    RectFill(win->bmp, 0, 0, win->bmp->width, views_y, g_frameColour);
    RectFill(win->bmp, 0, win->bmp->height - spacer, win->bmp->width, spacer, g_frameColour);
    RectFill(win->bmp, 0, 0, spacer, win->bmp->height, g_frameColour);
    RectFill(win->bmp, win->bmp->width - spacer, 0, spacer, win->bmp->height, g_frameColour);
    RectFill(win->bmp, text_view_x - spacer, 0, spacer, win->bmp->height, g_frameColour);

    if (DfEditBoxDo(win, &g_edit_box, spacer, edit_box_y, list_view_width, edit_box_height))
        populate_list_view(&g_list_view, &g_dict, g_edit_box.text);

    if (DfButtonDo(win, &g_help_button, help_button_x, help_button_y,
        help_button_w, help_button_h)) {
        strcpy(g_edit_box.text, "help text");
        populate_list_view(&g_list_view, &g_dict, g_edit_box.text);
    }

    DfListViewDo(win, &g_list_view, spacer, views_y, list_view_width, views_height);
    char const *selected_word = "";
    if (g_list_view.selectedItem >= 0)
        selected_word = g_list_view.items[g_list_view.selectedItem];
    if (selected_word != g_current_word) {
        DfTextViewEmpty(&g_text_view);

        int def_indices[6];
        int num_defs = dict_get_def_indices(&g_dict, selected_word, def_indices);
        for (int i = 0; i < num_defs; i++) {
            DfTextViewAddText(&g_text_view, dict_get_clean_def_text(&g_dict, def_indices[i]));
            DfTextViewAddText(&g_text_view, "\n\n");
        }

        g_current_word = selected_word;
    }

    DfTextViewDo(win, &g_text_view, text_view_x, views_y, text_view_width, views_height);

    if (win->input.keyDowns[KEY_C] && win->input.keys[KEY_CONTROL]) {
        int num_chars;
        char const *selected_text = DfTextViewGetSelectedText(&g_text_view, &num_chars);
        if (selected_text) {
            ClipboardSetData(selected_text, num_chars);
        }
    }
    
    UpdateWin(win);
}

void main() {
    g_window = CreateWin(800, 600, WT_WINDOWED_RESIZEABLE, APPLICATION_NAME);
    RegisterRedrawCallback(g_window, draw_frame);
    SetWindowIcon(g_window);
    BitmapClear(g_window->bmp, g_frameColour);
    UpdateWin(g_window);

    if (!dict_load(&g_dict, "data/dict.txt")) 
        ReleaseAssert(0, "Couldn't open dict.txt");

    //
    // Init GUI widgets

    g_list_view.items = (const char **)malloc(sizeof(const char **) * g_dict.num_words);
    populate_list_view(&g_list_view, &g_dict, g_edit_box.text);


    //
    // Main loop

    double next_force_frame_time = GetRealTime() + 0.2;
    while (!g_window->windowClosed && !g_window->input.keyDowns[KEY_ESC]) {
        bool force_frame = GetRealTime() > next_force_frame_time;
        if (force_frame) {
            next_force_frame_time = GetRealTime() + 0.2;
        }

        if (InputPoll(g_window) || force_frame) {
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
