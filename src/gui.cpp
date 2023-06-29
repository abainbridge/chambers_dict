#include "gui.h"

// Deadfrog lib headers.
#include "fonts/df_prop.h"
#include "df_bitmap.h"
#include "df_time.h"
#include "df_window.h"

// Standard headers.
#include <math.h>
#include <string.h>


DfColour g_backgroundColour = { 0xff353535 };
DfColour g_frameColour = { 0xff454545 };
DfColour g_buttonShadowColour = { 0xff2e2e2e };
DfColour g_buttonColour = { 0xff5a5a5a };
DfColour g_buttonHighlightColour = { 0xff6f6f6f };
DfColour g_normalTextColour = Colour(210, 210, 210, 255);
DfColour g_selectionColour = Colour(18, 71, 235);

double g_drawScale = 1.0;
int g_dragStartX;
int g_dragStartY;


// ****************************************************************************
// Misc functions
// ****************************************************************************

static int is_point_in_rect(int px, int py, int rx, int ry, int rw, int rh) {
    return px >= rx && 
           px < (rx + rw) &&
           py >= ry && 
           py < (ry + rh);
}


int is_mouse_in_bounds(DfWindow *win, int x, int y, int w, int h) {
    return is_point_in_rect(win->input.mouseX, win->input.mouseY, x, y, w, h);
}


void gui_do_frame(DfWindow *win) {
    if (win->input.lmbClicked) {
        g_dragStartX = win->input.mouseX;
        g_dragStartY = win->input.mouseY;
    }

    int scaleChanged = 0;
    if (win->input.keys[KEY_CONTROL]) {
        if (win->input.keyDowns[KEY_EQUALS] || win->input.keyDowns[KEY_PLUS_PAD]) {
            g_drawScale *= 1.1;
            scaleChanged = 1;
        }
        else if (win->input.keyDowns[KEY_MINUS] || win->input.keyDowns[KEY_MINUS_PAD]) {
            g_drawScale /= 1.1;
            scaleChanged = 1;
        }

    }

    if (!g_defaultFont || scaleChanged) {
        g_drawScale = ClampDouble(g_drawScale, 0.7, 3.0);
        double desired_height = g_drawScale * 13.0;
        int bestFontIdx = 0;
        char bestDeltaSoFar = fabs(df_prop.pixelHeights[0] - desired_height);
        for (int i = 1; i < df_prop.numSizes; i++) {
            double delta = fabs(desired_height - df_prop.pixelHeights[i]);
            if (delta < bestDeltaSoFar) {
                bestDeltaSoFar = delta;
                bestFontIdx = i;
            }
        }

        g_defaultFont = LoadFontFromMemory(df_prop.dataBlobs[bestFontIdx],
            df_prop.dataBlobsSizes[bestFontIdx]);
    }
}


void draw_sunken_box(DfBitmap *bmp, int x, int y, int w, int h) {
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

    int thickness = RoundToInt(g_drawScale * 1.5);
    DfColour dark = g_buttonShadowColour;
    RectFill(bmp, x, y, w, thickness, dark); // '1' pixels
    RectFill(bmp, x, y + h - thickness, w, thickness, dark); // '2' pixels
    RectFill(bmp, x, y + thickness, thickness, h - 2 * thickness, dark);
    RectFill(bmp, x + w - thickness, y + thickness, thickness, h - 2 * thickness, dark);

    x += thickness;
    y += thickness;
    w -= thickness * 2;
    h -= thickness * 2;
    RectFill(bmp, x, y, w, h, g_backgroundColour);
}


// ****************************************************************************
// V Scrollbar
// ****************************************************************************

int v_scrollbar_do(DfWindow *win, v_scrollbar_t *vs, int x, int y, int w, int h, int has_focus) {
    if (has_focus)
        vs->current_val -= win->input.mouseVelZ * 0.3;

    int mouse_in_bounds = is_mouse_in_bounds(win, x, y, w, h);
    if (win->input.lmbClicked && mouse_in_bounds) {
        vs->dragging = true;
        vs->current_val_at_drag_start = vs->current_val;
    }
    else if (!win->input.lmb) {
        vs->dragging = false;
    }

    if (vs->dragging) {
        int dragY = win->input.mouseY - g_dragStartY;
        if (dragY != 0)
            dragY = dragY;
        double scale = vs->maximum / (double)h;
        vs->current_val = vs->current_val_at_drag_start + dragY * scale;
    }

    if (vs->current_val < 0)
        vs->current_val = 0;
    else if (vs->current_val + vs->covered_range > vs->maximum)
        vs->current_val = vs->maximum - vs->covered_range;

    RectFill(win->bmp, x, y, w, h, g_frameColour);

    int handle_gap = g_drawScale;
    y += handle_gap;
    h -= handle_gap * 2;
    float inv_max = (float)h / (float)vs->maximum;
    float current_val_as_fraction = vs->current_val * inv_max;
    float covered_range_as_fraction = vs->covered_range * inv_max;
    int handle_x = x + handle_gap;
    int handle_y = y + current_val_as_fraction;
    int handle_w = w - 2 * handle_gap;
    int handle_h = covered_range_as_fraction;

    DfColour handleColour = g_buttonColour;
    if (mouse_in_bounds || vs->dragging)
        handleColour = g_buttonHighlightColour;
    RectFill(win->bmp, handle_x, handle_y, handle_w, handle_h, handleColour);
    return 0;
}


// ****************************************************************************
// Edit box
// ****************************************************************************

// Returns 1 if contents changed.
int edit_box_do(DfWindow *win, edit_box_t *eb, int x, int y, int w, int h) {
    draw_sunken_box(win->bmp, x, y, w, h);
    x += 2 * g_drawScale;
    y += 4 * g_drawScale;
    w -= 4 * g_drawScale;
    h -= 8 * g_drawScale;
    SetClipRect(win->bmp, x, y, w, h);

    double now = GetRealTime();
    if (now > eb->nextCursorToggleTime) {
        eb->cursorOn = !eb->cursorOn;
        eb->nextCursorToggleTime = now + 0.5;
    }

    if (win->input.keyDowns[KEY_LEFT]) {
        eb->cursorIdx = IntMax(0, eb->cursorIdx - 1);
        eb->nextCursorToggleTime = now;
    }
    else if (win->input.keyDowns[KEY_RIGHT]) {
        eb->cursorIdx = IntMin(strlen(eb->text), eb->cursorIdx + 1);
        eb->nextCursorToggleTime = now;
    }
    else if (win->input.keyDowns[KEY_HOME]) {
        eb->cursorIdx = 0;
        eb->nextCursorToggleTime = now;
    }
    else if (win->input.keyDowns[KEY_END]) {
        eb->cursorIdx = strlen(eb->text);
        eb->nextCursorToggleTime = now;
    }

    int contents_changed = 0;
    for (int i = 0; i < win->input.numKeysTyped; i++) {
        char c = win->input.keysTyped[i];
        if (c == 8) { // Backspace
            if (eb->cursorIdx > 0) {
                char *move_src = eb->text + eb->cursorIdx;
                unsigned move_size = strlen(move_src);
                memmove(move_src - 1, move_src, move_size);
                move_src[move_size - 1] = '\0';
                eb->cursorIdx--;
            }
        }
        else if (c == 127) { // Delete
            char *move_src = eb->text + eb->cursorIdx + 1;
            unsigned move_size = strlen(move_src);
            memmove(move_src - 1, move_src, move_size);
            move_src[move_size - 1] = '\0';
        }
        else {
            char *move_src = eb->text + eb->cursorIdx;
            unsigned move_size = strlen(move_src);
            memmove(move_src + 1, move_src, move_size);
            eb->text[eb->cursorIdx] = c;
            eb->text[sizeof(eb->text) - 1] = '\0';
            eb->cursorIdx = IntMin(eb->cursorIdx + 1, sizeof(eb->text) - 1);
        }

        contents_changed = 1;
        eb->nextCursorToggleTime = now;
    }

    DrawTextSimple(g_defaultFont, g_normalTextColour, win->bmp, x, y, eb->text);

    if (eb->cursorOn) {
        int cursorX = GetTextWidth(g_defaultFont, eb->text, eb->cursorIdx) + x;
        RectFill(win->bmp, cursorX, y, 2 * g_drawScale, g_defaultFont->charHeight, g_normalTextColour);
    }

    ClearClipRect(win->bmp);

    return contents_changed;
}


// ****************************************************************************
// List View
// ****************************************************************************

int list_view_do(DfWindow *win, list_view_t *lv, int x, int y, int w, int h) {
    draw_sunken_box(win->bmp, x, y, w, h);
    x += 2 * g_drawScale;
    y += 2 * g_drawScale;
    w -= 4 * g_drawScale;
    h -= 4 * g_drawScale;
    SetClipRect(win->bmp, x, y, w, h);

    const int num_rows = RoundToInt(h / (double)g_defaultFont->charHeight - 0.9);

    if (win->input.keyDowns[KEY_DOWN]) {
        lv->selected_item++;
        lv->first_display_item = IntMax(lv->selected_item - num_rows, lv->first_display_item);
    }
    else if (win->input.keyDowns[KEY_UP]) {
        lv->selected_item--;
        lv->first_display_item = IntMin(lv->selected_item, lv->first_display_item);
    }
    else if (win->input.keyDowns[KEY_PGDN]) {
        lv->selected_item += num_rows;
        lv->first_display_item += num_rows;
    }
    else if (win->input.keyDowns[KEY_PGUP]) {
        lv->selected_item -= num_rows;
        lv->first_display_item -= num_rows;
    }

    if (win->input.lmbClicked && is_mouse_in_bounds(win, x, y, w, h)) {
        int row = (win->input.mouseY - y) / g_defaultFont->charHeight;
        lv->selected_item = row + lv->first_display_item;
    }

    if (lv->selected_item >= lv->num_items || lv->num_items <= 0)
        lv->selected_item = lv->num_items - 1;
    else if (lv->selected_item < 0)
        lv->selected_item = 0;

    lv->first_display_item -= win->input.mouseVelZ / 36;
    lv->first_display_item = ClampInt(lv->first_display_item, 0, lv->num_items - num_rows);
    if (lv->num_items <= num_rows) lv->first_display_item = 0;

    int last_y = y + h;
    for (int i = lv->first_display_item; i < lv->num_items; i++) {
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

void text_view_empty(text_view_t *tv) {
    tv->text[0] = '\0';
    tv->selection_start_x = tv->selection_end_x;
    tv->selection_start_y = tv->selection_end_y;
}


void text_view_add_text(text_view_t *tv, char const *text) {
    int current_len = strlen(tv->text);
    int additional_len = strlen(text);
    int space = TEXT_VIEW_MAX_CHARS - current_len - 1;
    int amt_to_copy = IntMin(space, additional_len);
    memcpy(tv->text + current_len, text, amt_to_copy);
    tv->text[current_len + amt_to_copy] = '\0';
}


static char const *find_space(char const *c) {
    while (*c != ' ' && *c != '\n' && *c != '\0')
        c++;
    return c;
}


static int text_view_wrap_text(text_view_t *tv, int w) {
    int space_pixels = GetTextWidth(g_defaultFont, " ");
    char const *word_in = tv->text;
    char *word_out = tv->wrapped_text;
    int current_x = 0;
    int y = 0;

    // For each word...
    while (1) {
        char const *space = find_space(word_in);
        unsigned word_len = space - word_in;

        int num_pixels = GetTextWidth(g_defaultFont, word_in, word_len);
        if (current_x + num_pixels >= w) {
            current_x = 0;
            y += g_defaultFont->charHeight;
            *word_out = '\n';
            word_out++;
        }

        if (word_out + word_len >= tv->wrapped_text + sizeof(tv->wrapped_text)) {
            *word_out = '\0';
            break;
        }

        memcpy(word_out, word_in, word_len);
        word_out += word_len;

        current_x += num_pixels + space_pixels;
        word_in += word_len;
        *word_out = *space;
        word_out++;

        if (*space == '\n') {
            current_x = 0;
            y += g_defaultFont->charHeight;
        }

        if (*word_in == '\0') {
            *word_out = '\0';
            break;
        }

        word_in++;
    }

    return y;
}


static int get_selection_coords(text_view_t *tv, int *sx, int *sy, int *ex, int *ey) {
    int x_is_equal = (tv->selection_start_x == tv->selection_end_x);
    int y_is_equal = (tv->selection_start_y == tv->selection_end_y);
    if (x_is_equal && y_is_equal) {
        *sx = *sy = *ex = *ey = -1;
        return 0;
    }

    int swap_needed = (tv->selection_start_y > tv->selection_end_y);
    if (y_is_equal && tv->selection_start_x > tv->selection_end_x)
        swap_needed = 1;

    if (swap_needed) {
        *sx = tv->selection_end_x;
        *sy = tv->selection_end_y;
        *ex = tv->selection_start_x;
        *ey = tv->selection_start_y;
    }
    else {
        *sx = tv->selection_start_x;
        *sy = tv->selection_start_y;
        *ex = tv->selection_end_x;
        *ey = tv->selection_end_y;
    }

    return 1;
}


static int get_string_idx_from_pixel_offset(char const *s, int target_pixel_offset) {
    int pixel_offset = 0;
    for (int i = 0; s[i] != '\0'; i++) {
        pixel_offset += GetTextWidth(g_defaultFont, s + i, 1);
        if (pixel_offset > target_pixel_offset) {
            return i;
        }
    }

    return 0;
}


void text_view_do(DfWindow *win, text_view_t *tv, int x, int y, int w, int h) {
    int mouse_in_bounds = is_mouse_in_bounds(win, x, y, w, h);
    draw_sunken_box(win->bmp, x, y, w, h);

    int border_width = 2 * g_drawScale;
    x += 2 * border_width;
    y += 1 * border_width;
    w -= 4 * border_width;
    h -= 2 * border_width;
    SetClipRect(win->bmp, x, y, w, h);

    int scrollbar_w = 10 * g_drawScale;
    int scrollbar_x = x + w - scrollbar_w;
    int scrollbar_y = y + 1 * border_width;
    int scrollbar_h = h - 2 * border_width;

    int textRight = scrollbar_x;
    int space_pixels = GetTextWidth(g_defaultFont, " ");
    char const *line = tv->wrapped_text;

    int total_pixel_height = text_view_wrap_text(tv, w - scrollbar_w);

    tv->v_scrollbar.covered_range = scrollbar_h;
    tv->v_scrollbar.maximum = total_pixel_height;
    if (tv->v_scrollbar.maximum < scrollbar_h)
        tv->v_scrollbar.maximum = scrollbar_h;
    v_scrollbar_do(win, &tv->v_scrollbar, scrollbar_x, scrollbar_y, scrollbar_w, scrollbar_h, mouse_in_bounds);

    int sel_start_x, sel_start_y, sel_end_x, sel_end_y;
    get_selection_coords(tv, &sel_start_x, &sel_start_y, &sel_end_x, &sel_end_y);

    y -= tv->v_scrollbar.current_val;
    for (int line_num = 0; ; line_num++) {
        // Update selection block if mouse click on current line.
        if (mouse_in_bounds) {
            if (win->input.mouseY >= y && win->input.mouseY < (y + g_defaultFont->charHeight)) {
                if (win->input.lmbClicked) {
                    int pixel_offset = win->input.mouseX - x;
                    tv->selection_start_x = get_string_idx_from_pixel_offset(line, pixel_offset);
                    tv->selection_start_y = line_num;
                }

                if (win->input.lmb) {
                    int pixel_offset = win->input.mouseX - x;
                    tv->selection_end_x = get_string_idx_from_pixel_offset(line, pixel_offset);
                    tv->selection_end_y = line_num;
                }
            }
        }

        char const *end_of_line = strchr(line, '\n');
        if (!end_of_line) break;
        int line_len = end_of_line - line - 1;

        // Draw selection block
        if (line_num >= sel_start_y && line_num <= sel_end_y) {
            int start_idx = 0;
            int end_idx = line_len;
            if (line_num == sel_start_y)
                start_idx = sel_start_x;
            if (line_num == sel_end_y && (sel_end_x + 1) < line_len)
                end_idx = sel_end_x + 1;

            int sel_x = x + GetTextWidth(g_defaultFont, line, start_idx);
            int sel_w = GetTextWidth(g_defaultFont, line + start_idx, end_idx - start_idx);
            RectFill(win->bmp, sel_x, y, sel_w, g_defaultFont->charHeight, g_selectionColour);
        }

        DrawTextSimpleLen(g_defaultFont, g_normalTextColour, win->bmp,
            x, y, line, line_len);
        line = end_of_line + 1;
        y += g_defaultFont->charHeight;
    }

    ClearClipRect(win->bmp);
}


char const *text_view_get_selected_text(text_view_t *tv, int *num_chars) {
    int sel_start_x, sel_start_y, sel_end_x, sel_end_y;
    if (get_selection_coords(tv, &sel_start_x, &sel_start_y, &sel_end_x, &sel_end_y) == 0)
        return NULL;

    // Find the start of the selected text.
    char const *line = tv->wrapped_text;
    int line_num = 0;
    while (line_num < sel_start_y) {
        char const *end_of_line = strchr(line, '\n');
        DebugAssert(end_of_line);
        line = end_of_line + 1;
        line_num++;
    }

    char const *start_of_block = line + sel_start_x;

    // Find the end of the selected text.
    while (line_num < sel_end_y) {
        char const *end_of_line = strchr(line, '\n');
        DebugAssert(end_of_line);
        line = end_of_line + 1;
        line_num++;
    }

    *num_chars = line - start_of_block + sel_end_x + 1;

    return start_of_block;
}



// ****************************************************************************
// Button
// ****************************************************************************

int button_do(DfWindow *win, button_t *b, int x, int y, int w, int h) {
    int mouse_in_bounds = is_mouse_in_bounds(win, x, y, w, h);
    DfColour handleColour = g_buttonColour;
    if (mouse_in_bounds)
        handleColour = g_buttonHighlightColour;
    RectFill(win->bmp, x, y, w, h, handleColour);
    DrawTextCentre(g_defaultFont, g_normalTextColour, win->bmp, 
        x + w / 2, y + h / 5, b->label);
    return mouse_in_bounds && win->input.lmbClicked;
}
