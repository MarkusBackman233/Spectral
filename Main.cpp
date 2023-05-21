#include <SDL.h>
#include <stdio.h>
#include <iostream>
#include "Spectral.h"
#include "Player.h"
#include "PhysXManager.h"
#include <string>
int main(int argc, char* args[])
{
    SDL_Event event;
    SDL_Renderer* renderer = nullptr;
    SDL_Window* window = nullptr;

    bool keepRunning = true;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2, 0, &window, &renderer);
    //SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_RenderSetIntegerScale(renderer, SDL_TRUE);

    SDL_SetWindowTitle( window,"Spectral");
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_Texture* renderTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    auto spectral = new Spectral(renderer, window, event);
    Player* player = spectral->GetPlayer();
    //static uint32_t zBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
    while (keepRunning)
    {
        Uint32 frameStart = SDL_GetTicks();

        spectral->Update();

        //SDL_UpdateTexture(renderTexture, NULL, spectral->m_pixels, SCREEN_WIDTH * 4);




        /*for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
        {
            uint32_t color = spectral->m_depthBuffer[i] * 65536 ;
            uint8_t red = (color >> 24) & 0xFF;
            uint8_t green = (color >> 16) & 0xFF;
            uint8_t blue = (color >> 8) & 0xFF;
            uint8_t alpha = 255 & 0xFF;


            color = (blue << 24) | (blue << 16) | (blue << 8) | alpha;

            zBuffer[i] = color;
        }
        */

        SDL_UpdateTexture(renderTexture, NULL, spectral->m_albedoBuffer, SCREEN_WIDTH * 4);

        SDL_RenderCopyEx(renderer, renderTexture, NULL, NULL, 0.0, NULL, SDL_FLIP_VERTICAL);
        SDL_RenderPresent(renderer);



        static float yaw = 0.0f;
        static float pitch = 0.0f;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                keepRunning = false;
            }
            else if (event.type == SDL_MOUSEMOTION) {
                int dx = event.motion.xrel;
                int dy = event.motion.yrel;
                player->HandleMouseMovement(Math::Vector2i(dx, dy));
            }
        }
        if (SDL_GetTicks() - frameStart > 0)
        {
            Uint32 totalTime = 1000 / (SDL_GetTicks() - frameStart);

            //std::cout << "FPS: " << totalTime  << std::endl;
            SDL_SetWindowTitle(window, std::string("Spectral | FPS: " + std::to_string(totalTime)).c_str());
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}