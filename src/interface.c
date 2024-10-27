#include <stdlib.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <string.h>
#include "interface.h"
#include "raygui.h"
#include "gui_window_file_dialog.h"
#include "../res/images.h"

#define COLOR_COUNT 23
#define BG_COLOR_COUNT 4
#define SBAR_HEIGHT 24

// Color pallete
Color raylib_colors[COLOR_COUNT] = {
    LIGHTGRAY, YELLOW, PINK, GREEN, SKYBLUE, PURPLE, BEIGE,    
    GRAY, GOLD, RED, LIME, BLUE, VIOLET, BROWN, 
    DARKGRAY, ORANGE, MAROON, DARKGREEN, DARKBLUE, DARKPURPLE, DARKBROWN,
    BLACK, WHITE
    };
Color background_colors[BG_COLOR_COUNT] = {WHITE, (Color){200, 200, 200, 255}, DARKGRAY, BLACK};

typedef enum {
    NEW,
    OPEN,
    SAVE,
    SAVE_AS
} MenuButtons;


// Global idk man
#define TOOL_COUNT 4
static Texture menu_texture, transparent_texture;
static Texture tool_textures[TOOL_COUNT];

static GuiWindowFileDialogState dialog_state;
static bool create_canvas_active;
static Rectangle create_canvas_bounds;

void init_gui (Window *window) {
    //---Gui Style---
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(RAYWHITE));
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(INTERFACE_COLOR));
    GuiSetStyle(LISTVIEW, BASE_COLOR_FOCUSED, ColorToInt(INTERFACE_COLOR));
    GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, ColorToInt(INTERFACE_COLOR));
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, ColorToInt(BORDER_COLOR));
    GuiSetStyle(DEFAULT, LINE_COLOR, ColorToInt(TOOLBAR_COLOR));
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(TOOLBAR_COLOR));
    
    // File Dialog
    char *home_dir;

    #if defined(_WIN32) || defined(_WIN64)
        home_dir = getenv("USERPROFILE");
    #else
        home_dir = getenv("HOME");
    #endif
    if (home_dir == NULL) {
        TraceLog(LOG_ERROR ,"Could not retrieve home directory.\n");
        exit(1);
    }

    dialog_state = InitGuiWindowFileDialog("");
    strcpy(dialog_state.filterExt , "DIR;.png");
    
    float width = 0.75*dialog_state.windowBounds.width, height = 0.75*dialog_state.windowBounds.height;
    create_canvas_bounds = (Rectangle){(window->width-width)/2, (window->height-height)/2, width, height};

    // Assets
    Image image;
    image = LoadImageFromMemory(".png", transparent_png, transparent_png_len);
    transparent_texture = LoadTextureFromImage(image);
    UnloadImage(image);

    image = LoadImageFromMemory(".png", menu_png, menu_png_len);
    menu_texture = LoadTextureFromImage(image);
    UnloadImage(image);

    image = LoadImageFromMemory(".png", paintbrush_png, paintbrush_png_len);
    tool_textures[0] =LoadTextureFromImage(image); 
    UnloadImage(image);
    image = LoadImageFromMemory(".png", bucket_fill_png, bucket_fill_png_len);
    tool_textures[1] =LoadTextureFromImage(image); 
    UnloadImage(image);
    image = LoadImageFromMemory(".png", move_tool_png, move_tool_png_len);
    tool_textures[2] =LoadTextureFromImage(image); 
    UnloadImage(image);
    image = LoadImageFromMemory(".png", color_picker_png, color_picker_png_len);
    tool_textures[3] =LoadTextureFromImage(image); 
    UnloadImage(image);
}

void enable_create_canvas_gui() {
    create_canvas_active = true;
}

bool is_dialog_active() {
    return dialog_state.windowActive || create_canvas_active;
}

void export_dialog() {
    dialog_state.windowActive = true;
    dialog_state.saveFileMode = true;
    printf("export dialog\n");
}

void import_dialog() {
    dialog_state.windowActive = true;
    dialog_state.saveFileMode = false;
    printf("import dialog\n");
}

// Color selector window starting at origin rect 
bool color_selector(Rectangle origins_rect, Rectangle window_box, Window *window, Color *color) {
    Vector2 mouse_pos = GetMousePosition();
    
    if (GuiWindowBox(window_box, "")) {
        return false;
    }

    const int colors_cols = (int)COLOR_COUNT/3;
    const int color_rows = 4;
    const int padding = 8;
    const float color_rect_size = (window_box.width/colors_cols) - padding - padding/colors_cols;

    // Color selection
    for (int i = 0; i < COLOR_COUNT; i++) {
        int j = i % 7;
        int x = window_box.x + j*(color_rect_size + padding) + padding;
        int y = SBAR_HEIGHT + window_box.y + (i/colors_cols)*(color_rect_size + padding) + padding;
        Rectangle color_rect = {x ,y ,color_rect_size, color_rect_size};

        if (CheckCollisionPointRec(mouse_pos, color_rect)) {
            DrawRectangleRec(color_rect, Fade(raylib_colors[i], HOVER_FADE));
            // selected
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                *color = raylib_colors[i];
                return true;
            }
        } else {
            DrawRectangleRec(color_rect, raylib_colors[i]);
        }
    }

    return true;
}

// Setting Cursor
void set_mouse_cursor(Tools current_tool, Rectangle canvas_area) {
    if (!CheckCollisionPointRec(GetMousePosition(), canvas_area)){
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }
    else if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON) || current_tool == MOVE) {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    } else {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT); // Default
    }
}

static void gui_menu(Window *window, Rectangle bounds, int padding) {
    #define MENU_BUTTON_COUNT 4 
    DrawRectangleRec(bounds, TOOLBAR_COLOR);

    for (int i = 0; i < MENU_BUTTON_COUNT; i++) {
        Vector2 size = {(bounds.width - padding*(2 + MENU_BUTTON_COUNT))/MENU_BUTTON_COUNT, bounds.height - 2*padding};
        Rectangle rect = {window->l_border + i*(padding + size.x) + padding, padding, size.x, size.y};
        char *text;
        switch (i) {
            case NEW: if (GuiButton(rect, "New")) enable_create_canvas_gui();
                break;
            case OPEN: if (GuiButton(rect, "Open")) import_dialog();
                break;
            case SAVE: if (GuiButton(rect, "Save")) export_canvas(NULL);
                break;
            case SAVE_AS: if (GuiButton(rect, "Save as")) export_dialog();
                break;
            default:
                printf("Menu %d not doesnt exist\n", i);
                exit(1);
        }
        
    }

}

// UI to create a new canvas
void create_canvas_gui(Window *window) {
    Vector2 mouse_pos = GetMousePosition();
    static Vector2 drag_offset = {0};
    static bool dragging = false;
    static int height = 64;
    static int width = 64;
    static Color background = WHITE;

    Rectangle *bounds = &create_canvas_bounds; 

    // Dragging
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // Window can be dragged from the top window bar
        if (CheckCollisionPointRec(mouse_pos, (Rectangle){bounds->x, bounds->y, bounds->width, SBAR_HEIGHT})) {
            dragging = true;
            drag_offset.x = mouse_pos.x - bounds->x;
            drag_offset.y = mouse_pos.y - bounds->y;
        }
    }
    if (dragging) {
        bounds->x = (mouse_pos.x - drag_offset.x);
        bounds->y = (mouse_pos.y - drag_offset.y);
    }
    // Check screen limits to avoid moving out of screen
    if (bounds->width < window->width) {
        if (bounds->x < 0) bounds->x = 0;
        else if (bounds->x > (window->width - bounds->width)) bounds->x = window->width - bounds->width;
    }
    if (bounds->height < window->height) {
        if (bounds->y < 0) bounds->y = 0;
        else if (bounds->y > (window->height - bounds->height)) bounds->y = window->height - bounds->height;
    }
    
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) dragging = false;
    
    create_canvas_active = !GuiWindowBox(*bounds, "New Canvas");

    // Width
    float w = bounds->width, h = bounds->height;
    float size = 0.1*w;
    Rectangle box = {bounds->x + size, bounds->y + 1.8*size, size, size};
    
    DrawText(TextFormat("Width: %d", width), box.x, bounds->y + 30, 20, WHITE);
    if (GuiButton(box, "+")) width = MIN(width*2, 4096); 
    box.x += 0.1*w;
    if (GuiButton(box, "-")) width = MAX(width/2, 8);;
    
    // Height
    box.x = bounds->x + w - 4*size;
    DrawText(TextFormat("Height: %d", height), box.x, bounds->y + 30, 20, WHITE);
    if (GuiButton(box, "+")) height = MIN(height*2, 4096); 
    box.x += 0.1*w;
    if (GuiButton(box, "-")) height = MAX(height/2, 8);

    // BG colors
    const int padding = 8;
    const float color_rect_size = (bounds->width/BG_COLOR_COUNT)/2;
    DrawText("Background", bounds->x + padding, box.y + box.height + 2*padding, 18, WHITE);
    
    // Transparent Color
    Rectangle color_rect = {bounds->x + padding , bounds->y + bounds->height/1.8 ,color_rect_size, color_rect_size};
    if (CheckCollisionPointRec(mouse_pos, color_rect)) {
        DrawTexturePro(transparent_texture, (Rectangle){0,0,transparent_texture.width, transparent_texture.height},
                       color_rect, Vector2Zero(), 0, Fade(WHITE, HOVER_FADE));
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            background = (Color) {0,0,0,0};
    } else {
        DrawTexturePro(transparent_texture, (Rectangle){0,0,transparent_texture.width, transparent_texture.height},
                       color_rect, Vector2Zero(), 0, WHITE);
    }
    if (background.a == 0) DrawRectangleLinesEx(color_rect, 3, SKYBLUE);
    
    // Colors
    for (int i = 0; i < BG_COLOR_COUNT; i++) {
        color_rect.x += padding + color_rect_size;
        if (CheckCollisionPointRec(mouse_pos, color_rect)) {
            DrawRectangleRec(color_rect, Fade(background_colors[i], HOVER_FADE));
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                background = background_colors[i];
        } else {
            DrawRectangleRec(color_rect, background_colors[i]);
        }
        if (background.r == background_colors[i].r && background.a != 0)
            DrawRectangleLinesEx(color_rect, 3, SKYBLUE);
    }

    // Create Button
    float button_h = bounds->height/6;
    float button_w = bounds->width/2;
    Rectangle button = {bounds->x + button_w/2, bounds->y + bounds->height - button_h - padding, button_w, button_h};
    if (GuiButton(button, "Create")) {
        create_canvas_active = false;
        init_canvas(width, height, background);
    }
}

// Main UI loop
void handle_ui(Window *window, Canvas *canvas, Brush *brush, Tools *current_tool) {
    const int padding = 8;
    static Tools prev_tool = BRUSH;
    static bool color_window_active;
    static bool menu_active;
    Vector2 mouse_pos = GetMousePosition();

    if (*current_tool != NONE) prev_tool = *current_tool; // set prev tool

    set_mouse_cursor(*current_tool, window->canvas_area);

    //---Toolbar---
    DrawRectangle(0, 0, window->l_border, window->height, TOOLBAR_COLOR);
    DrawLine(window->l_border, 0, window->l_border, window->height, INTERFACE_COLOR);

    // ---Menu---
    Rectangle source = {0, 0, menu_texture.width, menu_texture.width};
    float size = window->l_border/2.0;
    Rectangle dest = {size/2, size/4, size, size}; 
    if (CheckCollisionPointRec(mouse_pos, dest)) {
        DrawTexturePro(menu_texture, source, dest, Vector2Zero(), 0, WHITE);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            menu_active = !menu_active;
        }
    } else {
        DrawTexturePro(menu_texture, source, dest, Vector2Zero(), 0, Fade(WHITE, HOVER_FADE));
    }

    if (menu_active) {
        float menu_h = 0.75 * window->l_border;
        float menu_w = 5 * window->l_border;
        Rectangle bounds = {window->l_border, 0, menu_w, menu_h};
        if (CheckCollisionPointRec(GetMousePosition(), bounds)) {
            if (*current_tool != NONE) *current_tool = NONE;
        } else {
            if (*current_tool == NONE) *current_tool = prev_tool;
        }
        gui_menu(window, bounds, padding);
    } 

    // Tool buttons
    int icon_pad = 1.5*padding;
    int start_y = size + icon_pad;
    for (int i = 0; i < TOOL_COUNT; i++) {
        Rectangle source = {0, 0, tool_textures[0].width, tool_textures[0].width};
        float size = window->l_border - 2 * icon_pad;
        dest = (Rectangle){icon_pad,start_y + icon_pad + i * (size + icon_pad), size, size}; 
        if (CheckCollisionPointRec(mouse_pos, dest) || *current_tool == i) {
            DrawTexturePro(tool_textures[i], source, dest, Vector2Zero(), 0, WHITE);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                *current_tool = i;
            }
        } else {
            DrawTexturePro(tool_textures[i], source, dest, Vector2Zero(), 0, Fade(WHITE, HOVER_FADE));
        }
    }

    //---Bottom Info---
    start_y = window->height - 2.2*window->l_border;
    
    // only draw if space for them
    if (start_y > dest.y + dest.height) {
     
        DrawText(TextFormat("Brush"), 5, start_y, 18, Fade(RAYWHITE, HOVER_FADE));
        DrawText(TextFormat("%.1f", (brush->radius)/10), icon_pad, start_y + 18 + padding, 18, WHITE);
        DrawText(TextFormat("%.0f%%", canvas->scale * 100), 5, window->height - 20, 18, WHITE);

        // Color Selection
        Rectangle color_select_rect = {padding, window->height - 80, window->l_border - 2*padding, window->l_border - 2*padding};
        if (CheckCollisionPointRec(mouse_pos, color_select_rect)) {
            DrawRectangleRec(color_select_rect, Fade(brush->color, HOVER_FADE));
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                color_window_active = !color_window_active;
        } else {
            DrawRectangleRec(color_select_rect, brush->color);
        }
        if (IsKeyPressed(KEY_Q)) color_window_active = !color_window_active; // Keybind

        // Color window 
        if (color_window_active) {
            Rectangle window_box = {window->l_border, color_select_rect.y - color_select_rect.height*4, 5*window->l_border, 3.4*window->l_border};
            if (CheckCollisionPointRec(mouse_pos, window_box)) {
                if (*current_tool != NONE) *current_tool = NONE;
            } else {
                if (*current_tool == NONE) *current_tool = prev_tool;
            }
            color_window_active = color_selector(color_select_rect,window_box, window, &brush->color);
        }
    }
    
    if (create_canvas_active) {
        if (*current_tool != NONE)
            *current_tool = NONE;
        create_canvas_gui(window);
    } 

    // ---File Dialog---
    if (dialog_state.windowActive && *current_tool != NONE) 
        *current_tool = NONE;

    GuiWindowFileDialog(&dialog_state);

    if (dialog_state.SelectFilePressed) {
        
        dialog_state.SelectFilePressed = false;
        char *sep;
        #if defined(_WIN32) || defined(_WIN64)
            sep = "\\";
        #else
            sep = "/";
        #endif
        
        // Export
        if (dialog_state.saveFileMode) {
            char *filename = dialog_state.fileNameText;
            if (GetFileLength(filename) == 0) {
                strcat(filename, "painting.png");
            } else if (!IsFileExtension(filename, ".png")) {
                strcat(filename, ".png");
            }
            char buff[1024];
            sprintf(buff, "%s%s%s", dialog_state.dirPathText, sep, filename);
            export_canvas(buff);
        // Import
        } else {
            char buff[1024];
            sprintf(buff, "%s%s%s", dialog_state.dirPathText, sep, dialog_state.fileNameText);
            printf("LOADING: %s\n", buff);
            load_canvas(buff);
        }
        
        // Set tool after export/import
        *current_tool = prev_tool;
    } 

}

