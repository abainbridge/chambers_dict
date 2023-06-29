#pragma once

// Deadfrog lib headers.
#include "df_bitmap.h"
#include "df_window.h"

extern DfColour g_backgroundColour;
extern DfColour g_frameColour;
extern DfColour g_buttonShadowColour;
extern DfColour g_buttonHighlightColour;
extern DfColour g_normalTextColour;
extern DfColour g_selectionColour;

extern double g_drawScale;
extern int g_dragStartX;
extern int g_dragStartY;


// ****************************************************************************
// Misc functions
// ****************************************************************************

int is_mouse_in_bounds(DfWindow *win, int x, int y, int w, int h);
void gui_do_frame(DfWindow *win);

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
void draw_sunken_box(DfBitmap *bmp, int x, int y, int w, int h);


// ****************************************************************************
// V Scrollbar
// ****************************************************************************

typedef struct {
    int maximum;
    int current_val;
    int covered_range;
    int speed;

    int dragging;
    int current_val_at_drag_start;
} v_scrollbar_t;

int v_scrollbar_do(DfWindow *win, v_scrollbar_t *vs, int x, int y, int w, int h, int has_focus);


// ****************************************************************************
// Edit box
// ****************************************************************************

typedef struct {
    char text[128];
    double nextCursorToggleTime;
    char cursorOn;
    int cursorIdx;
} edit_box_t;


// Returns 1 if contents changed.
int edit_box_do(DfWindow *win, edit_box_t *eb, int x, int y, int w, int h);


// ****************************************************************************
// List View
// ****************************************************************************

typedef struct {
    char const **items;
    int num_items;
    int selected_item;
    int first_display_item;
} list_view_t;


// Returns id of item that was selected, or -1 if none were.
int list_view_do(DfWindow *win, list_view_t *lv, int x, int y, int w, int h);


// ****************************************************************************
// Text View
// ****************************************************************************

enum { TEXT_VIEW_MAX_CHARS = 95000 };

typedef struct {
    v_scrollbar_t v_scrollbar;
    char text[TEXT_VIEW_MAX_CHARS];
    char wrapped_text[TEXT_VIEW_MAX_CHARS];
    
    // If start and end coords are the same, then there is no selection block.
    int selection_start_x;
    int selection_start_y;
    int selection_end_x;
    int selection_end_y;
} text_view_t;


void text_view_empty(text_view_t *tv);
void text_view_add_text(text_view_t *tv, char const *text);
void text_view_do(DfWindow *win, text_view_t *tv, int x, int y, int w, int h);
char const *text_view_get_selected_text(text_view_t *tv, int *num_chars);


// ****************************************************************************
// Button
// ****************************************************************************

typedef struct {
    char const *label;
} button_t;


int button_do(DfWindow *win, button_t *b, int x, int y, int w, int h);
