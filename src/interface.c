#include "interface.h"

void handle_ui_events(Window *window, Canvas *canvas, Brush *brush) {
    // Toolbar
    DrawRectangle(0, 0, window->l_border, window->height, TOOLBAR_COLOR);
    DrawLine(window->l_border, 0, window->l_border, window->height, INTERFACE_COLOR);
    
    
    
    // Scale
    DrawText(TextFormat("%.0f%%", canvas->scale * 100), 5, window->height - 20, 18, WHITE);
}