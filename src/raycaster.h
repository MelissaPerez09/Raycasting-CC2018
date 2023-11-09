#pragma once

#include "print.h"
#include <iostream>
#include <fstream>
#include <SDL_render.h>
#include <string>
#include <vector>
#include <cmath>
#include <SDL.h>
#include <unordered_map>
#include "color.h"
#include "imageloader.h"


const Color B = {0, 0, 0};
const Color W = {255, 255, 255};

//const int WIDTH = 8;
//const int HEIGHT = 10;
const int WIDTH = 16;
const int HEIGHT = 11;
const int BLOCK = 50;
const int SCREEN_WIDTH = WIDTH * BLOCK;
const int SCREEN_HEIGHT = HEIGHT * BLOCK;


struct Player {
  int x;
  int y;
  float a;
  float fov;
}; 

struct Impact {
  float d;
  std::string mapHit;  // + | -
  int tx;
};

struct Point {
  int x;
  int y;
};

struct Coordinate {
  int x;
  int y;
};

class Raycaster {
  public:
  std::vector<std::string> map;
  Raycaster(SDL_Renderer* renderer)
    : renderer(renderer) {

    player.x = BLOCK + BLOCK / 2;
    player.y = BLOCK + BLOCK / 2;

    player.a = M_PI / 4.0f;
    player.fov = M_PI / 3.0f;

    scale = 50;
    tsize = 128;
  }

  void load_map(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    while (getline(file, line)) {
      map.push_back(line);
    }
    file.close();
  }

  void point(int x, int y, Color c) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_RenderDrawPoint(renderer, x, y);
  }

  void rect(int x, int y, const std::string& mapHit) {
    for(int cx = x; cx < x + BLOCK; cx++) {
      for(int cy = y; cy < y + BLOCK; cy++) {
        int tx = ((cx - x) * tsize) / BLOCK;
        int ty = ((cy - y) * tsize) / BLOCK;

        Color c = ImageLoader::getPixelColor(mapHit, tx, ty);
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b , 255);
        SDL_RenderDrawPoint(renderer, cx, cy);
      }
    }
  }

  void render_player() {
    // Calculate the position for rendering the player's sprite on the right side
    int rightX = SCREEN_WIDTH + SCREEN_WIDTH / 2; // Center of the right side
    int rightY = SCREEN_HEIGHT / 2; // Center of the screen
    int spriteSize = 128; // Adjust the size as needed

    // Calculate the player's view angle and adjust the sprite angle accordingly
    float spriteAngle = player.a;

    // Adjust the sprite position based on the player's view
    rightX += static_cast<int>(SCREEN_WIDTH / 4 * cos(spriteAngle));
    rightY += static_cast<int>(SCREEN_HEIGHT / 4 * sin(spriteAngle));

    // Render the sprite at the adjusted position and angle
    ImageLoader::render(renderer, "p", rightX - spriteSize / 2, rightY - spriteSize / 2, spriteSize);
  }

  Impact cast_ray(float a) {
    float d = 0;
    std::string mapHit;
    int tx;

    while(true) {
      int x = static_cast<int>(player.x + d * cos(a)); 
      int y = static_cast<int>(player.y + d * sin(a)); 
      
      int i = static_cast<int>(x / BLOCK);
      int j = static_cast<int>(y / BLOCK);


      if (map[j][i] != ' ') {
        mapHit = map[j][i];

        int hitx = x - i * BLOCK;
        int hity = y - j * BLOCK;
        int maxhit;

        if (hitx == 0 || hitx == BLOCK - 1) {
          maxhit = hity;
        } else {
          maxhit = hitx;
        }

        tx = maxhit * tsize / BLOCK;

        break;
      }
     
      point(x, y, W);
      
      d += 1;
    }
    return Impact{d, mapHit, tx};
  }

  void draw_stake(int x, float h, Impact i) {
    float start = SCREEN_HEIGHT/2.0f - h/2.0f;
    float end = start + h;

    for (int y = start; y < end; y++) {
      int ty = (y - start) * tsize / h;
      Color c = ImageLoader::getPixelColor(i.mapHit, i.tx, ty);
      SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

      SDL_RenderDrawPoint(renderer, x, y);
    }
  } 

  float PLAYER_RADIUS = 5.0f;
  std::vector<Coordinate> gCoordinates;

  bool checkWin() {
    // Assuming you have a list of coordinates of all the 'g' characters
    for (int i = 0; i < gCoordinates.size(); i++) {
        int gx = gCoordinates.at(i).x;
        int gy = gCoordinates.at(i).y;

        // If the player is within the range of a 'g' character, return true
        if (std::abs(player.x - gx) <= PLAYER_RADIUS && std::abs(player.y - gy) <= PLAYER_RADIUS) {
            return true;
        }
    }

    // If no 'g' characters are found within the player's radius, return false
    return false;
}

  bool collision(int x, int y) {
    int i = x / BLOCK;
    int j = y / BLOCK;
    return map[j][i] != ' ';
  }
 
  void render() {
    
    // draw left side of the screen
    
    for (int x = 0; x < SCREEN_WIDTH; x += BLOCK) {
      for (int y = 0; y < SCREEN_HEIGHT; y += BLOCK) {
        int i = static_cast<int>(x / BLOCK);
        int j = static_cast<int>(y / BLOCK);
        
        if (map[j][i] != ' ') {
          std::string mapHit;
          mapHit = map[j][i];
          Color c = Color(255, 0, 0);
          rect(x, y, mapHit);
        }
      }
    }

    for (int i = 1; i < SCREEN_WIDTH; i++) {
      float a = player.a + player.fov / 2 - player.fov * i / SCREEN_WIDTH;
      cast_ray(a);
    }

    // draw right side of the screen
    
    for (int i = 1; i < SCREEN_WIDTH; i++) {
      double a = player.a + player.fov / 2.0 - player.fov * i / SCREEN_WIDTH;
      Impact impact = cast_ray(a);
      float d = impact.d;
      Color c = Color(255, 0, 0);

      if (d == 0) {
        print("you lose");
        exit(1);
      }
      int x = SCREEN_WIDTH + i;
      float h = static_cast<float>(SCREEN_HEIGHT)/static_cast<float>(d) * static_cast<float>(scale);
      draw_stake(x, h, impact);
    }

    //left side corner map position
    /*for (int x = 0; x < SCREEN_WIDTH / 3; x += BLOCK / 3) {
        for (int y = 0; y < SCREEN_HEIGHT / 3; y += BLOCK / 3) {
            int i = x / (BLOCK / 3);
            int j = y / (BLOCK / 3);
            if (j < map.size() && i < map[j].size() &&
                map[j][i] != ' ' && map[j][i] != '.') {
                std::string mapHit;
                mapHit = map[j][i];
                rect(x, y, mapHit);
            }
        }
    }*/
  }

  Player player;
  private:
  int scale;
  SDL_Renderer* renderer;
  int tsize;
};
