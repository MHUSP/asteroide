#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#define WIN_WIDTH 1100
#define WIN_HEIGHT 600
#define WIN_TITLE "Asteroides"

#define TARGET_FPS 60

#define SPRITE_SIZE 64
#define MAX_ASTS 15

#define MAX_PROJS 100
#define PROJS_SPEED 250

#define NUM_STARS 1200

#define MAX_SPEED 400

Mix_Chunk * Fire;
Mix_Chunk * Theme;
Mix_Chunk * Pause;
Mix_Chunk * Explode;


typedef struct Graphics Graphics;
typedef struct Game_State Game_State;
typedef struct Game_Resources Game_Resources;
typedef struct Options Options;
typedef struct Space_Object Asteroid;
typedef struct Space_Object Player;
typedef struct Space_Object Proj;

struct Space_Object {
    double x;
    double y;
    double vx;
    double vy;
    double angle;
    double omega;
    double dvdt; // acceleration
    int size;
    bool active;
    bool shooting;
};

struct Graphics {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* textures;
    TTF_Font* font;
};

struct Game_State {
    bool keep_playing;
    int level;
    int score;
    bool game_over;
    bool paused;
};

struct Game_Resources {
    Proj projs[MAX_PROJS];
    Asteroid asteroids[MAX_ASTS];
    Player player;
    SDL_Point stars[NUM_STARS];
};

struct Options {
    int seed;
};

void exit_sdl_error(char* msg);
Graphics init_graphics();
void end_graphics(Graphics* graphics);
Options default_options();
void print_help();
void invalid_arg(char* arg);
Options parse_options(int argc, char** argv);
void handle_events(SDL_Event* event, Game_Resources* res, Game_State* stat);
double wait_next_frame();
void draw(Graphics* gpx, Game_Resources* res, Game_State* stat);
void main_game(Options* opts, Graphics* gpx);

void PlayMusic(Mix_Chunk* chunk, int channel, int loop, float vol);
int Save(int level, int score);
int load(int* level, int* score);

#endif