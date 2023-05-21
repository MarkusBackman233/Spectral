#include "Spectral.h"

#define SCREEN_WIDTH 384
#define SCREEN_HEIGHT 216


#define MAP_SIZE 8
static Uint8 mapData[MAP_SIZE * MAP_SIZE] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 3, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 2, 0, 4, 4, 0, 1,
    1, 0, 0, 0, 4, 0, 0, 1,
    1, 0, 3, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
};



uint32_t Pixels[SCREEN_WIDTH * SCREEN_HEIGHT];





int main(int argc, char* args[])
{
    SDL_Event event;
    SDL_Renderer* renderer;
    SDL_Window* window;
    int i;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(SCREEN_WIDTH * 4, SCREEN_HEIGHT * 4, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_RenderSetIntegerScale(renderer, SDL_TRUE);

    SDL_Texture* renderTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);








    while (true) 
    {
        Render();

        SDL_UpdateTexture(renderTexture, NULL, Pixels, SCREEN_WIDTH * 4);
        SDL_RenderCopyEx(renderer, renderTexture, NULL, NULL, 0.0, NULL, SDL_FLIP_VERTICAL);

        SDL_RenderPresent(renderer);

        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}





void Render() 
{
    for (int x = 0; x < SCREEN_WIDTH; x++) 
    {


        const float xcam = (2 * (x / (float)(SCREEN_WIDTH))) - 1;

        // ray direction through this column
        const Vector2 dir = {
            dir.x + plane.x * xcam,
            dir.y + plane.y * xcam
        };







        verline(x, 20, SCREEN_HEIGHT - 20, 0xFFFF0000 | (x & 0xFF));
    }
}

void verline(int x, int y0, int y1, uint32_t color)
{
    for (int y = y0; y <= y1; y++)
    {
        Pixels[(y * SCREEN_WIDTH) + x] = color;
    }
}