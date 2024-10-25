#include "interface.h"
#include "raygui.h"
#include <raylib.h>

#define COLOR_COUNT 23

Color raylib_colors[COLOR_COUNT] = {
    LIGHTGRAY, YELLOW, PINK, GREEN, SKYBLUE, PURPLE, BEIGE,    
    GRAY, GOLD, RED, LIME, BLUE, VIOLET, BROWN, 
    DARKGRAY, ORANGE, MAROON, DARKGREEN, DARKBLUE, DARKPURPLE, DARKBROWN,
    BLACK, WHITE
    };

//MAGENTA


void init_gui() {
    GuiLoadStyle("res/style_dark.rgs");
}

void color_picker(Rectangle rect, Color *value) {
    // Move color picker code here
}

void handle_ui(Window *window, Canvas *canvas, Brush *brush, Tools *current_tool) {
    static bool show_color_window;
    Vector2 mouse_pos = GetMousePosition();
    
    // Toolbar
    DrawRectangle(0, 0, window->l_border, window->height, TOOLBAR_COLOR);
    DrawLine(window->l_border, 0, window->l_border, window->height, INTERFACE_COLOR);
    
    // Tool settings
    const int color_pad = 8;

    // Color Selection
    Rectangle color_select_rect = {color_pad, window->height - 80, window->l_border - 2*color_pad, window->l_border - 2*color_pad};
    if (CheckCollisionPointRec(mouse_pos, color_select_rect)) {
        DrawRectangleRec(color_select_rect, Fade(brush->color, 0.8));
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            show_color_window = !show_color_window;
    } else {
        DrawRectangleRec(color_select_rect, brush->color);
    }

    // Color window
    if (show_color_window) {
        Rectangle window_box = {window->l_border, color_select_rect.y - color_select_rect.height*4, 5*window->l_border, 3.4*window->l_border};
        // Disable brush when selecting
        if (CheckCollisionPointRec(mouse_pos, window_box)) {
            *current_tool = NONE;
        } else {
            *current_tool = BRUSH;
        }
        
        if (GuiWindowBox(window_box, "")) {
            show_color_window = false;
            *current_tool = BRUSH;
        }

        #define STATUS_BAR 24
        const int colors_cols = (int)COLOR_COUNT/3;
        const int color_rows = 4;
        const int padding = 8;
        const float color_rect_size = (window_box.width/colors_cols) - padding - padding/colors_cols;

        // Color selection
        for (int i = 0; i < COLOR_COUNT; i++) {
            int j = i % 7;
            int x = window_box.x + j*(color_rect_size + padding) + padding;
            int y = STATUS_BAR + window_box.y + (i/colors_cols)*(color_rect_size + padding) + padding;
            Rectangle color_rect = {x ,y ,color_rect_size, color_rect_size};

            if (CheckCollisionPointRec(mouse_pos, color_rect)) {
                DrawRectangleRec(color_rect, Fade(raylib_colors[i], 0.8));
                // selected
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    brush->color = raylib_colors[i];
                }
            } else {
                DrawRectangleRec(color_rect, raylib_colors[i]);
            }
        }

    }
    
    // Scale
    DrawText(TextFormat("%.0f%%", canvas->scale * 100), 5, window->height - 20, 18, WHITE);
}