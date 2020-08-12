#include <time.h>
#include <math.h>
#include <iostream>
#include "ConwaysGame.h"

// private

void ConwaysGame::create_grid(){
    int x = ceil(window_size.x / scale);
    int y = ceil(window_size.y / scale);
    grid_size.x = x;
    grid_size.y = y;
    grid = new Tile*[y];
    for(int i = 0; i < y; i++){
        grid[i] = new Tile[x];
    }
}

void ConwaysGame::resize_grid(){
    this->~ConwaysGame();
    create_grid();
}

void ConwaysGame::draw_grid_lines(){
    for(int i = 0; i <= window_size.y; i += scale)
        DrawLine(0, i, window_size.x, i, Color{125, 125, 125, 100});
    for(int i = 0; i <= window_size.x; i += scale)
        DrawLine(i, 0, i, window_size.y, Color{125, 125, 125, 100});
}

void ConwaysGame::draw_grid(){
    for(int i = 0; i < grid_size.y; i++)
        for(int j = 0; j < grid_size.x; j++)
            if(grid[i][j].alive)
                DrawRectangle(j * scale, i * scale, scale, scale, get_color(i, j));
}

void ConwaysGame::draw_walls(){
    DrawRectangleLinesEx(
            Rectangle{0, 0, window_size.x, window_size.y},
            2,
            RED
            );
}

void ConwaysGame::draw_help(){
    const int arr_size = 14;
    const int font_size = 45;
    const char* s[arr_size] = {
        "H - Toggle help menu",
        "Click - Toggle tile",
        "Space - step manually",
        "Esc - Close window",
        "L - Toggle loopable walls",
        "C - Clear tiles",
        "I - Invert tiles",
        "R - Randomize tiles",
        "P - Pause/Resume auto-stepping",
        "D - Reset to default values",
        "+/- - Increase/Decrease framerate",
        "[/] - Increase/Decrease scale",
        "S - Cycle through color schemes",
        "1, 2, 3, 4, 5 - Select color Scheme"
    };
    for(int i = 0; i < arr_size; i++)
        DrawText(s[i], (window_size.x - MeasureText(s[i], font_size)) / 2, 15 + font_size * i, font_size, BLACK);
}

void ConwaysGame::draw(){
    draw_grid();
    draw_grid_lines();
    draw_target_fps();
    if(!loop_walls)
        draw_walls();
    if(showing_help){
        draw_help();
    }
}

void ConwaysGame::draw_paused(){
    int font_size = 50;
    int padding = 10;
    const char* s = "Paused";
    int text_width = MeasureText(s, font_size);
    DrawText(s, window_size.x - text_width - padding, padding, font_size, Color{200, 0, 0, 120});
}

void ConwaysGame::draw_target_fps(){
    DrawText(TextFormat("%i FPS target", fps), 5, 5, 20, LIME);
    DrawFPS(5, 25);
}

void ConwaysGame::randomize_grid(){
    for (int i = 0; i < grid_size.y; i++)
        for (int j = 0; j < grid_size.x; j++)
            grid[i][j].alive = rand() % 2;
}

void ConwaysGame::clear_grid(){
    for (int i = 0; i < grid_size.y; i++)
        for (int j = 0; j < grid_size.x; j++)
            grid[i][j].alive = false;
}

void ConwaysGame::invert_grid(){
    for (int i = 0; i < grid_size.y; i++)
        for (int j = 0; j < grid_size.x; j++)
            grid[i][j].alive = !grid[i][j].alive;
}

void ConwaysGame::step(){
    for(int i = 0; i < grid_size.y; i++)
        for(int j = 0; j < grid_size.x; j++)
            count_neighbors(j, i);
    for(int i = 0; i < grid_size.y; i++)
        for(int j = 0; j < grid_size.x; j++)
            grid[i][j].act();
}

void ConwaysGame::count_neighbors(int x, int y){
    for(int i = -1; i <= 1; i++)
        for(int j = -1; j <= 1; j++){
            int xx = x + j;
            int yy = y + i;
            if((xx < 0 || xx >= grid_size.x || yy < 0 || yy >= grid_size.y)){
                if(loop_walls){
                    if(xx < 0)
                        xx += grid_size.x;
                    else if(xx >= grid_size.x)
                        xx -= grid_size.x;
                    if(yy < 0)
                        yy += grid_size.y;
                    else if(yy >= grid_size.y)
                        yy -= grid_size.y;
                }else
                    continue;
            }
            if(grid[yy][xx].alive && (i != 0 || j != 0))
                grid[y][x].neighbors++;
        }
}

void ConwaysGame::toggle_tile(int x, int y){
    x /= scale;
    y /= scale;
    if(y >= 0 && x >= 0 && y < window_size.y && x < window_size.x && grid[y][x].last_toggled + 0.2 <= GetTime()){
        grid[y][x].alive = !grid[y][x].alive;
        grid[y][x].last_toggled = GetTime();
    }
}

void ConwaysGame::keyboard_input(){
    int key = GetKeyPressed();
    switch (key) {
        case 'R':
        case 'r':
            randomize_grid();
            break;
        case 'P':
        case 'p':
            paused = !paused;
            break;
        case ' ':
            step();
            break;
        case 'C':
        case 'c':
            clear_grid();
            break;
        case 'I':
        case 'i':
            invert_grid();
            break;
        case 'S':
        case 's':
            cycle_schemes();
            break;
        case 'L':
        case 'l':
            loop_walls = !loop_walls;
            break;
        case 'H':
        case 'h':
            showing_help = !showing_help;
            break;
        case 'D':
        case 'd':
            reset_to_default();
            break;
        case '+':
            fps += 2;
            SetTargetFPS(fps);
            break;
        case '-':
            fps -= 2;
            if(fps < 1)
                fps = 1;
            SetTargetFPS(fps);
            break;
        case '[':
            scale -= 1;
            if(scale <= 1)
                scale = 1;
            resize_grid();
            break;
        case ']':
            scale += 1;
            resize_grid();
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
            scheme = static_cast<ColorScheme>(key - '1');
            break;
        default:
            break;
    }
}

void ConwaysGame::mouse_input(){
    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonDown(MOUSE_LEFT_BUTTON)){
        toggle_tile(GetMouseX(), GetMouseY());
    }
}

void ConwaysGame::cycle_schemes(){
    int s = scheme + 1;
    scheme = static_cast<ColorScheme>(s);
    if(scheme ==  LOOP_TO_START){
        scheme = BlackAndWhite;
    }
}

void ConwaysGame::reset_to_default(){
    window_size = DEFAULT_WINDOW_SIZE;
    scale = DEFAULT_SCALE;
    fps = DEFAULT_FPS;
    scheme = DEFAULT_SCHEME;
    resize_grid();
    SetTargetFPS(fps);
}

Color ConwaysGame::get_color(int i, int j){
    switch (scheme) {
        case  BlackAndWhite:
            return BLACK;
            break;
        case Rainbow:
            return ColorFromHSV(Vector3{360 * (i / grid_size.y + j / grid_size.x) / 2 ,1, 1});
            break;
        case Red:
            return Color{255 * (i / grid_size.y + j / grid_size.x) / 2 ,0, 0, 255};
            break;
        case Green:
            return Color{0, 255 * (i / grid_size.y + j / grid_size.x) / 2, 0, 255};
            break;
        case Blue:
            return Color{0, 0, 255 * (i / grid_size.y + j / grid_size.x) / 2, 255};
            break;
        default:
            return BLACK;
    }
}

// public

ConwaysGame::ConwaysGame():ConwaysGame(DEFAULT_WINDOW_SIZE.x, DEFAULT_WINDOW_SIZE.y, DEFAULT_SCALE, DEFAULT_FPS){
}

ConwaysGame::ConwaysGame(float scl):ConwaysGame(DEFAULT_WINDOW_SIZE.x, DEFAULT_WINDOW_SIZE.y, scl, DEFAULT_FPS){
}

ConwaysGame::ConwaysGame(float x, float y):ConwaysGame(x, y, DEFAULT_SCALE, DEFAULT_FPS){
}

ConwaysGame::ConwaysGame(float x, float y, float scl):ConwaysGame(x, y, scl, DEFAULT_FPS){
}

ConwaysGame::ConwaysGame(float x, float y, float scl, int f){
    window_size.x = x;
    window_size.y = y;
    scale = scl;
    fps = f;
    srand(time(0));
}

ConwaysGame::~ConwaysGame(){
    for(int i = 0; i < grid_size.y; i++){
        delete grid[i];
    }
    delete grid;
    grid = nullptr;
}

void ConwaysGame::run(){
    InitWindow(window_size.x, window_size.y, "Conway's Game of Life");
    SetTargetFPS(fps);
    create_grid();
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(WHITE);
        draw();
        if(!paused)
            step();
        else
            draw_paused();
        keyboard_input();
        mouse_input();
        EndDrawing();
    }
}
