#include "paint.h"
#include <raylib.h>

// Colors
#define INTERFACE_COLOR (Color) {40, 40, 40, 255}
#define HIGHLIGHT_COLOR (Color){204, 190, 155, 255}
#define TOOLBAR_COLOR (Color){30, 30, 30, 255}
#define HOVER_FADE 0.6

extern Texture transparent_bg_texture;

void init_gui(Window *window);
void deinit_gui();
void enable_create_canvas_gui();
void switch_brush_texture(bool eraser);
void create_canvas_gui(Window *window);
void handle_ui(Window *window, Canvas *canvas, Brush *brush, Tools *current_tool);
void export_dialog();
void import_dialog();
bool is_dialog_active();
