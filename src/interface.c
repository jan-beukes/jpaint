#include "interface.h"
#include "raygui.h"
#include <raylib.h>

#define COLOR_COUNT 23

void init_gui() {
    GuiLoadStyle("res/style_dark.rgs");
}

void handle_ui_events(Window *window, Canvas *canvas, Brush *brush, Tools *current_tool) {
    static bool show_color_window;
    Vector2 mouse_pos = GetMousePosition();
    // Toolbar
    DrawRectangle(0, 0, window->l_border, window->height, TOOLBAR_COLOR);
    DrawLine(window->l_border, 0, window->l_border, window->height, INTERFACE_COLOR);
    

    // Tool settings
    const int color_pad = 8;
    Rectangle color_window_rect = {color_pad, window->height - 80, window->l_border - 2*color_pad, window->l_border - 2*color_pad};
    if (CheckCollisionPointRec(mouse_pos, color_window_rect)) {
        DrawRectangleRec(color_window_rect, ColorAlpha(brush->color, 0.8));
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            show_color_window = true;
    } else {
        DrawRectangleRec(color_window_rect, brush->color);
    }

    // Color window
    if (show_color_window) {
        Rectangle window_box = {window->l_border, color_window_rect.y - color_window_rect.height*4, window->width/3, window->height/4};
        if (CheckCollisionPointRec(mouse_pos, window_box)) {
            *current_tool = NONE;
        } else {
            *current_tool = BRUSH;
        }
        
        if (GuiWindowBox(window_box, "")) {
            show_color_window = false;
            *current_tool = BRUSH;
        }

        const int colors_cols = (int)COLOR_COUNT/3;
        const int color_rows = 4;
        const int padding = 2;
        const float color_rect_width = (window_box.width/colors_cols) - padding; 
        const float color_rect_height = (window_box.height/color_rows) - padding;

        for (int i = 0; i < COLOR_COUNT; i++) {
            int j = i % 7;
            int x = color_window_rect.x + j*color_rect_width + padding;
            int y = color_window_rect.y + (i/colors_cols)*color_rect_height + padding;
            Rectangle color_rect = {x ,y ,color_rect_width, color_rect_height};
            DrawRectangleRec(color_rect, RAYWHITE);
        }

    }
    
    // Scale
    DrawText(TextFormat("%.0f%%", canvas->scale * 100), 5, window->height - 20, 18, WHITE);
}