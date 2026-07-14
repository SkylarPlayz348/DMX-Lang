#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include <SDL3/SDL.h>

#include "../compiler/dmx-interpreter.h"
#include "../version.h"

#define FPS 60
#define FPS_DELAY 1000 / FPS

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
/**************************** Based On OpenBSD's libc ****************************/
/* Here if I need to define guard it
static const unsigned char charmap[] = {
	'\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
	'\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
	'\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
	'\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
	'\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
	'\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
	'\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
	'\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
	'\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	'\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
	'\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	'\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
	'\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
	'\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
	'\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
	'\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
	'\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
	'\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
	'\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
	'\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
	'\300', '\301', '\302', '\303', '\304', '\305', '\306', '\307',
	'\310', '\311', '\312', '\313', '\314', '\315', '\316', '\317',
	'\320', '\321', '\322', '\323', '\324', '\325', '\326', '\327',
	'\330', '\331', '\332', '\333', '\334', '\335', '\336', '\337',
	'\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
	'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
	'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
	'\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
};

int strcasecmp(const char *s1, const char *s2)
{
	const unsigned char *cm = charmap;
	const unsigned char *us1 = (const unsigned char *)s1;
	const unsigned char *us2 = (const unsigned char *)s2;

	while (cm[*us1] == cm[*us2++])
		if (*us1++ == '\0')
			return (0);
	return (cm[*us1] - cm[*--us2]);
} */
/********************************************************************************/

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