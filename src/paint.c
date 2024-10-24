#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include "paint.h"
#include "interface.h"

#define ZOOM_STEP 0.05

// Globals
Window window;
Canvas canvas;
Brush brush;
Tools current_tool;

// Convert a window position to a canvas position
Vector2 window_to_canvas(Vector2 screen_pos) {
    // scale to canvas resolution based on position on the canvas area 
    Vector2 pos = {(screen_pos.x - window.canvas_area.x)/window.canvas_area.width*canvas.width,
                   (screen_pos.y - window.canvas_area.y)/window.canvas_area.height*canvas.height};
    return pos;
}

// Convert a canvas position to a window position
Vector2 canvas_to_window(Vector2 canvas_pos) {
    Vector2 pos = {window.canvas_area.x + (canvas_pos.x/canvas.width)*window.canvas_area.width,
                   window.canvas_area.y + (canvas_pos.y/canvas.height)*window.canvas_area.height};
    return pos;
}
 
// Paint to the canvas Texture
void paint_to_canvas() {
    static bool was_on_canvas = false;
    static Vector2 prev_canvas_pos; // This feels sus
    Vector2 mouse_pos = GetMousePosition();

    // Check if mouse is on the canvas
    if (!CheckCollisionPointRec(mouse_pos, window.canvas_area)) {
        was_on_canvas = false;
        return;
    }
    Vector2 canvas_pos = window_to_canvas(mouse_pos);
    if (!was_on_canvas) prev_canvas_pos = canvas_pos; // initialize

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        brush.drawing = true;
        bool draw_pixel = brush.radius == 1.0;
        Color color = brush.eraser ? canvas.background : brush.color;
        
        BeginTextureMode(canvas.rtexture);
        // fill large displacement with circles
        float distance = Vector2Distance(canvas_pos, prev_canvas_pos);
        if (distance > brush.radius) {
            float step_size = MIN(brush.radius * 0.5, 2.0);  // Adjust the step size
            int steps = (int)(distance / step_size);
            for (int i = 0; i <= steps; i++) {
                float t = (float)i / (float)steps;
                Vector2 interp_pos = Vector2Lerp(prev_canvas_pos, canvas_pos, t);
                if (!draw_pixel)
                    DrawCircleV(interp_pos, brush.radius, color);
                else
                    DrawPixel(interp_pos.x, interp_pos.y, color);
            }
        }

        // At mouse pos
        if (!draw_pixel) {
            DrawCircleV(canvas_pos, brush.radius, color);
        } else {
            DrawPixel(canvas_pos.x, canvas_pos.y, color);
        }
        EndTextureMode();
    } else { 
        brush.drawing = false;
    }
    prev_canvas_pos = canvas_pos; // Set for next frame
    was_on_canvas = true;
}

void set_tools() {
    // ---TOOLS---
    if (IsKeyPressed(KEY_E)) {
        current_tool = BRUSH;
        brush.eraser = !brush.eraser;
    }
    if (IsKeyPressed(KEY_B)) {
        current_tool = BRUSH;
        brush.eraser = false;
    }
    if (IsKeyPressed(KEY_H)) {
        current_tool = HAND;
    }

    // Setting Cursor
    if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON) || current_tool == HAND) {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    } else {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT); // Default
    }
}

// handle Keyboard/Mouse input events
void handle_user_input(float dt) {

    // Clear Canvas
    if (IsKeyPressed(KEY_C)) {
        BeginTextureMode(canvas.rtexture);
        ClearBackground(canvas.background);
        EndTextureMode();
    }
    
    // Handle current tools and switching between modes
    set_tools();

    // Canvas Movement
    bool canvas_move = IsMouseButtonDown(MOUSE_MIDDLE_BUTTON) || 
                       (current_tool == HAND && IsMouseButtonDown(MOUSE_BUTTON_LEFT));
    if (canvas_move) {
        Vector2 delta = GetMouseDelta();
        Vector2 canvas_area_pos = (Vector2){window.canvas_area.x, window.canvas_area.y};
        canvas_area_pos = Vector2Add(canvas_area_pos, delta);
        window.canvas_area.x = canvas_area_pos.x;
        window.canvas_area.y = canvas_area_pos.y;
    }

    //---Zoom---
    int zoom_key = 0;
    if (IsKeyDown(KEY_LEFT_CONTROL)) {
        if (IsKeyPressed(KEY_EQUAL)) {
            zoom_key = 1;
        } else if (IsKeyPressed(KEY_MINUS)) {
            zoom_key = -1;
        }
    }
    float mouse_scroll = GetMouseWheelMove();
    if (IsKeyDown(KEY_LEFT_CONTROL) && mouse_scroll != 0 || zoom_key) {
        int zoom_dir = zoom_key ? zoom_key : mouse_scroll;
        float _zoom_step = zoom_key ? ZOOM_STEP*3 : ZOOM_STEP;
        float zoom = zoom_dir > 0 ? (1+_zoom_step) : (1-_zoom_step);

        canvas.scale = MAX(canvas.scale*zoom, MIN_SCALE);
        if (canvas.scale > MIN_SCALE) {
            Vector2 prev_canvas_pos = window_to_canvas(GetMousePosition());
            window.canvas_area.width = canvas.width*canvas.scale;
            window.canvas_area.height = canvas.height*canvas.scale;
            Vector2 window_pos = canvas_to_window(prev_canvas_pos);
            
            // Move canvas to keep mouse on same position
            Vector2 displacement = Vector2Subtract(GetMousePosition(), window_pos);
            window.canvas_area.x += displacement.x;
            window.canvas_area.y += displacement.y;
        }
    } else if (mouse_scroll != 0) {
        float resize = mouse_scroll > 0 ? (1+BRUSH_RESIZE_STEP) : MAX((1-BRUSH_RESIZE_STEP), 0);
        brush.radius =  MAX(brush.radius*resize, 1);
    }

}


void draw_to_overlay(RenderTexture overlay) {
    // Clear Overlay texture
    BeginTextureMode(overlay);
    ClearBackground((Color){0,0,0,0});
    EndTextureMode();

    if (current_tool == BRUSH) {
        // Brush
        Vector2 mouse_pos = GetMousePosition();
        bool on_canvas = CheckCollisionPointRec(mouse_pos, window.canvas_area);
        if (!brush.drawing && on_canvas && !IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)) {
            bool draw_pixel = brush.radius == 1.0;
            Color color = brush.eraser ? canvas.background : brush.color;
            Vector2 canvas_pos = window_to_canvas(mouse_pos);            
            // Draw to overlay texture
            BeginTextureMode(overlay);
            if (!draw_pixel) {
                DrawCircleV(canvas_pos, brush.radius, color);
            } else {
                DrawPixel(canvas_pos.x, canvas_pos.y, color);
            }
            EndTextureMode();
        }
    }
}

void init_painting(int canvas_width, int canvas_height, Color background) {
    // ---Init Canvas---   
    canvas.width = canvas_width;
    canvas.height = canvas_height;
    // canvas area
    canvas.scale = (float)window.height/canvas.height;
    float canvas_window_width = canvas.width*canvas.scale;
    float canvas_window_height =  canvas.height*canvas.scale;
    window.canvas_area = (Rectangle){((window.width-window.l_border) - canvas_window_width)/2, 
                                     (window.height - canvas_window_height)/2,
                                     canvas_window_width, canvas_window_height};
    canvas.active_rect = (Rectangle){0, canvas.height, canvas.width, -canvas.height}; // Flip for OpenGl goofy
    canvas.background = background;
    canvas.rtexture = LoadRenderTexture(canvas.width, canvas.height);
    BeginTextureMode(canvas.rtexture);
    ClearBackground(canvas.background);
    EndTextureMode();

    // ---Init Brush---
    brush.radius = 0.01*canvas_height;
    brush.color = WHITE;
    brush.eraser = false;
    brush.drawing = false;
    current_tool = BRUSH;
}

int main(int argc, char **argv) {
    // ---Init Window---
    window = (Window){WINDOW_WIDTH, WINDOW_HEIGHT,L_BORDER};
    InitWindow(window.width, window.height, "Jpaint");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(500);
    init_painting(CANVAS_RES, CANVAS_RES, BLACK);
    SetTextureFilter(canvas.rtexture.texture, TEXTURE_FILTER_BILINEAR);
    
    // for canvas overlays
    RenderTexture overlay;
    overlay = LoadRenderTexture(canvas.width, canvas.height);
    
    double last_draw = 0;
    
    //---Main Loop---
    while (!WindowShouldClose()) {
        if (IsWindowResized()) {
            window.width = GetScreenWidth();
            window.height = GetScreenHeight();
        }

        float dt = GetFrameTime();
        handle_user_input(dt);

        if (current_tool == BRUSH || current_tool == BUCKET) { 
            paint_to_canvas();
        }

        // ---Drawing---
        BeginDrawing();
        ClearBackground(INTERFACE_COLOR);
        
        // Drawing the Canvas
        DrawTexturePro(canvas.rtexture.texture, canvas.active_rect,
                       window.canvas_area, Vector2Zero(), 0, WHITE); 
        
        // Overlay
        draw_to_overlay(overlay);
        DrawTexturePro(overlay.texture, canvas.active_rect,
                       window.canvas_area, Vector2Zero(), 0, WHITE);
        

        // UI
        handle_ui_events(&window, &canvas, &brush);
        EndDrawing();
    }

    return 0;
}
