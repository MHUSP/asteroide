#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <stdint.h>
#define COMP_NAME "CEFET"
#define GAME_NAME "EXEMPLO_SALVAR"
#include "engine.h"
#include "game.h"


void exit_sdl_error(char* msg) {
    fprintf(stderr, "%s: %s\n", msg, SDL_GetError());
    exit(EXIT_FAILURE);
}

Graphics init_graphics() {
    Graphics graphics;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        exit_sdl_error("Impossível iniciar o vídeo.");

    graphics.window = SDL_CreateWindow(
        WIN_TITLE,
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED,
        WIN_WIDTH, 
        WIN_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
    );
    if (graphics.window == NULL) exit_sdl_error("Impossível criar a janela");

    graphics.renderer = SDL_CreateRenderer(graphics.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (graphics.renderer == NULL) exit_sdl_error("Impossível criar o renderer");

    SDL_Surface* surface_tile = IMG_Load("../sprites.png");
    if (surface_tile == NULL) exit_sdl_error("Impossível ler o arquivo de sprites.");
    
    graphics.textures = SDL_CreateTextureFromSurface(
        graphics.renderer,
        surface_tile
    );
    if (graphics.textures == NULL) exit_sdl_error("Impossível criar textura dos tiles.");

    SDL_FreeSurface(surface_tile);

    if (TTF_Init() < 0) exit_sdl_error("Impossível inicializar TTF.");

    graphics.font = TTF_OpenFont("../monogram.ttf", 32);
    if (graphics.font == NULL) exit_sdl_error("Impossível carregar a fonte.");

    return graphics;
}

void end_graphics(Graphics* graphics) {
    SDL_DestroyTexture(graphics->textures);
    SDL_DestroyRenderer(graphics->renderer);
    SDL_DestroyWindow(graphics->window);
    SDL_Quit();
}

Options default_options() {
    Options opts;
    opts.seed = 10;
    return opts;
}

void print_help() {
    printf("Ajuda\n");
}

void invalid_arg(char* arg) {
    fprintf(stderr, "Formato inválido para %s: %s\n", arg, optarg);
    exit(EXIT_FAILURE);
}

Options parse_options(int argc, char** argv) {
    // Opções padrão
    Options options = default_options();

    int c;
    while ((c = getopt(argc, argv, "s:h")) != -1) {
        switch(c) {
            case 'h':
                print_help();
                exit(EXIT_SUCCESS);
                break;
            case 's':
                options.seed = atoi(optarg);
                if (options.seed < 1) {
                    invalid_arg("s");
                }
                break;
            case '?':
                fprintf(stderr, "A opção -%c necessita de um argumento.\n", optopt);
                exit(EXIT_FAILURE);
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }

    srand(options.seed);

    return options;
}

void handle_events(SDL_Event* event, Game_Resources* res, Game_State* stat) {
    switch (event->type)
    {
    case SDL_KEYDOWN:
        handle_key_down(event, res, stat);
        break;
    case SDL_KEYUP:
        handle_key_up(event, res);
        break;
    default:
        break;
    }
}

double wait_next_frame(uint32_t last_time) {
    uint32_t now = SDL_GetTicks();
    uint32_t elapsed = now - last_time;
    
    uint32_t should_wait_total = (int) 1000.0 / TARGET_FPS;

    if (should_wait_total > elapsed) {
        uint32_t should_wait_encore = should_wait_total - elapsed;
        SDL_Delay(should_wait_encore);
        return should_wait_total / 1000.0;
    } else {
        return elapsed / 1000.0;
    }
}

void draw(Graphics* gpx, Game_Resources* res, Game_State* stat) {
    SDL_SetRenderDrawColor(gpx->renderer, 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(gpx->renderer);

    draw_game(gpx, res, stat);

    SDL_RenderPresent(gpx->renderer);
}
char* get_save_path() {
    char* path = SDL_GetPrefPath( COMP_NAME, GAME_NAME );
    strcat(path,"save_game");
    return path;
}

int loadInt(FILE *f)
{
    char line[50];
    if (fgets(line, sizeof line, f) == NULL)
    {
        return 0;
    }
    int value = atoi(line);
    return value;
}

int loadFloat(FILE *f)
{
    char line[50];
    if (fgets(line, sizeof line, f) == NULL)
    {
        return 0;
    }
    int value = atof(line);
    return value;
}
int load(int* level, int* score)
{

    FILE *arquivo;
    arquivo = fopen(get_save_path() , "r");
    if (arquivo == NULL) {
        printf("Não foi possivel criar o save\n\n");
        return 0;
    }

    *level = loadInt(arquivo);
    *score = loadInt(arquivo);

    fclose(arquivo);
    return 1;
}

int Save(int level, int score)
{
    FILE *arquivo;
    arquivo = fopen(get_save_path() , "w+");
    if (arquivo == NULL) {
        printf("Não foi encontrado o save\n\n");
        return 0;
    }

    fprintf(arquivo,"%d\n",level);
    fprintf(arquivo,"%d\n",score);

    fclose(arquivo);
    return 1;
}

void main_game(Options* opts, Graphics* gpx) {
    Game_Resources res = init_resources(opts);
    Game_State stat = init_game_state(opts);

    bool first = true;
    SDL_Event event;
    double dt = 1.0 / TARGET_FPS;
    while (stat.keep_playing) {
        uint32_t last_time = SDL_GetTicks();
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                stat.keep_playing = false;
                break;
            }

            handle_events(&event, &res, &stat);
        }

        step_game(&res, &stat, dt);
        draw(gpx, &res, &stat);
            if (first){
                Mix_Pause(1);
                first = false;
                SDL_MessageBoxButtonData buttons[] = {
                        { 0,                                       4, "Tutorial"},
                        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Load Game"},
                        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 2, "New Game"},
                };
                SDL_MessageBoxData messageboxdata = {
                        0,
                        NULL,
                        "Inicio de jogo",
                        "",
                        SDL_arraysize(buttons),
                        buttons,

                };
                int buttonid;
                if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
                    SDL_Log("error displaying message box");

                }
                if (buttonid == 4) {
                    first = true;
                    SDL_ShowSimpleMessageBox(
                            0,
                            "Tutorial",
                            "Aperte S para salvar o jogo.\n Controle a nave nas setas e atire no F.\n Para reviver necessita de 200 pontos de Score\n Tecla P pausa o jogo.",
                            gpx->window
                    );

                }
                if (buttonid == 1) {
                    load(&stat.level, &stat.score);


                }
                Mix_Resume(1);
                continue;
            }

        dt = wait_next_frame(last_time);

        verify_new_game(&res, &stat, gpx, opts);
    }
}

Mix_Chunk* LoadMusic(char* filename)
{
    Mix_Chunk* chunk = Mix_LoadWAV(filename);
    if(chunk ==NULL)
    {
        exit_sdl_error("Erro ao carregar audio");
    }
    return chunk;
}

void init_audio()
{
    Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096);
    Fire = LoadMusic("..\\tiro.wav");
    Theme = LoadMusic("..\\fundo.wav");
    Pause= LoadMusic("..\\Pause.wav");
    Explode = LoadMusic("..\\Explode.wav");

    PlayMusic(Theme,1,-1,5.0);
}

void end_audio()
{
    if(Fire!=NULL)
        Mix_FreeChunk (Fire);
    Mix_CloseAudio();
}

void PlayMusic(Mix_Chunk* chunk, int channel, int loop, float vol)
{
   //if(Mix_Playing(channel))
//        return;

    if(loop < 0)
    {
        loop = -1;
    }else
    {
        loop -= 1;
    }
    Mix_VolumeChunk(chunk,(int)(vol*1.28));
    if(Mix_PlayChannel(channel, chunk, loop)) {
        printf("Mix_PlayChannel: %s\n",Mix_GetError());
    };
}

void ShowGameWindow(Options options)
{

    Graphics graphics = init_graphics();
    init_audio();

    main_game(&options, &graphics);

    end_audio();
    end_graphics(&graphics);
}

int main(int argc, char** argv) {
    Options options = parse_options(argc, argv);

    ShowGameWindow(options);

    return EXIT_SUCCESS;
}