#include <stdlib.h>
#include <stdbool.h>

#include <SDL3/SDL.h>
#include <emscripten.h>

#include "../compiler/dmx-interpreter.h"

#define FPS 60
#define FPS_DELAY 1000 / FPS

#define DELAY 1000

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
    SDL_Thread *timer_thread;
    VisualizerColor color;
    int w,h;
    SDL_AtomicInt timer;
    char timer_text[256];
    DMX_File *file;
    bool ready;
    bool running;
} VisualizerAppData;

typedef struct VisualizerThread
{
    VisualizerAppData *visualizer;
    SDL_AtomicInt running;
}VisualizerThread;

/* void clear_console(void) {
#if defined(_WIN32) || defined(_WIN64)
    system("cls");  // Windows
#else
    printf("\033[2J\033[H");
    fflush(stdout);
#endif
} */

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

void change_color(VisualizerAppData *visualizer)
{
    if(visualizer->color == VISUALIZER_WHITE)
        visualizer->color = VISUALIZER_RED;
    else
        visualizer->color++;
}

void interruptible_delay(VisualizerAppData *visualizer, int ms, bool countdown)
{
    while(ms > 0 && visualizer->running)
    {
        if(countdown)
            SDL_SetAtomicInt(&visualizer->timer, ms);
        int chunk = ms > 50 ? 50 : ms;
        SDL_Delay((Uint32)chunk);
        ms -= chunk;
    }
    if(countdown)
        SDL_SetAtomicInt(&visualizer->timer, 0);
}

int timer(void* data){
    VisualizerThread *td = (VisualizerThread *)data;
    VisualizerAppData *visualizer = td->visualizer;
    while(SDL_GetAtomicInt(&td->running)){
        interruptible_delay(visualizer, DELAY, true);
        change_color(visualizer);
    }
    return 0;
}

int *file_dialog_handler(void *userdata, const char * const *filelist, int filter)
{
    SDL_Log("File Dialog Handled(NOT)");
    return 0;
}

VisualizerAppData visualizer;
VisualizerThread timer_td;

void mainloop()
{
    if(visualizer.running)
    {
        Uint32 frame_start = SDL_GetTicks();
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
                case SDL_EVENT_WINDOW_RESIZED:
                    SDL_GetWindowSize(visualizer.window, &visualizer.w, &visualizer.h);
                    break;
            }
        };
        render_color(&visualizer);
        SDL_RenderClear(visualizer.renderer);
        // clear_console();
        SDL_Log("Visualizer Color: %i\nDelta: %i", visualizer.color, SDL_GetAtomicInt(&visualizer.timer));
        // SDL_Log("Timer: %i", visualizer.timer);
        sprintf(visualizer.timer_text, "Delta: %i", SDL_GetAtomicInt(&visualizer.timer));
        int remaining_ms = SDL_GetAtomicInt(&visualizer.timer);

        switch(visualizer.color)
        {
            default :
                SDL_SetRenderDrawColor(visualizer.renderer, 255, 255, 255, 255);
                break;
            case VISUALIZER_AMBER:
            case VISUALIZER_WHITE:
                SDL_SetRenderDrawColor(visualizer.renderer, 0, 0, 0, 255);
                break;
        }
        SDL_RenderDebugText(visualizer.renderer, (float)((visualizer.w - (SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE *strlen(visualizer.timer_text))) / 2), (float)(visualizer.h / 2), visualizer.timer_text);
        SDL_RenderPresent(visualizer.renderer);
        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if(frame_time < FPS_DELAY)
            SDL_Delay(FPS_DELAY - frame_time);
    } else {
        SDL_DestroyWindow(visualizer.window);
        SDL_DestroyRenderer(visualizer.renderer);
        SDL_Quit();
    }
}

int main(){
    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        SDL_Log("Failed to Load SDL3: %s", SDL_GetError());

    }
    if(!SDL_CreateWindowAndRenderer("DMX Lang Visualizer", 600, 600, SDL_WINDOW_RESIZABLE, &visualizer.window, &visualizer.renderer))
    {
        SDL_Log("Failed to Create Window or Renderer: %s", SDL_GetError());
        SDL_Quit();

    }
    SDL_SetAtomicInt(&visualizer.timer, DELAY); // 1 second
    visualizer.color = VISUALIZER_RED;
    timer_td.visualizer = &visualizer;
    SDL_SetAtomicInt(&timer_td.running, 1);
    visualizer.timer_thread = SDL_CreateThread(timer, "Timer Thread", &timer_td);
    SDL_GetWindowSize(visualizer.window, &visualizer.w, &visualizer.h);
    // SDL_ShowOpenFileDialog(file_dialog_handler, NULL, visualizer.window, NULL, NULL, NULL, false);
    visualizer.ready = true;
    visualizer.running = true;
    emscripten_set_main_loop(mainloop, 0, 1);
}