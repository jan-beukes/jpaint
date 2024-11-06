#include <raylib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "images.h"

// Load packed image from dumped image.h with filename
Image load_packed_image(char *name) {
    
    if (strcmp(name,"bucket-fill.png") == 0)
        return LoadImageFromMemory(".png", bucket_fill_png, bucket_fill_png_len);
    
    else if (strcmp(name, "color-picker.png") == 0) 
        return LoadImageFromMemory(".png", color_picker_png, color_picker_png_len);
   
    else if (strcmp(name, "menu.png") == 0) 
        return LoadImageFromMemory(".png", menu_png, menu_png_len);
   
    else if (strcmp(name, "move-tool.png") == 0)
        return LoadImageFromMemory(".png", paintbrush_png, paintbrush_png_len);
    
    else if (strcmp(name, "paintbrush.png") == 0)
        return LoadImageFromMemory(".png", paintbrush_png, paintbrush_png_len);

    else if (strcmp(name, "transparent-bg.png") == 0)
        return LoadImageFromMemory(".png", transparent_bg_png, transparent_bg_png_len);
    
    else if (strcmp(name, "transparent.png") == 0)
        return LoadImageFromMemory(".png", transparent_png, transparent_png_len);
    
    printf("asset %s not found\n", name);
    exit(1);
}

Texture load_packed_texture(char *name) {
    Image img = load_packed_image(name);
    Texture ret = LoadTextureFromImage(img);
    UnloadImage(img);
    return ret;
}
