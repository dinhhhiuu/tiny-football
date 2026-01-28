#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Surface* gPNGSurface = NULL;

SDL_Surface* loadSurface(const char* path)
{
    SDL_Surface* loadedSurface = IMG_Load(path);
    if (loadedSurface == NULL)
    {
        printf("Unable to load image %s! SDL_image Error: %s\n",
               path, IMG_GetError());
        return NULL;
    }

    SDL_Surface* optimizedSurface =
        SDL_ConvertSurface(loadedSurface, gScreenSurface->format, 0);

    SDL_FreeSurface(loadedSurface);
    return optimizedSurface;
}

int main(int argc, char* args[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL error: %s\n", SDL_GetError());
        return 1;
    }

    // Init SDL_image (PNG)
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        printf("SDL_image error: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    gWindow = SDL_CreateWindow(
        "SDL - Image Formats",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    gScreenSurface = SDL_GetWindowSurface(gWindow);

    gPNGSurface = loadSurface("src/test.png");
    if (gPNGSurface == NULL)
        return 1;

    bool quit = false;
    SDL_Event e;

    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                quit = true;
        }

        SDL_BlitSurface(gPNGSurface, NULL, gScreenSurface, NULL);
        SDL_UpdateWindowSurface(gWindow);
    }

    SDL_FreeSurface(gPNGSurface);
    SDL_DestroyWindow(gWindow);

    IMG_Quit();
    SDL_Quit();

    return 0;
}
