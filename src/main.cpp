#include <SDL.h>
#include <SDL_events.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <thread>

#include "color.h"
#include "imageloader.h"
#include "raycaster.h"
#include "music.h"

SDL_Window* window;
SDL_Renderer* renderer;

void clear() {
  SDL_SetRenderDrawColor(renderer, 56, 56, 56, 255);
  SDL_RenderClear(renderer);
}

void draw_floor() {
  // floor color
  SDL_SetRenderDrawColor(renderer, 78, 163, 54, 255);
  SDL_Rect rect = {
    SCREEN_WIDTH, 
    SCREEN_HEIGHT / 2,
    SCREEN_WIDTH,
    SCREEN_HEIGHT / 2
  };
  SDL_RenderFillRect(renderer, &rect);
}

int main() {

  SDL_Init(SDL_INIT_VIDEO);
  ImageLoader::init();
  Uint32 lastTime = SDL_GetTicks();
  int frameCount = 0;

  window = SDL_CreateWindow("DOOM", 0, 0, SCREEN_WIDTH * 2, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  ImageLoader::loadImage("+", "assets/wall3.png");
  ImageLoader::loadImage("-", "assets/wall1.png");
  ImageLoader::loadImage("|", "assets/wall2.png");
  ImageLoader::loadImage("*", "assets/wall4.png");
  ImageLoader::loadImage("g", "assets/wall5.png");

  Raycaster r = { renderer };
  r.load_map("assets/map.txt");

  std::thread musicThread(backgroundMusic);

  bool running = true;
  int speed = 10;
  while(running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
        break;
      }
      if (event.type == SDL_KEYDOWN) {
        switch(event.key.keysym.sym ){
          case SDLK_LEFT:
            r.player.a += 3.14/24;
            break;
          case SDLK_RIGHT:
            r.player.a -= 3.14/24;
            break;
          case SDLK_UP:
            r.player.x -= speed * cos(r.player.a);
            r.player.y -= speed * sin(r.player.a);
            break;
          case SDLK_DOWN:
            r.player.x += speed * cos(r.player.a);
            r.player.y += speed * sin(r.player.a);
            break;
            default:
            break;
          }
      }

      if (event.type == SDL_MOUSEMOTION) {
        int mouseX = event.motion.x;
        const float sensitivity = 0.02f;
        r.player.a -= static_cast<float>(mouseX) * sensitivity;
      }
    }

    clear();
    draw_floor();

    r.render();

    frameCount++;
    Uint32 currentTime = SDL_GetTicks();
    Uint32 elapsedTime = currentTime - lastTime;
    if (elapsedTime >= 1000) {
      int fps = frameCount * 1000 / elapsedTime;
      std::string title = "Raycasting - FPS: " + std::to_string(fps);
      SDL_SetWindowTitle(window, title.c_str());
      frameCount = 0;
      lastTime = currentTime;
    }

    SDL_RenderPresent(renderer);
  }

  SDL_DestroyWindow(window);
  SDL_Quit();
}
