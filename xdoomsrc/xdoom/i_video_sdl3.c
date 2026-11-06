// i_video_sdl3.c - SDL3 video driver for psdoom
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomdef.h"
#include "doomstat.h"
#include "d_main.h"
#include "i_system.h"
#include "i_video.h"
#include "v_video.h"
#include "m_argv.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static uint32_t argb_palette[256];
static uint32_t argb_buffer[SCREENWIDTH * SCREENHEIGHT];

static int mouse_grabbed = 0;

static int TranslateKey(SDL_Keycode sym)
{
    switch (sym) {
        case SDLK_LEFT:       return KEY_LEFTARROW;
        case SDLK_RIGHT:      return KEY_RIGHTARROW;
        case SDLK_DOWN:       return KEY_DOWNARROW;
        case SDLK_UP:         return KEY_UPARROW;
        case SDLK_ESCAPE:     return KEY_ESCAPE;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:   return KEY_ENTER;
        case SDLK_TAB:        return KEY_TAB;
        case SDLK_F1:         return KEY_F1;
        case SDLK_F2:         return KEY_F2;
        case SDLK_F3:         return KEY_F3;
        case SDLK_F4:         return KEY_F4;
        case SDLK_F5:         return KEY_F5;
        case SDLK_F6:         return KEY_F6;
        case SDLK_F7:         return KEY_F7;
        case SDLK_F8:         return KEY_F8;
        case SDLK_F9:         return KEY_F9;
        case SDLK_F10:        return KEY_F10;
        case SDLK_F11:        return KEY_F11;
        case SDLK_F12:        return KEY_F12;
        case SDLK_BACKSPACE:  return KEY_BACKSPACE;
        case SDLK_PAUSE:      return KEY_PAUSE;
        case SDLK_EQUALS:     return KEY_EQUALS;
        case SDLK_MINUS:      return KEY_MINUS;
        case SDLK_RSHIFT:
        case SDLK_LSHIFT:     return KEY_RSHIFT;
        case SDLK_RCTRL:
        case SDLK_LCTRL:      return KEY_RCTRL;
        case SDLK_RALT:
        case SDLK_LALT:       return KEY_RALT;
        case SDLK_CAPSLOCK:   return KEY_CAPSLOCK;
        default:
            if (sym >= SDLK_A && sym <= SDLK_Z)
                return sym - SDLK_A + 'a';
            if (sym >= SDLK_0 && sym <= SDLK_9)
                return sym - SDLK_0 + '0';
            if (sym < 128)
                return (int)sym;
            return 0;
    }
}

void I_InitGraphics(void)
{
    static int initialized = 0;
    if (initialized)
        return;
    initialized = 1;

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
        I_Error("SDL_InitSubSystem(VIDEO) failed: %s", SDL_GetError());
    }

    int window_width = SCREENWIDTH * 2;
    int window_height = SCREENHEIGHT * 2;

    if (M_CheckParm("-3")) {
        window_width = SCREENWIDTH * 3;
        window_height = SCREENHEIGHT * 3;
    } else if (M_CheckParm("-4")) {
        window_width = SCREENWIDTH * 4;
        window_height = SCREENHEIGHT * 4;
    } else if (M_CheckParm("-1")) {
        window_width = SCREENWIDTH;
        window_height = SCREENHEIGHT;
    }

    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS;
    if (!M_CheckParm("-window") && !M_CheckParm("-windowed")) {
        flags |= SDL_WINDOW_FULLSCREEN;
    }

    window = SDL_CreateWindow("psDooM", window_width, window_height, flags);
    if (!window) {
        I_Error("SDL_CreateWindow failed: %s", SDL_GetError());
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        I_Error("SDL_CreateRenderer failed: %s", SDL_GetError());
    }

    SDL_SetRenderLogicalPresentation(renderer, SCREENWIDTH, SCREENHEIGHT,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                SCREENWIDTH, SCREENHEIGHT);
    if (!texture) {
        I_Error("SDL_CreateTexture failed: %s", SDL_GetError());
    }

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    if (M_CheckParm("-nograb")) {
        mouse_grabbed = 0;
    } else {
        mouse_grabbed = 1;
        SDL_SetWindowRelativeMouseMode(window, true);
    }
}

void I_ShutdownGraphics(void)
{
    if (mouse_grabbed && window) {
        SDL_SetWindowRelativeMouseMode(window, false);
    }
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = NULL;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void I_SetPalette(byte *palette)
{
    for (int i = 0; i < 256; i++) {
        uint8_t r = gammatable[usegamma][*palette++];
        uint8_t g = gammatable[usegamma][*palette++];
        uint8_t b = gammatable[usegamma][*palette++];
        argb_palette[i] = (0xFFu << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
    }
}

void I_UpdateNoBlit(void)
{
}

void I_FinishUpdate(void)
{
    if (!screens[0] || !texture || !renderer)
        return;

    const byte *src = screens[0];
    for (int i = 0; i < SCREENWIDTH * SCREENHEIGHT; i++) {
        argb_buffer[i] = argb_palette[src[i]];
    }

    SDL_UpdateTexture(texture, NULL, argb_buffer, SCREENWIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void I_ReadScreen(byte *scr)
{
    if (screens[0]) {
        memcpy(scr, screens[0], SCREENWIDTH * SCREENHEIGHT);
    }
}

void I_StartFrame(void)
{
}

void I_StartTic(void)
{
    SDL_Event sdlevent;
    event_t event;

    while (SDL_PollEvent(&sdlevent)) {
        switch (sdlevent.type) {
            case SDL_EVENT_QUIT:
                I_Quit();
                break;

            case SDL_EVENT_KEY_DOWN: {
                if (sdlevent.key.key == SDLK_RETURN && (sdlevent.key.mod & SDL_KMOD_ALT)) {
                    SDL_WindowFlags cur_flags = SDL_GetWindowFlags(window);
                    bool is_full = (cur_flags & SDL_WINDOW_FULLSCREEN) != 0;
                    SDL_SetWindowFullscreen(window, !is_full);
                    break;
                }
                int key = TranslateKey(sdlevent.key.key);
                if (key != 0) {
                    event.type = ev_keydown;
                    event.data1 = key;
                    event.data2 = 0;
                    event.data3 = 0;
                    D_PostEvent(&event);
                }
                break;
            }

            case SDL_EVENT_KEY_UP: {
                int key = TranslateKey(sdlevent.key.key);
                if (key != 0) {
                    event.type = ev_keyup;
                    event.data1 = key;
                    event.data2 = 0;
                    event.data3 = 0;
                    D_PostEvent(&event);
                }
                break;
            }

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                int buttons = 0;
                SDL_MouseButtonFlags btn_state = SDL_GetMouseState(NULL, NULL);
                if (btn_state & SDL_BUTTON_LMASK) buttons |= 1;
                if (btn_state & SDL_BUTTON_RMASK) buttons |= 2;
                if (btn_state & SDL_BUTTON_MMASK) buttons |= 4;

                event.type = ev_mouse;
                event.data1 = buttons;
                event.data2 = 0;
                event.data3 = 0;
                D_PostEvent(&event);
                break;
            }

            case SDL_EVENT_MOUSE_MOTION: {
                int buttons = 0;
                SDL_MouseButtonFlags btn_state = SDL_GetMouseState(NULL, NULL);
                if (btn_state & SDL_BUTTON_LMASK) buttons |= 1;
                if (btn_state & SDL_BUTTON_RMASK) buttons |= 2;
                if (btn_state & SDL_BUTTON_MMASK) buttons |= 4;

                event.type = ev_mouse;
                event.data1 = buttons;
                event.data2 = (int)(sdlevent.motion.xrel * 4.0f);
                event.data3 = -(int)(sdlevent.motion.yrel * 4.0f);
                D_PostEvent(&event);
                break;
            }

            default:
                break;
        }
    }
}
