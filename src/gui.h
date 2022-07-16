#pragma once


#include "df_bitmap.h"
#include "df_window.h"


DfColour g_backgroundColour = { 0xff323232 };
DfColour g_buttonShadowColour = { 0xff919191 };
DfColour g_buttonHighlightColour = { 0xffd9d9d9 };
DfColour g_normalTextColour = Colour(200, 200, 200, 255);
DfColour g_selectionColour = Colour(21, 79, 255);

double g_drawScale = 1.0;



void draw_raised_box(DfBitmap *bmp, int x, int y, int w, int h) {
    // The specified w and h is the external size of the box.
    //
    //        <-------- w -------->
    //     ^  1 1 1 1 1 1 1 1 1 1 1  ^
    //     |  1 1 1 1 1 1 1 1 1 1 1  | thickness
    //     |  1 1 1 1 1 1 1 1 1 1 1  v
    //     |  3 3 3           4 4 4
    //   h |  3 3 3           4 4 4
    //     |  3 3 3           4 4 4
    //     |  2 2 2 2 2 2 2 2 2 2 2
    //     |  2 2 2 2 2 2 2 2 2 2 2
    //     v  2 2 2 2 2 2 2 2 2 2 2
    //       <--->
    //     thickness

    int thickness = RoundToInt(g_drawScale);
    DfColour light = g_buttonHighlightColour;
    DfColour dark = g_buttonShadowColour;
    RectFill(bmp, x, y, w, thickness, light); // '1' pixels
    RectFill(bmp, x, y + h - thickness, w, thickness, dark); // '2' pixels
    RectFill(bmp, x, y + thickness, thickness, h - 2 * thickness, light);
    RectFill(bmp, x + w - thickness, y + thickness, thickness, h - 2 * thickness, dark);
}



typedef struct {
    char const **items;
    unsigned num_items;
    unsigned selected_item;
} list_view_t;


// Returns id of item that was selected, or -1 if none were.
int list_view_do(DfWindow *win, list_view_t *lv, int x, int y, int w, int h) {
    if (win->input.keyDowns[KEY_DOWN] && lv->selected_item < lv->num_items)
        lv->selected_item++;
    else if (win->input.keyDowns[KEY_UP] && lv->selected_item > 0)
        lv->selected_item--;

    draw_raised_box(win->bmp, x, y, w, h);
    x += 2; y += 2; w -= 4; h -= 4;
    SetClipRect(win->bmp, x, y, w, h);

    int last_y = y + h;
    for (unsigned i = 0; i < lv->num_items; i++) {
        if (y > last_y) break;

        if (i == lv->selected_item) {
            RectFill(win->bmp, x, y, w, g_defaultFont->charHeight, g_selectionColour);
        }

        DrawTextSimple(g_defaultFont, g_normalTextColour, win->bmp, x + 2, y, lv->items[i]);
        y += g_defaultFont->charHeight;
    }

    ClearClipRect(win->bmp);

    return -1;
}
