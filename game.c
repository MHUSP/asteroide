#include <math.h>

#include "game.h"
#include "engine.h"
#include <stdio.h>
#include <SDL2/SDL.h>
#define TO_GRAUS 180 / 3.14159265358979323846

Game_Resources init_resources() {
    Game_Resources res;
    
    for (int k = 0; k < MAX_ASTS; k++)
        res.asteroids[k] = (Asteroid) {.active = false};
    
    for (int k = 0; k < MAX_PROJS; k++)
        res.projs[k].active = false;

    Player player = {.x = 300, .y = 300, .vx = rand() % 50, .vy = rand() % 50};
    res.player = player;

    for (int k = 0; k < NUM_STARS; k++) {
        res.stars[k] = (SDL_Point) {.x = rand() % WIN_WIDTH, .y = rand() % WIN_HEIGHT};
    }

    return res;
}

Game_State init_game_state(Options* opts) {
    srand(opts->seed);

    Game_State stat;
    stat.keep_playing = true;
    stat.level = 0;
    stat.score = 0;
    stat.game_over = false;
    stat.paused = false;

    return stat;
}

void wrap_coords(double x, double y, double* xo, double* yo) {
    if (x >= WIN_WIDTH)
        *xo -= WIN_WIDTH;
    if (x < 0)
        *xo += WIN_WIDTH;

    if (y < 0)
        *yo += WIN_HEIGHT;
    if (y >= WIN_HEIGHT)
        *yo -= WIN_HEIGHT;
}

void build_proj(Game_Resources* res) {
    static uint32_t last_fire = 0;
    uint32_t now = SDL_GetTicks();

    if (last_fire != 0 && now - last_fire < 75)
        return;
    last_fire = now;

    int ptr;
    for (ptr = 0; ptr < MAX_PROJS; ptr++)
        if (!res->projs[ptr].active) break;
    if (ptr == MAX_PROJS) return;

    Proj* proj = &(res->projs[ptr]);
    proj->active = true;
    double x_proj = res->player.x + SPRITE_SIZE / 4 + (SPRITE_SIZE / 4) * cos(res->player.angle);
    double y_proj = res->player.y + SPRITE_SIZE / 4 - (SPRITE_SIZE / 4) * sin(res->player.angle);
    wrap_coords(x_proj, y_proj, &x_proj, &y_proj);

    proj->x = x_proj;
    proj->y = y_proj;
    proj->vx = res->player.vx + PROJS_SPEED * cos(res->player.angle);
    proj->vy = res->player.vy - PROJS_SPEED * sin(res->player.angle);
}

void handle_key_down(SDL_Event* event, Game_Resources* res, Game_State* stat) {
    SDL_MessageBoxButtonData buttons[] = {
            { 0,                                       4, "Sim"},
            { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Não"},
    };
    SDL_MessageBoxData messageboxdata = {
            0,
            NULL,
            "Fechar jogo",
            "Tem certeza que quer fechar o jogo?",
            SDL_arraysize(buttons),
            buttons,

    };
    int buttonid;
    switch (event->key.keysym.sym)
    {
            break;
        case SDLK_s:
            Save(stat->level, stat->score);
            break;
        case SDLK_q:
            stat->keep_playing = false;
            break;
        case SDLK_LEFT:
            if (res->player.omega < 2) res->player.omega++;
            break;
        case SDLK_RIGHT:
            if (res->player.omega > -2) res->player.omega--;
            break;
        case SDLK_UP:
            res->player.dvdt = 200;
            break;
        case SDLK_f:
            res->player.shooting = true;
            PlayMusic(Fire,0,1,50.0);
            break;
        case SDLK_n:
            stat->game_over = true;
            break;
        case SDLK_ESCAPE:

            if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
                SDL_Log("error displaying message box");
            }
            if (buttonid == 4){
                stat->keep_playing = false;
            }
            break;

        case SDLK_p:
            stat->paused = !stat->paused;
            PlayMusic(Pause,2,1,100.0);
            if(stat->paused)
            {
                Mix_Pause(1);
            }
            else
            {
               while(Mix_Playing(2));
               Mix_Resume(1);
            }
        default:
            break;
    }
}

void handle_key_up(SDL_Event* event, Game_Resources* res) {
    switch (event->key.keysym.sym)
    {
        case SDLK_f:
            res->player.shooting = false;
            break;
        default:
            break;
    }
}

void step_asteroid(Game_Resources* res, double dt) {
    for (int k = 0; k < MAX_ASTS; k++) {
        Asteroid* ast = &(res->asteroids[k]);
        if (!ast->active) continue;

        ast->x += ast->vx * dt;
        ast->y += ast->vy * dt;
        wrap_coords(ast->x, ast->y, &(ast->x), &(ast->y));

        ast->angle += ast->omega * dt;
    }
}

void step_projs(Game_Resources* res, double dt) {
    for (int k = 0; k < MAX_PROJS; k++) {
        Proj* proj = &(res->projs[k]);

        if (!proj->active) continue;


        proj->x += proj->vx * dt;
        proj->y += proj->vy * dt;

        if (proj->x < 0 || proj->x >= WIN_WIDTH) proj->active = false;
        if (proj->y < 0 || proj->y >= WIN_HEIGHT) proj->active = false;
    }
}

void step_player(Game_Resources* res, double dt) {
    Player* player = &(res->player);

    player->x += player->vx * dt;
    player->y += player->vy * dt;
    wrap_coords(player->x, player->y, &(player->x), &(player->y));

    player->angle += player->omega * dt;

    player->vx += + cos(player->angle) * player->dvdt * dt;
    player->vy += - sin(player->angle) * player->dvdt * dt;

    if (player->vx > MAX_SPEED) player->vx = MAX_SPEED;
    if (player->vy > MAX_SPEED) player->vy = MAX_SPEED;

    // Limpa a aceleração após a atualização
    player->dvdt = 0;
}

bool intersecting(Proj* proj, Asteroid* ast) {
    double r = SPRITE_SIZE / 2 / ast->size;
    double dx = proj->x - (ast->x + SPRITE_SIZE / 2 / ast->size);
    double dy = proj->y - (ast->y + SPRITE_SIZE / 2 / ast->size);
    bool one = r * r > dx * dx + dy * dy;

    double x_wrap = ast->x + SPRITE_SIZE / 2 / ast->size;
    double y_wrap = ast->y + SPRITE_SIZE / 2 / ast->size;

    wrap_coords(x_wrap, y_wrap, &x_wrap, &y_wrap);

    dx = proj->x - x_wrap;
    dy = proj->y - y_wrap;
    bool two = r * r > dx * dx + dy * dy;

    return one || two;
}

void build_asteroid(Game_Resources* res, int x, int y, int size) {
    if (size > 4) return;

    int ptr = 0;
    for (ptr = 0; ptr < MAX_ASTS; ptr++)
        if (!res->asteroids[ptr].active) break;
    if (ptr == MAX_ASTS) return;

    Asteroid* ast = &(res->asteroids[ptr]);
    ast->active = true;
    ast->x = x - 15 + (rand() % 15);
    ast->y = y - 15 + (rand() % 15);
    ast->size = size;
    ast->angle = 0;
    ast->omega = rand() % 2 == 0 ? -0.5 : 0.5;
    ast->vx = rand() % 50;
    ast->vy = rand() % 50;
}

void verify_proj_ast(Game_Resources* res, Game_State* stat) {
    for (int p = 0; p < MAX_PROJS; p++) {
        Proj* proj = &(res->projs[p]);
        if (!proj->active) continue;
        for (int a = 0; a < MAX_ASTS; a++) {
            Asteroid* ast = &(res->asteroids[a]);
            if (!ast->active) continue;

            if (intersecting(proj, ast)) {
                PlayMusic(Explode,3,1,100.0);

                stat->score += 8 / ast->size;

                proj->active = false;
                ast->active = false;

                int x = ast->x;
                int y = ast->y;
                int size = ast->size * 2;

                build_asteroid(res, x, y, size);
                build_asteroid(res, x, y, size);
                break;
            }
        }
    }
}

void verify_ast_player(Game_Resources* res, Game_State* stat) {
    double x_player_center = res->player.x + SPRITE_SIZE / 4;
    double y_player_center = res->player.y + SPRITE_SIZE / 4;
    double r_player = SPRITE_SIZE / 4;

    for (int k = 0; k < MAX_ASTS; k++) {
        Asteroid* ast = &(res->asteroids[k]);
        if (!ast->active) continue;

        double r_ast = SPRITE_SIZE / 2 / ast->size;

        double x_ast_center = ast->x + SPRITE_SIZE / 2 / ast->size;
        double y_ast_center = ast->y + SPRITE_SIZE / 2 / ast->size;

        double dx = x_player_center - x_ast_center;
        double dy = y_player_center - y_ast_center;

        double r = sqrt(dx * dx + dy * dy);

        if (r < r_player + r_ast) {
            stat->game_over = true;
            
            return;
        }

        // Agora com wrap
        wrap_coords(x_ast_center, y_ast_center, &x_ast_center, &y_ast_center);

        dx = x_player_center - x_ast_center;
        dy = y_player_center - y_ast_center;

        r = sqrt(dx * dx + dy * dy);

        if (r < r_player + r_ast) {
            stat->game_over = true;
            return;
        }
    }
}

void verify_new_game(Game_Resources* res, Game_State* stat, Graphics* gpx, Options* opts) {
    if (!stat->game_over) return;

    SDL_MessageBoxButtonData buttons[] = {
            { 0,                                       4, "Fechar"},
            { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Reviver"},
            { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 2, "Reiniciar"},
    };
    SDL_MessageBoxData messageboxdata = {
            0,
            NULL,
            "Fim de Jogo",
            "Sua nave foi atingida",
            SDL_arraysize(buttons),
            buttons,

    };
    int buttonid;
    if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
        SDL_Log("error displaying message box");
        return;
    }
    if (buttonid == 4){
        stat->keep_playing = false;
        return;

    }
    if (buttonid == 2) {
        SDL_Log("Reiniciou jogo");
        * res = init_resources();
        *stat = init_game_state(opts);
    }

    if (buttonid == 1) {
        int score = stat->score;
        int level = stat->level;

        if (stat->score >= 200) {
            * res = init_resources();
            *stat = init_game_state(opts);
            stat->score = score - 200;
            stat->game_over = false;
            stat->level = level - 1;


            return;                }
    }
            *res = init_resources();
    *stat = init_game_state(opts);
}

void verify_collisions(Game_Resources* res, Game_State* stat) {
    verify_proj_ast(res, stat);
    verify_ast_player(res, stat);
}

void new_level(Game_Resources* res, Game_State* stat) {
    for (int k = 0; k < MAX_ASTS; k++)
        if (res->asteroids[k].active) return;
    stat->level++;

    Player* player = &(res->player);
    double player_center_x = player->x + SPRITE_SIZE / 4;
    double player_center_y = player->y + SPRITE_SIZE / 4;

    for (int k = 0; k < MAX_ASTS && k < stat->level; k++) {
        double x, y, dist2 = 0;
        while (dist2 < SPRITE_SIZE * SPRITE_SIZE) {
            x = rand() % WIN_WIDTH;
            y = rand() % WIN_HEIGHT;

            double x_center = x + SPRITE_SIZE / 2;
            double y_center = y + SPRITE_SIZE / 2;

            double dx = player_center_x - x_center;
            double dy = player_center_y - y_center;

            dist2 = dx * dx + dy * dy;
        }
        build_asteroid(res, x, y, 1);
    }
}

void step_stars(Game_Resources* res, double dt) {
    for (int k = 0; k < NUM_STARS; k++) {
        SDL_Point* star = &(res->stars[k]);

        // SE NÃO COLOCA O CAST, AS ESTRELAS GRUDAM NA TELA SABE-SE BELZEBU LÁ PQ
        star->x -= (int) (res->player.vx * dt);
        star->y -= (int) (res->player.vy * dt);

        double x = star->x;
        double y = star->y;

        wrap_coords(x, y, &x, &y);

        star->x = (int) x;
        star->y = (int) y;
    }
}

void step_game(Game_Resources* res, Game_State* stat, double dt) {
    if (stat->paused)
        return;

    if (res->player.shooting)
        build_proj(res);
    
    verify_collisions(res, stat);
    step_asteroid(res, dt);
    step_projs(res, dt);
    step_stars(res, dt);
    step_player(res, dt);

    // Verifica fim de uma fase
    new_level(res, stat);
}

void draw_wrap(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect* src, SDL_Rect* dest, SDL_Point* center, double angle) {    
    if (dest->x > WIN_WIDTH - SPRITE_SIZE) {
        SDL_Rect wr = {.x = -(WIN_WIDTH - dest->x), .y = dest->y, .w = dest->w, .h = dest->h};
        SDL_RenderCopyEx(renderer, texture, src, &wr, -angle * TO_GRAUS, center, SDL_FLIP_NONE);
    }

    if (dest->y > WIN_HEIGHT - SPRITE_SIZE) {
        SDL_Rect wr = {.x = dest->x, .y = -(WIN_HEIGHT - dest->y), .w = dest->w, .h = dest->h};
        SDL_RenderCopyEx(renderer, texture, src, &wr, -angle * TO_GRAUS, center, SDL_FLIP_NONE);
    }

    SDL_RenderCopyEx(renderer, texture, src, dest, -angle * TO_GRAUS, center, SDL_FLIP_NONE);
}

void draw_asteroids(Graphics* gpx, Game_Resources* res) {
    for (int k = 0; k < MAX_ASTS; k++) {
        Asteroid* ast = &(res->asteroids[k]);
        if (!ast->active) continue;

        SDL_Rect dest = {
            .x = ast->x,
            .y = ast->y,
            .w = SPRITE_SIZE / ast->size,
            .h = SPRITE_SIZE / ast->size
        };
        SDL_Rect src = {.x = 0, .y = 0, .w = SPRITE_SIZE, .h = SPRITE_SIZE};

        SDL_Point center = {.x = SPRITE_SIZE / 2 / ast->size, .y = SPRITE_SIZE / 2 / ast->size};
        draw_wrap(gpx->renderer, gpx->textures, &src, &dest, &center, ast->angle);
    }
}

void draw_player(Graphics* gpx, Game_Resources* res) {
    Player* player = &(res->player);

    SDL_Rect dest = {.x = player->x, .y = player->y, .w = SPRITE_SIZE / 2, .h = SPRITE_SIZE / 2};
    SDL_Rect src = {.x = 64, .y = 0, .w = SPRITE_SIZE / 2, .h = SPRITE_SIZE / 2};
    SDL_Point center = {.x = SPRITE_SIZE / 4, .y = SPRITE_SIZE / 4};

    draw_wrap(gpx->renderer, gpx->textures, &src, &dest, &center, player->angle);
}

void draw_projs(Graphics* gpx, Game_Resources* res) {
    //SDL_SetRenderDrawColor(gpx->renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);

    for (int k = 0; k < MAX_PROJS; k++) {
        Proj* proj = &(res->projs[k]);
        if (!proj->active) continue;

        SDL_Rect src = {.x = 64, .y = 32, .w = 3, .h = 3};
        SDL_Rect dest = {.x = proj->x - 1, .y = proj->y - 1, .w = 3, .h = 3};
        SDL_RenderCopy(gpx->renderer, gpx->textures, &src, &dest);        

        //SDL_RenderDrawPoint(gpx->renderer, proj->x, proj->y);
    }
}

void draw_pause(Graphics* gpx)
{
    SDL_Color color = {0xFF, 0, 0, 255};
    char buffer[50];
    snprintf(buffer, 50, "Pause");

    SDL_Surface* textSurface = TTF_RenderText_Solid( gpx->font, buffer, color );
    SDL_Texture* Message = SDL_CreateTextureFromSurface(gpx->renderer, textSurface);

    SDL_Rect Message_rect; //create a rect
    Message_rect.x = (WIN_WIDTH-textSurface->w)/2;  //controls the rect's x coordinate
    Message_rect.y = (WIN_HEIGHT-textSurface->h)/2; // controls the rect's y coordinte
    Message_rect.w = textSurface->w; // controls the width of the rect
    Message_rect.h = textSurface->h; // controls the height of the rect

    SDL_RenderCopy(gpx->renderer, Message, NULL, &Message_rect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(Message);
}

void draw_score(Graphics* gpx, Game_State* stat) {
    SDL_Color color = {0xFF, 0, 0, 255};
    char buffer[50];
    snprintf(buffer, 50, "SCORE: %04d LEVEL: %d", stat->score, stat->level);

    SDL_Surface* textSurface = TTF_RenderText_Solid( gpx->font, buffer, color );
    SDL_Texture* Message = SDL_CreateTextureFromSurface(gpx->renderer, textSurface);
    
    SDL_Rect Message_rect; //create a rect
    Message_rect.x = 15;  //controls the rect's x coordinate 
    Message_rect.y = 15; // controls the rect's y coordinte
    Message_rect.w = textSurface->w; // controls the width of the rect
    Message_rect.h = textSurface->h; // controls the height of the rect

    SDL_RenderCopy(gpx->renderer, Message, NULL, &Message_rect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(Message);
}

void draw_stars(Graphics* gpx, Game_Resources* res) {
    SDL_SetRenderDrawColor(gpx->renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawPoints(gpx->renderer, res->stars, NUM_STARS);
}

void draw_game(Graphics* gpx, Game_Resources* res, Game_State* stat) {
    if(stat->paused) {
        draw_pause(gpx);
    }
    else {
        draw_stars(gpx, res);
        draw_asteroids(gpx, res);
        draw_projs(gpx, res);
        draw_player(gpx, res);
        draw_score(gpx, stat);
    }
}