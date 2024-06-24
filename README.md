# C9 gui

C9 gui is a boilerplate for creating a simple gui in C9 using SDL2. Note that SDL2 needs to be installed on your system for this to work.

## Prerequisites
SDL2, SDL2_ttf and SDL2_image libraries are required to compile and run this project.

### MacOS
Download sdl2, sdl2_ttf and sdl2_image from official releases and move them to `/Library/Frameworks`
https://stackoverflow.com/questions/60202947/vscode-intellisense-not-recognising-sdl-image-extension-library-for-sdl-framewor

## Compiling and running

### Clang
Compiling with SDL2
`clang -std=c99 -Wall -Wextra -F /Library/Frameworks -framework SDL2 main.c -o main`

Compiling with SDL2, SDL2_ttf and SDL2_image:
`clang -std=c99 -Wall -Wextra  -F /Library/Frameworks -framework SDL2 -framework SDL2_ttf -framework SDL2_image main.c -o main`

`clang $(sdl2-config --cflags --libs) -lSDL2_ttf -lSDL2_image -o main main.c`

## Usage

### Text rendering:
Foreground and background:
`TTF_RenderUTF8_Shaded(Inter, label_text, BLACK, WHITE);`

Foreground and transparent:
`TTF_RenderUTF8_Blended(Inter, label_text, BLACK);`

### Image rendering
Render image to surface and then to texture:

```C
  SDL_Surface *icon_application_surface =
      SDL_LoadBMP("icons/application-x-executable.bmp");
  SDL_Texture *icon_application_texture =
      SDL_CreateTextureFromSurface(renderer, icon_application_surface);
```

Render image directly to texture:
`SDL_Texture *icon_application_texture =  IMG_LoadTexture(renderer,"icons/application-x-executable.bmp");`
