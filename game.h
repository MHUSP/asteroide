#ifndef __GAME_H__
#define __GAME_H__

#include <SDL2/SDL.h>

#include "engine.h"

Game_Resources init_resources();
Game_State init_game_state(Options* opts);
void handle_key_down(SDL_Event* event, Game_Resources* res, Game_State* stat);
void handle_key_up(SDL_Event* event, Game_Resources* res);
void step_game(Game_Resources* res, Game_State* stat, double dt);
void draw_game(Graphics* gpx, Game_Resources* res, Game_State* stat);
void verify_new_game(Game_Resources* res, Game_State* stat, Graphics* gpx, Options* opts);

#endif