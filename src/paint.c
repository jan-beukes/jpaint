
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "glad.h"
#include "paint.h"
#include "interface.h"
#include "p_stack.h"

#define ZOOM_STEP 0.05

// Global
Window window;
Canvas canvas;
RenderTexture2D overlay, output;
Brush brush;
Tools current_tool;
char *current_file = NULL;

bool is_same_color(Color col1, Color col2)
{
    bool result = true;
    int delta = 4;

    if (col1.r < col2.r - delta || col1.r > col2.r + delta) result = false;
    if (col1.g < col2.g - delta || col1.g > col2.g + delta) result = false;
    if (col1.b < col2.b - delta || col1.b > col2.b + delta) result = false;
    if (col1.a < col2.a - delta || col1.a > col2.a + delta) result = false;

    return result;
}

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
    if (IsKeyDown(KEY_LEFT_SHIFT) && !IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)
        && !brush.eraser && !(brush.prev_draw_pos.x == 0 && brush.prev_draw_pos.y == 0)){
        prev_canvas_pos = brush.prev_draw_pos;
    }
    
    Vector2 canvas_pos = window_to_canvas(mouse_pos);
    if (!was_on_canvas) prev_canvas_pos = canvas_pos; // initialize
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        brush.drawing = true;
        bool draw_pixel = brush.radius == 1.0;
        Color color = brush.eraser ? WHITE : brush.color;
        
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
            if (brush.eraser) {
                // Magic
                BeginBlendMode(BLEND_CUSTOM); glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA); 
                DrawCircleV(canvas_pos, brush.radius, color);
                EndBlendMode(); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            } else {
                DrawCircleV(canvas_pos, brush.radius, color);
            }
            
        } else {
            DrawPixel(canvas_pos.x, canvas_pos.y, color);
        }

        if(!brush.eraser) brush.prev_draw_pos = canvas_pos;
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
    ClearBackground(BLANK);
    EndTextureMode();
    
    // Brush
    Vector2 mouse_pos = GetMousePosition();
    bool on_canvas = CheckCollisionPointRec(mouse_pos, window.canvas_area);
    if (current_tool == BRUSH && !brush.drawing && on_canvas) {
        // Shift Line 
        if (IsKeyDown(KEY_LEFT_SHIFT) && !IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)
            && !brush.eraser && !(brush.prev_draw_pos.x == 0 && brush.prev_draw_pos.y == 0)) {
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
        #define ERASER_OUTLINE_THRESH 15.0
        } else if (!IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)) {
            bool draw_pixel = brush.radius == 1.0;
            Color color = brush.eraser ? WHITE : brush.color;
            Vector2 canvas_pos = window_to_canvas(mouse_pos);            
            
            // Draw to overlay texture
            BeginTextureMode(overlay);
            if (!draw_pixel) {
                // Outline for eraser
                DrawCircleV(canvas_pos, brush.radius, color);
                if (brush.eraser && brush.radius > ERASER_OUTLINE_THRESH)
                    DrawCircleLinesV(canvas_pos, brush.radius, BLANK);
            } else {
                DrawPixel(canvas_pos.x, canvas_pos.y, color);
            }
            EndTextureMode();
        }
    }
}

// fill region with paint bucket 
void flood_fill(int x, int y, Image *image, Color target, Color source) {
    PStack stack = {0};
    PVec2 pos = {x, y};
    pstack_push(&stack, pos);

    while (!pstack_empty(stack)) {
        pos = pstack_pop(&stack);
        // Bounds
        if ((pos.x >= 0 && pos.x < image->width) && (pos.y >= 0 && pos.y < image->height)) {

            Color current = GetImageColor(*image, pos.x, pos.y);
            if (!is_same_color(current, target) && is_same_color(current, source)) {
                ImageDrawPixel(image, pos.x, pos.y, target);
                
                pstack_push(&stack, (PVec2){pos.x + 1, pos.y});
                pstack_push(&stack, (PVec2){pos.x - 1, pos.y});
                pstack_push(&stack, (PVec2){pos.x, pos.y + 1});
                pstack_push(&stack, (PVec2){pos.x, pos.y - 1});
            }
        }
    }
  
}

void paint_bucket_fill() {
    // Not on canvas
    if (!CheckCollisionPointRec(GetMousePosition(), window.canvas_area)) return; 

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 pos = window_to_canvas(GetMousePosition());

        // Image to read pixel colors
        Image canvas_image = LoadImageFromTexture(canvas.rtexture.texture);

        pos.y = canvas_image.height - pos.y; // Texture coordinates flipped OpenGL and dat 

        Color source = GetImageColor(canvas_image, (int)pos.x, (int)pos.y);
        
        // Skip if already correct color
        if (!is_same_color(source, brush.color)) {
            flood_fill((int)pos.x, (int)pos.y, &canvas_image, brush.color, source);
            UpdateTexture(canvas.rtexture.texture, canvas_image.data);
        }

        UnloadImage(canvas_image);
    }

}

void color_picker() {
    // Not on canvas
    if (!CheckCollisionPointRec(GetMousePosition(), window.canvas_area)) return; 

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Image canvas_image = LoadImageFromTexture(canvas.rtexture.texture);
        Vector2 pos = window_to_canvas(GetMousePosition());
        pos.y = canvas_image.height - pos.y; // Texture coordinates flipped OpenGL and dat!!!

        Color color = GetImageColor(canvas_image, (int)pos.x, (int)pos.y);
        
        // Sussy Case for non transparent background
        if (color.a == 0 && canvas.background.a != 0) color = canvas.background; 

        brush.color = color;
        current_tool = BRUSH;

        UnloadImage(canvas_image);
    }

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

    // Clear to background for export
    RenderTexture temp = LoadRenderTexture(canvas.width, canvas.height);
    BeginTextureMode(temp);
    ClearBackground(canvas.background);
    DrawTexture(canvas.rtexture.texture, 0, 0, WHITE);
    EndTextureMode();
    
    Image export_image = LoadImageFromTexture(temp.texture);
    ExportImage(export_image, filename);

    UnloadImage(export_image);
    UnloadRenderTexture(temp);
    printf("Image %s saved!\n", filename);
}

void load_canvas(char *filename) {
    Image image = LoadImage(filename);
    
    // jpeg convert
    if (image.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8) {
        ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    }

    canvas.width = image.width;
    canvas.height = image.height;
    if (IsFileExtension(filename, ".jpg")) { 
        canvas.background = WHITE;
    } else {
        canvas.background = BLANK;
    }

    // canvas scaling and area
    canvas.scale = (float)window.height / canvas.height;
    float canvas_window_width = canvas.width * canvas.scale;
    float canvas_window_height = canvas.height * canvas.scale;
    window.canvas_area = (Rectangle){((window.width - window.l_border) - canvas_window_width) / 2, 
                                     (window.height - canvas_window_height) / 2,
                                     canvas_window_width, canvas_window_height};
    canvas.active_rect = (Rectangle){0, 0, canvas.width, -canvas.height}; // Flip for Opengl goofy
    
    if (canvas.rtexture.id != 0) UnloadRenderTexture(canvas.rtexture);
    if (overlay.id != 0) UnloadRenderTexture(overlay);
    if (output.id != 0) UnloadRenderTexture(output);
    canvas.rtexture = LoadRenderTexture(canvas.width, canvas.height);
    overlay = LoadRenderTexture(canvas.width, canvas.height);
    output = LoadRenderTexture(canvas.width, canvas.height);
    
    BeginTextureMode(canvas.rtexture);
    ClearBackground(BLANK);
    Texture temp = LoadTextureFromImage(image);
    DrawTexture(temp, 0, 0, WHITE);  // Draw the image onto the render texture
    EndTextureMode();

    // Reset brush
    brush.radius = 0.01*canvas.height;
    brush.color = BLACK;
    brush.eraser = false;
    brush.drawing = false;
    current_tool = BRUSH;

    UnloadTexture(temp);
    UnloadImage(image);
}

void init_canvas(int width, int height, Color background) {
    // ---Init Canvas--- 
    canvas.width = width;
    canvas.height = height;
    canvas.background = background;
    // canvas area
    canvas.scale = (float)window.height/canvas.height;
    float canvas_window_width = canvas.width*canvas.scale;
    float canvas_window_height =  canvas.height*canvas.scale;
    window.canvas_area = (Rectangle){((window.width-window.l_border) - canvas_window_width)/2, 
                                    (window.height - canvas_window_height)/2,
                                    canvas_window_width, canvas_window_height};
    canvas.active_rect = (Rectangle){0, 0, canvas.width, -canvas.height}; // Flip for OpenGl goofy
    
    if (canvas.rtexture.id != 0) UnloadRenderTexture(canvas.rtexture);
    if (overlay.id != 0) UnloadRenderTexture(overlay);
    if (output.id != 0) UnloadRenderTexture(output);
    canvas.rtexture = LoadRenderTexture(canvas.width, canvas.height);
    overlay = LoadRenderTexture(canvas.width, canvas.height);
    output = LoadRenderTexture(canvas.width, canvas.height);
    
    BeginTextureMode(canvas.rtexture);
    ClearBackground(BLANK);
    EndTextureMode();
}

// handle Keyboard/Mouse input events
void handle_user_input() {

    // Clear Canvas
    if (IsKeyPressed(KEY_C)) {
        BeginTextureMode(canvas.rtexture);
        ClearBackground(BLANK);
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
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_N)) {
        enable_create_canvas_gui();
    }

    // ---TOOLS---
    if (IsKeyPressed(KEY_E)) {
        current_tool = BRUSH;
        brush.eraser = true;
        switch_brush_texture(true);
    }
    if (IsKeyPressed(KEY_B)) {
        current_tool = BRUSH;
        brush.eraser = false;
        switch_brush_texture(false);
        brush.prev_draw_pos = Vector2Zero(); // reset when switching from eraser
    }
    if (IsKeyPressed(KEY_H)) {
        current_tool = MOVE;
    }
    if (IsKeyPressed(KEY_G)) {
        current_tool = BUCKET;
    }
    if (IsKeyPressed(KEY_LEFT_ALT)) {
        current_tool = COLOR_PICKER;
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
    if ((IsKeyDown(KEY_LEFT_CONTROL) && mouse_scroll != 0) || zoom_key) {
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

int main(int argc, char **argv) {
    // ---Init Window---
    window = (Window){0};
    window.width = WINDOW_WIDTH; window.height = WINDOW_HEIGHT;
    window.l_border = L_BORDER;
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(window.width, window.height, "Jpaint");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(500);
    //SetExitKey(KEY_NULL);
    //SetTextureFilter(canvas.rtexture.texture, TEXTURE_FILTER_BILINEAR);
    
    // ---Initialize application---
    if (argc > 0 && FileExists(argv[1]) && ((IsFileExtension(argv[1], ".png;.jpg")))) {
        load_canvas(argv[1]);
    } else {
        init_canvas(CANVAS_RES, CANVAS_RES, WHITE);
    }
    brush.radius = 0.01*canvas.height;
    brush.color = BLACK;
    brush.eraser = false;
    brush.drawing = false;
    current_tool = BRUSH;
    init_gui(&window);
        
    //---Main Loop---
    while (!WindowShouldClose()) {
        if (IsWindowResized()) {
            window.width = GetScreenWidth();
            window.height = GetScreenHeight(); 
        }

        if (!is_dialog_active())
            handle_user_input();

        // ---Drawing---
        BeginDrawing();
        ClearBackground(INTERFACE_COLOR);
        
        if (canvas.background.a == 0)
            DrawTexturePro(transparent_bg_texture, 
                          (Rectangle){0, 0, transparent_bg_texture.width, transparent_bg_texture.width},
                          window.canvas_area, Vector2Zero(), 0, WHITE);
        DrawRectangleRec(window.canvas_area, canvas.background);

        //---Canvas---
        paint_to_canvas();        
        if (current_tool == BUCKET) paint_bucket_fill();
        if (current_tool== COLOR_PICKER) color_picker();
        draw_to_overlay();

        BeginTextureMode(output);
            ClearBackground(BLANK);
            DrawTextureRec(canvas.rtexture.texture, canvas.active_rect, Vector2Zero(), WHITE);
            
            // set alpha based on drawing when on eraser
            if (brush.eraser) {
                BeginBlendMode(BLEND_CUSTOM); glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA); 
                DrawTextureRec(overlay.texture, canvas.active_rect, Vector2Zero(), WHITE);
                EndBlendMode(); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                //Outline
                if (brush.radius > ERASER_OUTLINE_THRESH && current_tool == BRUSH &&
                    CheckCollisionPointRec(GetMousePosition(), window.canvas_area))
                    DrawCircleLinesV(window_to_canvas(GetMousePosition()), brush.radius,
                                    canvas.background.r > 150 ? DARKGRAY : RAYWHITE);
            } else {
                DrawTextureRec(overlay.texture, canvas.active_rect, Vector2Zero(), WHITE);
            }
                    
        EndTextureMode();
        DrawTexturePro(output.texture, canvas.active_rect, window.canvas_area, Vector2Zero(), 0, WHITE);

        // UI
        handle_ui(&window, &canvas, &brush, &current_tool);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
