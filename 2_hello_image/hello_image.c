#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <emscripten.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

/**
 * Loads the image located at 'fileName' and copies it to the
 * renderer 'renderer'
 */
int testImage(SDL_Renderer* renderer, const char* fileName) {
  SDL_Surface *image = IMG_Load(fileName);
  if (!image) {
     printf("IMG_Load: %s\n", IMG_GetError());
     return 0;
  }

  int result = image->w;

  /**
   * position and size that you wish the image to be copied
   * to on the renderer:
   */
  SDL_Rect dest = {.x = 0, .y = 0, .w = 320, .h = 80};

  SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, image);

  SDL_RenderCopy (renderer, tex, NULL, &dest);

  /**
   * Now that the image data is in the renderer, we can free the memory
   * used by the texture and the image surface
   */
  SDL_DestroyTexture (tex);

  SDL_FreeSurface (image);

  return result;
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window *window;
  SDL_Renderer *renderer;

  SDL_CreateWindowAndRenderer(1024, 600, 0, &window, &renderer);

  int result = 0;

  /**
   * Set up a white background
   */
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);

  /**
   * Load and copy the test image to the renderer
   */
  result |= testImage(renderer, "assets/images/paddle.png");

  /**
   * Show what is in the renderer
   */
  SDL_RenderPresent(renderer);

  printf("you should see an image.\n");

  return 0;
}
