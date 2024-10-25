#include "paint.h"
#include <raylib.h>
#include "raygui.h"

// Colors
#define INTERFACE_COLOR (Color) {27, 30, 32, 255}
#define TOOLBAR_COLOR (Color){42, 46, 50, 255}



void init_gui();
void handle_ui_events(Window *window, Canvas *canvas, Brush *brush, Tools *current_tool);