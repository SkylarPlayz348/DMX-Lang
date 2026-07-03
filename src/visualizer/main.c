#include <stdlib.h>
#include <stdbool.h>

#include <SDL3/SDL.h>

#include "../compiler/dmx-interpreter.h"

typedef enum VisualizerColor
{
    VISUALIZER_RED=0,
    VISUALIZER_GREEN,
    VISUALIZER_BLUE,
    VISUALIZER_AMBER,
    VISUALIZER_WHITE
} VisualizerColor;

typedef struct VisualizerAppData
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;
    SDL_SystemTheme theme;
    SDL_Thread *color_thread;
    VisualizerColor color;
    int delta;
    DMX_File *file;
    bool running;
} VisualizerAppData;

typedef struct VisualizerThread
{
    VisualizerAppData *visualizer;
    SDL_AtomicInt running;
}VisualizerThread;

void clear_console(void) {
#if defined(_WIN32) || defined(_WIN64)
    system("cls");  // Windows
#else
    printf("\033[2J\033[H");
    fflush(stdout);
#endif
}

/* Future Proofing when I actually get assets */
void change_theme(VisualizerAppData visualizer)
{
    switch(visualizer.theme)
    {
        case SDL_SYSTEM_THEME_LIGHT:
            SDL_Log("System Theme Changed to Light Theme");
            break;
        case SDL_SYSTEM_THEME_DARK:
            SDL_Log("System Theme Changed to Dark Theme");
            break;
        case SDL_SYSTEM_THEME_UNKNOWN:
            SDL_Log("System Theme Changed to Unknown Theme Forcing Dark Mode");
            break;
    }
}

void render_color(VisualizerAppData* visualizer)
{
    switch(visualizer->color)
    {
        case VISUALIZER_RED:
            SDL_SetRenderDrawColor(visualizer->renderer, 255, 0, 0, 255);
            break;
        case VISUALIZER_GREEN:
            SDL_SetRenderDrawColor(visualizer->renderer, 0, 255, 0, 255);
            break;
        case VISUALIZER_BLUE:
            SDL_SetRenderDrawColor(visualizer->renderer, 0, 0, 255, 255);
            break;
        case VISUALIZER_AMBER:
            SDL_SetRenderDrawColor(visualizer->renderer, 255, 155+75, 0, 255);
            break;
        case VISUALIZER_WHITE:
            SDL_SetRenderDrawColor(visualizer->renderer, 255, 255, 255, 255);
            break;

    }
}

int change_color(void *data)
{
    VisualizerThread *td = (VisualizerThread *)data;
    VisualizerAppData *visualizer = td->visualizer;

    while(SDL_GetAtomicInt(&td->running)){
        switch(visualizer->color){
            case VISUALIZER_WHITE:
                visualizer->color = VISUALIZER_RED;
                break;
            default:
                visualizer->color++;
                break;
        }
        if(visualizer->color > 5){
            visualizer->color = VISUALIZER_RED;
        }
        render_color(visualizer);
        SDL_Delay(visualizer->delta);
    }
    return 0;
}

int main()
{
    VisualizerAppData visualizer;
    VisualizerThread color_td;
    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        SDL_Log("Failed to Load SDL3: %s", SDL_GetError());
        return -1;
    }
    if(!SDL_CreateWindowAndRenderer("DMX Lang Visualizer", 600, 600, SDL_WINDOW_RESIZABLE, &visualizer.window, &visualizer.renderer))
    {
        SDL_Log("Failed to Create Window or Renderer: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    visualizer.delta = 1000; // 1 second
    visualizer.running = true;
    color_td.visualizer = &visualizer;
    SDL_SetAtomicInt(&color_td.running, 1);
    visualizer.color_thread = SDL_CreateThread(change_color, "Color Change Thread", &color_td);
    while(visualizer.running)
    {
        while(SDL_PollEvent(&visualizer.event))
        {
            switch(visualizer.event.type)
            {
                case SDL_EVENT_QUIT:
                    visualizer.running = false;
                    break;
                case SDL_EVENT_SYSTEM_THEME_CHANGED:
                    visualizer.theme = SDL_GetSystemTheme();
                    break;
                case SDL_EVENT_KEY_DOWN:
                    switch(visualizer.event.key.key)
                    {
                        case SDLK_UP:
                            visualizer.delta++;
                            break;
                        case SDLK_DOWN:
                            if(visualizer.delta != 1)
                                visualizer.delta--;
                            break;
                    }
                    break;
            }
        };
        SDL_RenderClear(visualizer.renderer);
        SDL_RenderPresent(visualizer.renderer);
        clear_console();
        SDL_Log("Visualizer Color: %i\nDelta: %i", visualizer.color, visualizer.delta);
    };

    SDL_DestroyWindow(visualizer.window);
    SDL_DestroyRenderer(visualizer.renderer);
    SDL_Quit();
    return 0;
}