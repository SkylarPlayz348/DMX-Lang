#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "../compiler/dmx-interpreter.h"
#include "../version.h"

#define FPS 60
#define FPS_DELAY 1000 / FPS

#ifdef _WIN32
    #define strcasecmp _stricmp
#endif

typedef enum VisualizerColor
{
    VISUALIZER_BLACKOUT=0,
    VISUALIZER_RED,
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
    SDL_Thread *playback_thread;
    SDL_AtomicInt running;
    VisualizerColor color;
    SDL_AtomicInt timer_ms;
    char timer_text[256];
    char version_text[256];
    int w,h;
    SDL_AtomicInt ready;
    bool blackout;

    DMX_File dmx;
    bool dmx_opened;
    DMXD_File dmxd;
    bool dmxd_opened;
    char preloaded_warning[1024];
} VisualizerAppData;

static const SDL_DialogFileFilter filters[] = {
    { "DMX-Lang", "dmx;dmxd" },
};

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
        case VISUALIZER_BLACKOUT:
            SDL_SetRenderDrawColor(visualizer->renderer, 0, 0, 0, 255);
            break;
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
    if(visualizer->color == VISUALIZER_WHITE || visualizer->color == VISUALIZER_BLACKOUT)
        visualizer->color = VISUALIZER_RED;
    else
        visualizer->color++;
}

void interruptible_delay(VisualizerAppData *visualizer, int ms, bool countdown)
{
    while(ms > 0 && SDL_GetAtomicInt(&visualizer->running))
    {
        if(countdown)
            SDL_SetAtomicInt(&visualizer->timer_ms, ms);
        int chunk = ms > 50 ? 50 : ms;
        SDL_Delay((Uint32)chunk);
        ms -= chunk;
    }
    if(countdown)
        SDL_SetAtomicInt(&visualizer->timer_ms, 0);
}

int playback(void *data)
{
    VisualizerAppData *visualizer = (VisualizerAppData *) data;
    while(SDL_GetAtomicInt(&visualizer->running))
    {
        if(!SDL_GetAtomicInt(&visualizer->ready))
        {
            SDL_Delay(10);
            continue;
        }
        if(visualizer->dmx.instruction_count == 0)
        {
            SDL_Delay(10);
            continue;
        }

        for(int i = 0; i < visualizer->dmx.instruction_count && SDL_GetAtomicInt(&visualizer->running); i++)
        {
            const DMX_Instruction *instr = &visualizer->dmx.instructions[i];

            if(instr->kind == INSTR_DELAY)
            {
                interruptible_delay(visualizer, instr->seconds * 1000, true);
                continue;
            }

            bool is_blackout = strcmp(instr->command, "blackout") == 0;
            if(is_blackout)
            {
                if((visualizer->blackout = !visualizer->blackout))
                    visualizer->color = VISUALIZER_BLACKOUT;
                change_color(visualizer);
                continue;
            }

            change_color(visualizer);
        }
    }
    return 0;
}

int timer(void* data){
    // Trying to compile before breaking stuff with actual playback thread
    return 0;
}

void SDLCALL file_dialog_handler(void *userdata, const char * const *files, int filter)
{
    VisualizerAppData *visualizer = (VisualizerAppData *)userdata;
    if(files == NULL)
    {
        return;
    }
    if(*files == NULL)
    {
        return;
    }
    while (*files) {
        const char *dot = strrchr(*files, '.');
        if(!dot)
        {
            files++;
            continue;
        }
        const char*ext = dot+1;

        SDL_Log("Read: %s", *files);
        if(strcasecmp(ext, "dmx") == 0)
        {
            snprintf(visualizer->preloaded_warning, sizeof(visualizer->preloaded_warning), "Multiple DMX Files Read. %s Ignored.", *files);
            if(!visualizer->dmx_opened)
            {
                visualizer->dmx.handler = fopen(*files,"r");
                if(!visualizer->dmx.handler)
                {
                    SDL_Log("Error: Failed to Open DMX file: %s", *files);
                    files++;
                    continue;
                }
                visualizer->dmx_opened = true;
            }
            else
            {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "DMX-Lang Visualizer", visualizer->preloaded_warning, visualizer->window);
            }
            files++;
            continue;
        }

        if(strcasecmp(ext, "dmxd") == 0)
        {
            snprintf(visualizer->preloaded_warning, sizeof(visualizer->preloaded_warning), "Multiple DMXD Files Read. %s Ignored.", *files);
            if(!visualizer->dmxd_opened)
            {
                visualizer->dmxd.handler = fopen(*files,"r");
                if(!visualizer->dmxd.handler)
                {
                    SDL_Log("Error: Failed to Open DMXD file: %s", *files);
                    files++;
                    continue;
                }
                visualizer->dmxd_opened = true;
            }
            else
            {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "DMX-Lang Visualizer", visualizer->preloaded_warning, visualizer->window);
            }
            files++;
            continue;
        }

        files++;
    }

    if(!visualizer->dmxd.handler)
    {
        SDL_Log("Error: Failed to verify DMXD file");
        SDL_SetAtomicInt(&visualizer->running, 0);
        return;
    }

    if(!visualizer->dmx.handler)
    {
        SDL_Log("Error: Failed to verify DMX file");
        SDL_SetAtomicInt(&visualizer->running, 0);
        return;
    }

    if(!visualizer_load_sequence(&visualizer->dmx, &visualizer->dmxd))
    {
        SDL_Log("Error: Failed to Decode Sequence");
        SDL_SetAtomicInt(&visualizer->running, 0);
        return;
    }
    SDL_SetAtomicInt(&visualizer->ready, 1);
}

int main()
{
    VisualizerAppData visualizer = {0};
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
    visualizer.playback_thread = SDL_CreateThread(playback, "Playback Thread", &visualizer);
    SDL_GetWindowSize(visualizer.window, &visualizer.w, &visualizer.h);
    const int debug_charsize = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;
    sprintf(visualizer.version_text, "Compiled with Version: v%i.%i.%i", DMXLANG_VERSION_MAJOR, DMXLANG_VERSION_MINOR, DMXLANG_VERSION_PATCH);
    SDL_ShowOpenFileDialog(&file_dialog_handler, &visualizer, visualizer.window, filters, SDL_arraysize(filters), NULL, true); // pass visualizer data so we can read and wite to the dmx and dmxd members
    SDL_SetAtomicInt(&visualizer.running, 1);
    while(SDL_GetAtomicInt(&visualizer.running) == 1)
    {
        Uint32 frame_start = SDL_GetTicks();
        while(SDL_PollEvent(&visualizer.event))
        {
            switch(visualizer.event.type)
            {
                case SDL_EVENT_QUIT:
                    SDL_SetAtomicInt(&visualizer.running, 0);
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
        sprintf(visualizer.timer_text, "Delta: %i", SDL_GetAtomicInt(&visualizer.timer_ms) / 1000);
        int remaining_ms = SDL_GetAtomicInt(&visualizer.timer_ms);
        Uint8 r, g, b, a;
        SDL_GetRenderDrawColor(visualizer.renderer, &r, &g, &b, &a);
        int luma = (r * 299 + g * 587 + b * 114) / 1000;
            if(luma > 140)
                SDL_SetRenderDrawColor(visualizer.renderer, 0, 0, 0, 255);
            else
                SDL_SetRenderDrawColor(visualizer.renderer, 255, 255, 255, 255);
        if(remaining_ms > 0)
        {
            SDL_RenderDebugText(visualizer.renderer, (float)((visualizer.w - (debug_charsize *strlen(visualizer.timer_text))) / 2), (float)(visualizer.h / 2), visualizer.timer_text);
        }
        float width = visualizer.h / 90.0f;
        SDL_RenderDebugText(visualizer.renderer, 10.0f, width, visualizer.version_text);
        SDL_RenderPresent(visualizer.renderer);
        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if(frame_time < FPS_DELAY)
            SDL_Delay(FPS_DELAY - frame_time);
    };

    SDL_DestroyWindow(visualizer.window);
    SDL_DestroyRenderer(visualizer.renderer);
    SDL_Quit();
    return 0;
}