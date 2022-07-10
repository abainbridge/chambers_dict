// Deadfrog lib headers
#include "fonts/df_prop.h"
#include "df_font.h"
#include "df_window.h"

// Project headers
#include "dict.h"

// Standard headers
#include <stdlib.h>

static const char APPLICATION_NAME[] = "21st Century Dictionary";

#include <direct.h>

void main() {
    DfWindow *win = CreateWin(800, 600, WT_WINDOWED_RESIZEABLE, APPLICATION_NAME);
    g_defaultFont = LoadFontFromMemory(df_prop_7x13, sizeof(df_prop_7x13));

    char buf[200];
    getcwd(buf, 200);

    dict_t dict;
    if (!dict_load(&dict, "data/dict.txt")) 
        ReleaseAssert(0, "Couldn't open dict.txt");

    while (!win->windowClosed && !win->input.keyDowns[KEY_ESC]) {
        InputPoll(win);

        BitmapClear(win->bmp, g_colourWhite);
        DrawTextSimple(g_defaultFont, g_colourBlack, win->bmp, 10, 10, "Hello");

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
