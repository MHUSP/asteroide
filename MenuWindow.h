//
// Created by Asus on 10/07/2020.
//

#ifndef ASTEROIDES_MENUWINDOW_H
#define ASTEROIDES_MENUWINDOW_H
#include "engine.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define  MENU_EXIT_CLOSE_WINDOW -1
#define  MENU_EXIT_PLAY_GAME 1


SDL_Texture* ButtonStartImg;
SDL_Rect ButtonStartRect = {.x=0, .y=0,.w=217,.h=65};

SDL_Texture* LogoImg;
SDL_Rect LogoRect = {.x=0, .y=0,.w=631,.h=303};

int terminate = 0;

Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
        case 1:
            return *p;
            break;

        case 2:
            return *(Uint16 *)p;
            break;

        case 3:
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
            else
                return p[0] | p[1] << 8 | p[2] << 16;
            break;

        case 4:
            return *(Uint32 *)p;
            break;

        default:
            return 0;       /* shouldn't happen, but avoids warnings */
    }
}

SDL_Texture* LoadTexture(Graphics* graphics, char* filename)
{
    SDL_Surface* surf = IMG_Load(filename);
    if (surf == NULL) exit_sdl_error("Impossível ler o arquivo.");
    SDL_Texture*  texture = SDL_CreateTextureFromSurface(graphics->renderer, surf);
    SDL_FreeSurface(surf);
    return  texture;
}

Graphics MenuInit()
{
    Graphics graphics = init_graphics();

    ButtonStartImg = LoadTexture(&graphics,"../btnStart.png") ;
    LogoImg = LoadTexture(&graphics,"../logo.png") ;
    SDL_RenderPresent(graphics.renderer);

    return graphics;

}


bool IsClickInArea(int32_t x, int32_t y,SDL_Rect area)
{
    int32_t  areaRight = area.x+area.w;
    int32_t  areaBotom = area.y+area.h;

    if((x>area.x)&&(x<areaRight))
    {
        if((y>area.y)&&(y<areaBotom))
        {
            return  true;
        }
    }
    return  false;
}

int32_t  x_,y_;



void MenuLoop( double delta, Graphics* graphics)
{
    SDL_Rect dest =  {.x=(WIN_WIDTH-ButtonStartRect.w)/2, .y=(WIN_HEIGHT-ButtonStartRect.h)*3/4,.w=ButtonStartRect.w,.h=ButtonStartRect.h};
    SDL_RenderCopy(graphics->renderer,ButtonStartImg ,&ButtonStartRect,&dest );

    SDL_Rect destlogo =  {.x=(WIN_WIDTH-LogoRect.w)/2, .y=(WIN_HEIGHT-LogoRect.h)/5,.w=LogoRect.w,.h=LogoRect.h};
    SDL_RenderCopy(graphics->renderer,LogoImg ,&LogoRect,&destlogo );

//    *terminate = 1;
}

void MenuKeyPress(int key)
{
   printf("Key press %d\n",key);
}


void MenuMouseClick(int button, int32_t x, int32_t y)
{
    x_ = x;
    y_ = y;


    SDL_Rect btnArea =  {.x=(WIN_WIDTH-ButtonStartRect.w)/2, .y=(WIN_HEIGHT-ButtonStartRect.h)*3/4,.w=ButtonStartRect.w,.h=ButtonStartRect.h};
    if(IsClickInArea(x,y,btnArea))
    {
        printf("Mouse Ok Click %d at x:%d and y:%d\n",button,x,y);
        terminate = MENU_EXIT_PLAY_GAME;
    }
}

void MenuHandleEvents(SDL_Event* event) {
    switch (event->type)
    {
        case SDL_KEYUP:
            MenuKeyPress(event->key.keysym.sym);
            break;
        case SDL_MOUSEBUTTONUP:
            MenuMouseClick(event->button.button,event->button.x,event->button.y);
            break;
        default:
            break;
    }
}

void MenuFinish(Graphics graphics)
{
    SDL_DestroyTexture(ButtonStartImg);
    SDL_DestroyTexture(LogoImg);
    end_graphics(&graphics);
}


int ShowMenuWindow(Options options)
{
    Graphics graphics = MenuInit();
    SDL_Event event;
    double delta = 1.0 / TARGET_FPS;
    do {
        uint32_t last_time = SDL_GetTicks();
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                terminate = MENU_EXIT_CLOSE_WINDOW;
                break;
            }

            MenuHandleEvents(&event);
        }
        MenuLoop(delta,&graphics);
        SDL_RenderPresent(graphics.renderer);
        delta = wait_next_frame(last_time);
    }
    while(!terminate);
    MenuFinish(graphics);
    return terminate;
}



#endif //ASTEROIDES_MENUWINDOW_H
