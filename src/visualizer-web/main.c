#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include <SDL3/SDL.h>
#include <emscripten.h>

#include "../compiler/dmx-interpreter.h"
#include "../version.h"

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

static VisualizerAppData visualizer;

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

/* --- Web file intake ---------------------------------------------------
 * SDL_ShowOpenFileDialog / SDL_ShowSimpleMessageBox have no Emscripten
 * backend in this SDL build, so file selection is done via a small JS
 * shim (EM_JS below) that reads the browser-picked file(s) and writes
 * them into Emscripten's virtual filesystem, then calls back into
 * web_receive_file() with the resulting path. Once the file exists in
 * the virtual FS, fopen()/fgets() work completely normally, so the rest
 * of this matches the native visualizer's file_dialog_handler logic. */

static void process_selected_file(VisualizerAppData *v, const char *path)
{
    const char *dot = strrchr(path, '.');
    if(!dot)
        return;
    const char *ext = dot + 1;

    SDL_Log("Read: %s", path);

    if(strcasecmp(ext, "dmx") == 0)
    {
        snprintf(v->preloaded_warning, sizeof(v->preloaded_warning), "Multiple DMX Files Read. %s Ignored.", path);
        if(!v->dmx_opened)
        {
            v->dmx.handler = fopen(path, "r");
            if(!v->dmx.handler)
            {
                SDL_Log("Error: Failed to Open DMX file: %s", path);
                return;
            }
            v->dmx_opened = true;
        }
        else
        {
            EM_ASM({ alert(UTF8ToString($0)); }, v->preloaded_warning);
        }
        return;
    }

    if(strcasecmp(ext, "dmxd") == 0)
    {
        snprintf(v->preloaded_warning, sizeof(v->preloaded_warning), "Multiple DMXD Files Read. %s Ignored.", path);
        if(!v->dmxd_opened)
        {
            v->dmxd.handler = fopen(path, "r");
            if(!v->dmxd.handler)
            {
                SDL_Log("Error: Failed to Open DMXD file: %s", path);
                return;
            }
            v->dmxd_opened = true;
        }
        else
        {
            EM_ASM({ alert(UTF8ToString($0)); }, v->preloaded_warning);
        }
        return;
    }
}

static void try_finalize(VisualizerAppData *v)
{
    if(!v->dmx.handler || !v->dmxd.handler)
        return;
    if(SDL_GetAtomicInt(&v->ready))
        return;

    if(!visualizer_load_sequence(&v->dmx, &v->dmxd))
    {
        SDL_Log("Error: Failed to Decode Sequence");
        return;
    }
    SDL_SetAtomicInt(&v->ready, 1);
}

EMSCRIPTEN_KEEPALIVE
void web_receive_file(const char *path)
{
    process_selected_file(&visualizer, path);
    try_finalize(&visualizer);
}

EM_JS(void, web_init_file_picker, (void), {
    const button = document.createElement('button');
    button.textContent = 'Load .dmx / .dmxd files';
    button.style.position = 'absolute';
    button.style.top = '8px';
    button.style.left = '8px';
    button.style.zIndex = 1000;
    document.body.appendChild(button);

    const input = document.createElement('input');
    input.type = 'file';
    input.accept = '.dmx,.dmxd';
    input.multiple = true;
    input.style.display = 'none';
    document.body.appendChild(input);

    button.addEventListener('click', () => input.click());

    input.addEventListener('change', () => {
        for (const file of input.files) {
            const reader = new FileReader();
            reader.onload = () => {
                try { FS.mkdir('/uploads'); } catch (e) {}
                const path = '/uploads/' + file.name;
                FS.writeFile(path, new Uint8Array(reader.result));
                ccall('web_receive_file', null, ['string'], [path]);
            };
            reader.readAsArrayBuffer(file);
        }
        input.value = "";
    });
});

void mainloop(void)
{
    if(!SDL_GetAtomicInt(&visualizer.running))
    {
        SDL_DestroyWindow(visualizer.window);
        SDL_DestroyRenderer(visualizer.renderer);
        SDL_Quit();
        emscripten_cancel_main_loop();
        return;
    }

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
        SDL_RenderDebugText(visualizer.renderer, (float)((visualizer.w - (SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * strlen(visualizer.timer_text))) / 2), (float)(visualizer.h / 2), visualizer.timer_text);
    }
    float width = visualizer.h / 90.0f;
    SDL_RenderDebugText(visualizer.renderer, 10.0f, width, visualizer.version_text);
    SDL_RenderPresent(visualizer.renderer);
}

int main(void)
{
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
    SDL_SetAtomicInt(&visualizer.running, 1);
    visualizer.playback_thread = SDL_CreateThread(playback, "Playback Thread", &visualizer);
    if(!visualizer.playback_thread)
        SDL_Log("Error: Failed to Create Playback Thread: %s", SDL_GetError());
    SDL_GetWindowSize(visualizer.window, &visualizer.w, &visualizer.h);
    snprintf(visualizer.version_text, sizeof(visualizer.version_text), "Compiled with Version: v%i.%i.%i", DMXLANG_VERSION_MAJOR, DMXLANG_VERSION_MINOR, DMXLANG_VERSION_PATCH);

    web_init_file_picker();

    emscripten_set_main_loop(mainloop, 0, 1);
    return 0;
}
