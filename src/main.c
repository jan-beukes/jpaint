#include <stdio.h>
#include <raylib.h>
#include <raymath.h>

#define MAX(X, Y) (X) > (Y) ? (X) : (Y)

// Defaults
#define L_BORDER 50
#define WINDOW_WIDTH 800 + L_BORDER
#define WINDOW_HEIGHT 800
#define MIN_SCALE 0.4
#define CANVAS_RES 400

#define ZOOM_SPEED 5.0
#define BRUSH_RESIZE_SPEED 5

// UI ?
#define INTERFACE_COLOR DARKGRAY
#define TOOLBAR_COLOR (Color){40, 40, 40, 255}

typedef struct Window{
    int width;
    int height;
    int l_border;
    Rectangle canvas_area; // Area occupied by canvas
} Window;

typedef struct Canvas {
    RenderTexture rtexture;
    // Resolution (pixels)
    int width; 
    int height;
    float scale;
    Rectangle active_rect;
    Color background;
} Canvas;

typedef struct Brush {
    Color color;
    float radius;
    bool eraser; // in eraser mode
    bool drawing;
} Brush;

// Globals
Window window;
Canvas canvas;
Brush brush;

// Convert a window position to a canvas position
Vector2 window_to_canvas(Vector2 screen_pos) {
    
    // scale to canvas resolution based on position on the canvas area 
    Vector2 pos = {(screen_pos.x - window.canvas_area.x)/window.canvas_area.width*canvas.width,
                   (screen_pos.y - window.canvas_area.y)/window.canvas_area.height*canvas.height};
    return pos;
}

// Draw to the canvas Texture
void draw_to_canvas() {
    static bool was_on_canvas = false;
    static Vector2 prev_canvas_pos; // This feels sus
    Vector2 mouse_pos = GetMousePosition();

    // Clear Screen
    if (IsKeyPressed(KEY_C)) {
        BeginTextureMode(canvas.rtexture);
        ClearBackground(canvas.background);
        EndTextureMode();
        return;
    }
    // Check if mouse is on the canvas
    if (!CheckCollisionPointRec(mouse_pos, window.canvas_area)) {
        was_on_canvas = false;
        return;
    }
    Vector2 canvas_pos = window_to_canvas(mouse_pos);
    if (!was_on_canvas) prev_canvas_pos = canvas_pos; // initialize

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        brush.drawing = true;
        Color color = brush.eraser ? canvas.background : brush.color;
        BeginTextureMode(canvas.rtexture);
        // fill large displacement with circles
        float distance = Vector2Distance(canvas_pos, prev_canvas_pos);
        if (distance > brush.radius) {
            Vector2 line_dir = Vector2Normalize(Vector2Subtract(canvas_pos, prev_canvas_pos));

            int circles = (int)(2*distance/brush.radius) - 1; // Leave the last one for the default draw
            Vector2 pos = prev_canvas_pos;
            for (int i = 0; i < circles; i++) {
                pos = Vector2Add(pos, Vector2Scale(line_dir, brush.radius));
                DrawCircleV(pos, brush.radius, color);
            }
        }

        // At mouse pos
        DrawCircleV(canvas_pos, brush.radius, color);
        EndTextureMode();
    } else { 
        brush.drawing = false;
    }
    prev_canvas_pos = canvas_pos; // Set for next frame
    was_on_canvas = true;
}

// handle Keyboard/Mouse input events
void handle_user_input(float dt) {
    // Eraser
    if (IsKeyPressed(KEY_E)) {
        brush.eraser = !brush.eraser;
    }
    // Canvas Movement
    if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)) {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        Vector2 delta = GetMouseDelta();
        Vector2 canvas_area_pos = (Vector2){window.canvas_area.x, window.canvas_area.y};
        canvas_area_pos = Vector2Add(canvas_area_pos, delta);
        window.canvas_area.x = canvas_area_pos.x;
        window.canvas_area.y = canvas_area_pos.y;

    } else {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }
    // Zoom
    float mouse_scroll = GetMouseWheelMove();
    if (IsKeyDown(KEY_LEFT_CONTROL) && mouse_scroll != 0) {
        float zoom = mouse_scroll > 0 ? (1+ZOOM_SPEED*dt) : (1-ZOOM_SPEED*dt);
        canvas.scale = MAX(canvas.scale*zoom, MIN_SCALE);
        if (canvas.scale > MIN_SCALE) {
            Vector2 prev_mouse_canvas_pos = window_to_canvas(GetMousePosition());
            printf("canvas pos 1(%f, %f)\n", prev_mouse_canvas_pos.x, prev_mouse_canvas_pos.y);
            
            window.canvas_area.width = canvas.width*canvas.scale;
            window.canvas_area.height = canvas.height*canvas.scale;
            Vector2 mouse_canvas_pos = window_to_canvas(GetMousePosition());
            printf("canvas pos 2(%f, %f)\n", mouse_canvas_pos.x, mouse_canvas_pos.y);
            
            // Move canvas to keep mouse on the same position
            Vector2 displacement = Vector2Subtract(mouse_canvas_pos, prev_mouse_canvas_pos);
            window.canvas_area.x += displacement.x;
            window.canvas_area.y += displacement.y;
            
            mouse_canvas_pos = window_to_canvas(GetMousePosition());
            printf("canvas pos 3(%f, %f)\n\n", mouse_canvas_pos.x, mouse_canvas_pos.y);
        }
    } else if (mouse_scroll != 0) {
        brush.radius += mouse_scroll*BRUSH_RESIZE_SPEED*dt;
    }


}

void handle_ui() {
    // Toolbar
    DrawRectangle(0, 0, window.l_border, window.height, TOOLBAR_COLOR);
    DrawLine(window.l_border, 0, window.l_border, window.height, INTERFACE_COLOR);

    DrawText(TextFormat("%.0f%%", canvas.scale * 100), 5, window.height - 20, 18, WHITE);
}

int main() {
    // Init Window
    window = (Window){WINDOW_WIDTH, WINDOW_HEIGHT,L_BORDER};
    InitWindow(window.width, window.height, "JPaint");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(120);

    // Init canvas     
    canvas.width = CANVAS_RES;
    canvas.height = CANVAS_RES;
    // canvas area
    canvas.scale = window.height/canvas.height;
    window.canvas_area = (Rectangle){window.l_border, 0, canvas.width*canvas.scale, canvas.height*canvas.scale};

    canvas.active_rect = (Rectangle){0, canvas.height, canvas.width, -canvas.height}; // Flip for OpenGl goofy
    canvas.background = BLACK;
    canvas.rtexture = LoadRenderTexture(canvas.width, canvas.height);
    BeginTextureMode(canvas.rtexture);
    ClearBackground(canvas.background);
    EndTextureMode();


    // Init Brush
    brush.radius = 2.0;
    brush.color = WHITE;
    brush.eraser = false;
    brush.drawing = false;

    while (!WindowShouldClose()) {
        if (IsWindowResized()) {
            window.width = GetScreenWidth();
            window.height = GetScreenHeight();
        }
        
        float dt = GetFrameTime();
        handle_user_input(dt);

        BeginDrawing();
        ClearBackground(INTERFACE_COLOR);
        
        // Drawing Canvas
        draw_to_canvas();
        DrawTexturePro(canvas.rtexture.texture, canvas.active_rect,
                       window.canvas_area, Vector2Zero(), 0, WHITE); 
        
        // show brush when not drawing
        Vector2 mouse_pos = GetMousePosition();
        bool on_canvas = CheckCollisionPointRec(mouse_pos, window.canvas_area);
        if (!brush.drawing && on_canvas && !IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)) {
            Color color = brush.eraser ? canvas.background : brush.color;
            DrawCircleV(mouse_pos, brush.radius*canvas.scale, color);
        }


        // UI
        handle_ui();
        EndDrawing();
    }

    return 0;
}