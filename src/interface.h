#include "paint.h"
#include <raylib.h>
#include "raygui.h"

// Colors
#define INTERFACE_COLOR (Color) {27, 30, 32, 255}
#define BORDER_COLOR (Color){126, 138, 150, 255}
#define TOOLBAR_COLOR (Color){42, 46, 50, 255}
#define HOVER_FADE 0.6

void init_gui();
void handle_ui(Window *window, Canvas *canvas, Brush *brush, Tools *current_tool);
void export_dialog();
void import_dialog();
bool is_dialog_active();