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
#define HOVER_FADE 0.6

Color raylib_colors[COLOR_COUNT] = {
    LIGHTGRAY, YELLOW, PINK, GREEN, SKYBLUE, PURPLE, BEIGE,    
    GRAY, GOLD, RED, LIME, BLUE, VIOLET, BROWN, 
    DARKGRAY, ORANGE, MAROON, DARKGREEN, DARKBLUE, DARKPURPLE, DARKBROWN,
    BLACK, WHITE
    };

//MAGENTA

#define TOOL_COUNT 4
static Texture tool_textures[TOOL_COUNT];
static GuiWindowFileDialogState dialog_state;

void init_gui() {
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

    dialog_state = InitGuiWindowFileDialog(".");

    // Assets
    Image image;
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

Canvas new_canvas(Window *window) {
    // UI to create a new canvas
}

bool is_dialog_active() {
    return dialog_state.windowActive;
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
bool color_selector(Rectangle origins_rect, Window *window, Tools *current_tool, Brush *brush) {
    Vector2 mouse_pos = GetMousePosition();
    static Tools prev_tool = NONE;

    Rectangle window_box = {window->l_border, origins_rect.y - origins_rect.height*4, 5*window->l_border, 3.4*window->l_border};
    // Disable brush when selecting
    if (CheckCollisionPointRec(mouse_pos, window_box)) {
        if (prev_tool == NONE || *current_tool != NONE) prev_tool = *current_tool; // set prev tool
        *current_tool = NONE; 
    } else if (prev_tool != NONE && *current_tool == NONE) {
        *current_tool = prev_tool;
    }
    
    if (GuiWindowBox(window_box, "")) {
        // closed
        *current_tool = prev_tool;
        return false;
    }

    #define SBAR_HEIGHT 24
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
                brush->color = raylib_colors[i];
                return true;
            }
        } else {
            DrawRectangleRec(color_rect, raylib_colors[i]);
        }
    }

    return true;
}

void handle_ui(Window *window, Canvas *canvas, Brush *brush, Tools *current_tool) {
    const int padding = 8;

    static bool show_color_window;
    Vector2 mouse_pos = GetMousePosition();
    
    //---Toolbar---
    DrawRectangle(0, 0, window->l_border, window->height, TOOLBAR_COLOR);
    DrawLine(window->l_border, 0, window->l_border, window->height, INTERFACE_COLOR);

    // Tool buttons
    for (int i = 0; i < TOOL_COUNT; i++) {
        int icon_pad = 1.5*padding;

        Rectangle source = {0, 0, tool_textures[0].width, tool_textures[0].width};
        float size = window->l_border- 2 * icon_pad;
        Rectangle dest = {icon_pad, icon_pad + i * (size + icon_pad), size, size}; 

        if (CheckCollisionPointRec(mouse_pos, dest) || *current_tool == i) {
            DrawTexturePro(tool_textures[i], source, dest, Vector2Zero(), 0, WHITE);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                *current_tool = i;
            }
        } else {
            DrawTexturePro(tool_textures[i], source, dest, Vector2Zero(), 0, Fade(WHITE, HOVER_FADE));
        }
    }


    //---Tool settings---
    
    // Color Selection
    Rectangle color_select_rect = {padding, window->height - 80, window->l_border - 2*padding, window->l_border - 2*padding};
    if (CheckCollisionPointRec(mouse_pos, color_select_rect)) {
        DrawRectangleRec(color_select_rect, Fade(brush->color, HOVER_FADE));
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            show_color_window = !show_color_window;
    } else {
        DrawRectangleRec(color_select_rect, brush->color);
    }
    if (IsKeyPressed(KEY_Q)) show_color_window = !show_color_window; // Keybind


    // Color window 
    if (show_color_window)   
        show_color_window = color_selector(color_select_rect, window, current_tool, brush);

    // Scale
    DrawText(TextFormat("%.0f%%", canvas->scale * 100), 5, window->height - 20, 18, WHITE);


    // ---Handle Dialog---
    if (dialog_state.windowActive && *current_tool != NONE) 
        *current_tool = NONE;
    
    GuiWindowFileDialog(&dialog_state);
    if (dialog_state.SelectFilePressed) {
        dialog_state.SelectFilePressed = false;

        // Export
        if (dialog_state.saveFileMode) {
            char *filename = dialog_state.fileNameText;
            int len = strlen(filename);
            // no .png extension
            if (len < 4 || strcmp(filename + len - 4, ".png") != 0) {
                strcat(filename, ".png");
            }
            export_canvas(dialog_state.fileNameText);
        // Import
        } else {
            
        }
        // Set tool to brush after export/import
        *current_tool = BRUSH;
    } 

}

