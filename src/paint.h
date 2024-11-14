#ifndef PAINT_H
#define PAINT_H
#include <raylib.h>

/* 
We do a little bit of painting
*/

#define MAX(X, Y) (X) > (Y) ? (X) : (Y)
#define MIN(X, Y) (X) < (Y) ? (X) : (Y)

#define GRAY_SCALE(C) (C.r + C.b + C.b)/3.0

// Defaults
#define L_BORDER 60
#define WINDOW_WIDTH 1220 + L_BORDER
#define WINDOW_HEIGHT 720
#define MIN_SCALE 0.10
#define CANVAS_RES 512

#define BRUSH_RESIZE_STEP 0.1

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
    Vector2 prev_draw_pos;
} Brush;

typedef enum Tools {
    BRUSH = 0,       // Brush and Eraser
    BUCKET,
    MOVE,
    COLOR_PICKER,
    
    NONE,
} Tools;

// Convert a window position to a canvas position
Vector2 window_to_canvas(Vector2 screen_pos);
void export_canvas(char *filename);
void load_canvas(char *filename);
void init_canvas(int width, int height, Color background);

#endif
