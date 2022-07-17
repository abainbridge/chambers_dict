#pragma once

// Deadfrog lib headers.
#include "df_bitmap.h"
#include "df_window.h"

// Standard headers.
#include <string.h>


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


// ****************************************************************************
// List View
// ****************************************************************************

typedef struct {
    char const **items;
    unsigned num_items;
    unsigned selected_item;
} list_view_t;


// Returns id of item that was selected, or -1 if none were.
int list_view_do(DfWindow *win, list_view_t *lv, int x, int y, int w, int h) {
    const int num_rows = h / g_defaultFont->charHeight;

    if (win->input.keyDowns[KEY_DOWN])
        lv->selected_item++;
    else if (win->input.keyDowns[KEY_UP])
        lv->selected_item--;
    else if (win->input.keyDowns[KEY_PGDN])
        lv->selected_item += num_rows;
    else if (win->input.keyDowns[KEY_PGUP])
        lv->selected_item -= num_rows;

    if (lv->selected_item >= lv->num_items)
        lv->selected_item = lv->num_items - 1;
    else if (lv->selected_item < 0)
        lv->selected_item = 0;

    int first_display_item = lv->selected_item - num_rows / 2;
    first_display_item = ClampInt(first_display_item, 0, lv->num_items - num_rows / 2);

    draw_raised_box(win->bmp, x, y, w, h);
    x += 2 * g_drawScale;
    y += 2 * g_drawScale;
    w -= 4 * g_drawScale;
    h -= 4 * g_drawScale;
    SetClipRect(win->bmp, x, y, w, h);

    int last_y = y + h;
    for (unsigned i = first_display_item; i < lv->num_items; i++) {
        if (y > last_y) break;

        if (i == lv->selected_item) {
            RectFill(win->bmp, x, y, w, g_defaultFont->charHeight, g_selectionColour);
        }

        DrawTextSimple(g_defaultFont, g_normalTextColour, win->bmp, 
                       x + 2 * g_drawScale, y, lv->items[i]);
        y += g_defaultFont->charHeight;
    }

    ClearClipRect(win->bmp);

    return -1;
}


// ****************************************************************************
// Text View
// ****************************************************************************

enum { TEXT_VIEW_MAX_LINES = 200 };

typedef struct {
    char const *text;
} text_view_t;


static int render_word(char const *word, unsigned *chars_consumed) {
}


char const *find_space(char const *c) {
    while (*c != ' ' && *c != '\n' && *c != '\0') 
        c++;
    return c;
}


void text_view_do(DfWindow *win, text_view_t *tv, int x, int y, int w, int h) {
    draw_raised_box(win->bmp, x, y, w, h);
    x += 4 * g_drawScale;
    y += 2 * g_drawScale;
    w -= 8 * g_drawScale;
    h -= 4 * g_drawScale;
    SetClipRect(win->bmp, x, y, w, h);

    int space_pixels = GetTextWidth(g_defaultFont, " ");
    char const *c = tv->text;
    int current_x = x;
    while (1) {
        char const *space = find_space(c);
        unsigned word_len = space - c;

        int num_pixels = GetTextWidth(g_defaultFont, c, word_len);
        if (current_x + num_pixels >= win->bmp->clipRight) {
            current_x = x;
            y += g_defaultFont->charHeight;
        }

        DrawTextSimpleLen(g_defaultFont, g_normalTextColour, win->bmp, 
            current_x, y, c, word_len);

        current_x += num_pixels + space_pixels;
        c += word_len;

        if (*space == '\n') {
            current_x = x;
            y += g_defaultFont->charHeight;
        }

        if (*c == '\0') break;
        c++;
    }

    ClearClipRect(win->bmp);
}