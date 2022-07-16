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


void main() {
    DfWindow *win = CreateWin(800, 600, WT_WINDOWED_RESIZEABLE, APPLICATION_NAME);
    g_defaultFont = LoadFontFromMemory(df_prop_7x13, sizeof(df_prop_7x13));

    dict_t dict;
    if (!dict_load(&dict, "data/dict.txt")) 
        ReleaseAssert(0, "Couldn't open dict.txt");

    // GUI widgets
    list_view_t list_view = { 0 };
    list_view.items = (const char **)malloc(sizeof(const char **) * dict.num_word_def_indices);
    for (unsigned i = 0; i < dict.num_word_def_indices; i++) {
        list_view.items[i] = dict.word_def_indices[i].word;
    }
    list_view.num_items = dict.num_word_def_indices;

    while (!win->windowClosed && !win->input.keyDowns[KEY_ESC]) {
        InputPoll(win);

        BitmapClear(win->bmp, g_backgroundColour);
        DrawTextSimple(g_defaultFont, g_normalTextColour, win->bmp, 10, 10, "Hello");

        list_view_do(win, &list_view, 10, 30, 270, 560);

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
