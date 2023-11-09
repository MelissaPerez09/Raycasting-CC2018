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
  SDL_SetRenderDrawColor(renderer, 150, 136, 105, 255);
  SDL_Rect rect = {
    SCREEN_WIDTH, 
    SCREEN_HEIGHT / 2,
    SCREEN_WIDTH,
    SCREEN_HEIGHT / 2
  };
  SDL_RenderFillRect(renderer, &rect);
}

void showWelcomeScreen() {
  SDL_RenderClear(renderer);
  const std::string WELCOME_IMAGE_KEY = "welcome_image";

  ImageLoader::loadImage(WELCOME_IMAGE_KEY, "assets/welcome.png");
  ImageLoader::render(renderer, WELCOME_IMAGE_KEY, 0, 0);

  SDL_RenderPresent(renderer);

  bool choosingMap = true;
  std::string selectedMap;  // This will store the chosen map

  while (choosingMap) {
    SDL_Event event;
    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
          case SDLK_a:
            selectedMap = "map.txt"; // Set the map for option A
            choosingMap = false; // Exit the loop
            break;
          case SDLK_b:
            selectedMap = "mapB.txt"; // Set the map for option B
            choosingMap = false; // Exit the loop
            break;
        }
      }
    }
  }
}

void showWinScreen() {
  SDL_RenderClear(renderer);
  const std::string WIN_IMAGE_KEY = "win_image";
  
  ImageLoader::loadImage(WIN_IMAGE_KEY, "assets/win.png");
  ImageLoader::render(renderer, WIN_IMAGE_KEY, 0, 0);
  
  SDL_RenderPresent(renderer);

  // Wait for a key press to exit the win screen
  bool waitingForInput = true;
  SDL_Event event;
  while (waitingForInput) {
    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_KEYDOWN) {
        waitingForInput = false; // Exit the loop when a key is pressed
      }
    }
  }
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
  ImageLoader::loadImage("p", "assets/sprite1.png");

  Raycaster r = { renderer };
  r.load_map("assets/map.txt");

  std::thread musicThread(backgroundMusic);
  showWelcomeScreen();

  bool running = true;
  bool win = false;
  bool photo = false;
  int speed = 10;
  Uint32 animationStartTime = 0;
  Uint32 animationDuration = 1000;

  while(running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
        break;
      }
      if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
          case SDLK_LEFT: {
            r.player.a += 3.14 / 24;
            break;
          }
          case SDLK_RIGHT: {
            r.player.a -= 3.14 / 24;
            break;
          }
          case SDLK_UP: {
            int followX = r.player.x - speed * cos(r.player.a);
            int followY = r.player.y - speed * sin(r.player.a);
            if (r.checkWin()) {
              win = true;
            }
            if (!r.collision(followX, followY)) {
              r.player.x = followX;
              r.player.y = followY;
            }
            break;
          }
          case SDLK_DOWN: {
            int followX = r.player.x + speed * cos(r.player.a);
            int followY = r.player.y + speed * sin(r.player.a);
            if (r.checkWin()) {
              win = true;
            }
            if (!r.collision(followX, followY)) {
              r.player.x = followX;
              r.player.y = followY;
            }
            break;
          }
          case SDLK_SPACE: {
          if (!photo) {
            photo = true;
            animationStartTime = SDL_GetTicks(); // Start the animation timer
            playSoundEffect();
          }
          break;
        }
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
    int size = 128;

    if (win) {
      showWinScreen();  // Show the win screen
      break;            // Break out of the loop to stop updating the game
    } else {
      r.render();
      r.render_player();
      if (photo) {
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - animationStartTime >= animationDuration) {
          photo = false;
          playSoundEffect();
        }
        if (currentTime - animationStartTime < animationDuration) {
          ImageLoader::loadImage("p", "assets/sprite2.png");
        } else {
          ImageLoader::loadImage("p", "assets/sprite1.png");
        }
      }
    }

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
