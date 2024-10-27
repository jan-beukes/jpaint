#define _POSIX_C_SOURCE 200809L

#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "paint.h"
#include "interface.h"

#define ZOOM_STEP 0.05

// Global
Window window;
Canvas canvas;
RenderTexture2D overlay;
Brush brush;
Tools current_tool;
char *current_file = NULL;

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

    // Check if on canvas and using drawing tool
    if (!CheckCollisionPointRec(mouse_pos, window.canvas_area) || current_tool != BRUSH) {
        was_on_canvas = false;
        return;
    }

    // Check if shift is held to set prev_canvas_pos to last draw_pos
    if (IsKeyDown(KEY_LEFT_SHIFT) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !brush.eraser){
        prev_canvas_pos = brush.prev_draw_pos;
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
        brush.prev_draw_pos = canvas_pos;
        EndTextureMode();
    } else { 
        brush.drawing = false;
    }
    prev_canvas_pos = canvas_pos; // Set for next frame
    was_on_canvas = true;
}

void draw_to_overlay() {
    // Clear Overlay texture 

    BeginTextureMode(overlay);
    ClearBackground((Color){0,0,0,0});
    EndTextureMode();
    
    // Brush
    Vector2 mouse_pos = GetMousePosition();
    bool on_canvas = CheckCollisionPointRec(mouse_pos, window.canvas_area);
    if (current_tool == BRUSH && !brush.drawing && on_canvas) {
        // Shift Line 
        if (IsKeyDown(KEY_LEFT_SHIFT) && !IsMouseButtonDown(MOUSE_MIDDLE_BUTTON) && !brush.eraser) {
            BeginTextureMode(overlay);
            float thick = brush.radius == 1.0 ? 1.0 : 2 * brush.radius;
            Vector2 canvas_pos = window_to_canvas(mouse_pos); 
            DrawLineEx(brush.prev_draw_pos, canvas_pos, thick, brush.color);
            if (thick != 1.0)
                DrawCircleV(window_to_canvas(mouse_pos), brush.radius, brush.color);
            else 
                DrawPixel(canvas_pos.x, canvas_pos.y, brush.color);
            EndTextureMode();
        // Brush hover
        } else if (!IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)) {
            bool draw_pixel = brush.radius == 1.0;
            Color color = brush.eraser ? canvas.background : brush.color;
            Vector2 canvas_pos = window_to_canvas(mouse_pos);            
            // Draw to overlay texture
            BeginTextureMode(overlay);
            if (!draw_pixel) {
                // Outline for eraser
                DrawCircleV(canvas_pos, brush.radius, color);
                if (brush.eraser)                
                    DrawCircleLinesV(canvas_pos, brush.radius, GRAY_SCALE(canvas.background) > 150.0 ? BLACK : WHITE);
            } else {
                DrawPixel(canvas_pos.x, canvas_pos.y, color);
            }
            EndTextureMode();
        }
    }
}

void paint_bucket_fill() {
    //
}

// fill region with paint bucket 
void flood_fill(int x, int y, Image *image, Color target, Color current) {
    
}

void export_canvas(char *filename) {
    if (current_file == NULL && filename == NULL){
        export_dialog();
        return;
    } else if (filename == NULL) {
        filename = current_file;
    } else {
        free(current_file);
        current_file = strdup(filename);        
    }
    Image export_image = LoadImageFromTexture(canvas.rtexture.texture);
    ImageFlipVertical(&export_image);
    ExportImage(export_image, filename);
    UnloadImage(export_image);
    printf("Image %s saved!\n", filename);
}

void load_canvas(char *filename) {
    Image image = LoadImage(filename);
    canvas.width = image.width;
    canvas.height = image.height;
    
    // canvas scaling and area
    canvas.scale = (float)window.height / canvas.height;
    float canvas_window_width = canvas.width * canvas.scale;
    float canvas_window_height = canvas.height * canvas.scale;
    window.canvas_area = (Rectangle){((window.width - window.l_border) - canvas_window_width) / 2, 
                                     (window.height - canvas_window_height) / 2,
                                     canvas_window_width, canvas_window_height};
    canvas.active_rect = (Rectangle){0, canvas.height, canvas.width, -canvas.height}; // Flip for Opengl goofy
    canvas.background = WHITE;

    // Free existing render texture if it exists
    if (canvas.rtexture.id > 0) UnloadRenderTexture(canvas.rtexture);
    canvas.rtexture = LoadRenderTexture(canvas.width, canvas.height);
    
    BeginTextureMode(canvas.rtexture);
    DrawTexture(LoadTextureFromImage(image), 0, 0, WHITE);  // Draw the image onto the render texture
    EndTextureMode();

    UnloadImage(image);
    UnloadRenderTexture(overlay);
    overlay = LoadRenderTexture(canvas.width, canvas.height);
}

// handle Keyboard/Mouse input events
void handle_user_input() {

    // Clear Canvas
    if (IsKeyPressed(KEY_C)) {
        BeginTextureMode(canvas.rtexture);
        ClearBackground(canvas.background);
        EndTextureMode();
    }
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
        if (current_file == NULL)
            export_dialog();
        else 
            export_canvas(NULL);
    }
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_O)) {
        import_dialog();
    }

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
        current_tool = MOVE;
    }
    if (IsKeyPressed(KEY_G)) {
        current_tool = BUCKET;
    }

    // Canvas Movement
    bool canvas_move = IsMouseButtonDown(MOUSE_MIDDLE_BUTTON) || 
                       (current_tool == MOVE && IsMouseButtonDown(MOUSE_BUTTON_LEFT));
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

Canvas init_canvas(int argc, char **argv) {

    if (argc > 0 && FileExists(argv[1]) && IsFileExtension(argv[1], ".png")) {
        load_canvas(argv[1]);
    } else {
        // call canvas GUI function 

        // ---Init Canvas---   
        canvas.width = CANVAS_RES;
        canvas.height = CANVAS_RES;
        // canvas area
        canvas.scale = (float)window.height/canvas.height;
        float canvas_window_width = canvas.width*canvas.scale;
        float canvas_window_height =  canvas.height*canvas.scale;
        window.canvas_area = (Rectangle){((window.width-window.l_border) - canvas_window_width)/2, 
                                        (window.height - canvas_window_height)/2,
                                        canvas_window_width, canvas_window_height};
        canvas.active_rect = (Rectangle){0, canvas.height, canvas.width, -canvas.height}; // Flip for OpenGl goofy
        canvas.background = WHITE;
        canvas.rtexture = LoadRenderTexture(canvas.width, canvas.height);
        BeginTextureMode(canvas.rtexture);
        ClearBackground(canvas.background);
        EndTextureMode();
    }

    // ---Init Brush---
    brush.radius = 0.01*canvas.height;
    brush.color = BLACK;
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
    SetExitKey(KEY_NULL);
    
    // Initialize application
    init_canvas(argc, argv);
    init_gui(&window);

    //SetTextureFilter(canvas.rtexture.texture, TEXTURE_FILTER_BILINEAR);
    
    // for canvas overlays
    overlay = LoadRenderTexture(canvas.width, canvas.height);
    
    double last_draw = 0;
    
    //---Main Loop---
    while (!WindowShouldClose()) {
        if (IsWindowResized()) {
                window.width = GetScreenWidth();
                window.height = GetScreenHeight(); 
        }

        if (!is_dialog_active())
            handle_user_input();
        paint_to_canvas();

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
        handle_ui(&window, &canvas, &brush, &current_tool);
        EndDrawing();
    }

    return 0;
}
