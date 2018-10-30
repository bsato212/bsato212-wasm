#include <stdio.h>
#include <math.h>
#include <emscripten.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define VIEW_WIDTH 1024
#define VIEW_HEIGHT 600

/**
 * Set of input states
 */
enum input_state {
  NOTHING_PRESSED = 0,
  UP_PRESSED      = 1 << 0,
  DOWN_PRESSED    = 1 << 1,
  LEFT_PRESSED    = 1 << 2,
  RIGHT_PRESSED   = 1 << 3
};

/**
 * Context structure that will be passed to the loop handler
 */
struct context {
  SDL_Renderer *renderer;

  /**
   * Rectangles that the paddle and ball textures will be rendered into
   */
  SDL_Rect paddle_dest;
  SDL_Texture *paddle_tex;
  SDL_Rect ball_dest;
  SDL_Texture *ball_tex;

  /**
   * Font that is used for rendering text, and
   * a texture the text is rendered into
   */
  TTF_Font *font;
  SDL_Texture *text_tex;

  enum input_state active_state;

  /**
   * x and y components of paddle's velocity
   */
  int paddle_vx;
  int paddle_vy;
  int paddle_tex_height;
  int paddle_tex_width;

  /**
   * x and y components of ball's velocity
   */
  int ball_vx;
  int ball_vy;
  int ball_tex_height;
  int ball_tex_width;
};

/**
 * Set the context's text texture to show the text 'text' 
 */
void set_font_text(struct context *ctx, const char *text) {
  SDL_Color fg = {0, 0, 0, 255};
  SDL_Surface *text_surface = TTF_RenderText_Blended(ctx->font, text, fg);
  ctx->text_tex = SDL_CreateTextureFromSurface(ctx->renderer, text_surface);
  SDL_FreeSurface(text_surface);
}

/**
 * Load the font
 */
int get_font_texture(struct context *ctx) {
  ctx->font = TTF_OpenFont("assets/fonts/FreeSans.ttf", 30);
  set_font_text(ctx, "");
  return 1;
}

/**
 * Loads the paddle texture into the context
 */
int get_paddle_texture(struct context * ctx) {
  SDL_Surface *image = IMG_Load("assets/images/paddle.png");
  if (!image) {
    printf("IMG_Load: %s\n", IMG_GetError());
    return 0;
  }

  ctx->paddle_tex = SDL_CreateTextureFromSurface(ctx->renderer, image);
  ctx->paddle_dest.w = image->w;
  ctx->paddle_dest.h = image->h;
  ctx->paddle_tex_width = image->w;
  ctx->paddle_tex_height = image->h;

  SDL_FreeSurface(image);

  return 1;
}

/**
 * Loads the ball texture into the context
 */
int get_ball_texture(struct context * ctx) {
  SDL_Surface *image = IMG_Load("assets/images/ball.png");
  if (!image) {
    printf("IMG_Load: %s\n", IMG_GetError());
    return 0;
  }

  ctx->ball_tex = SDL_CreateTextureFromSurface(ctx->renderer, image);
  ctx->ball_dest.w = image->w;
  ctx->ball_dest.h = image->h;
  ctx->ball_tex_width = image->w;
  ctx->ball_tex_height = image->h;

  SDL_FreeSurface(image);

  return 1;
}

/**
 * Processes the input events and sets the velocity
 * of the paddle accordingly
 */
void process_input(struct context *ctx) {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.key.keysym.sym) {
      case SDLK_UP:
        if (event.key.type == SDL_KEYDOWN) {
          ctx->active_state |= UP_PRESSED;
        } else if (event.key.type == SDL_KEYUP) {
          ctx->active_state ^= UP_PRESSED;
        }
        break;

      case SDLK_DOWN:
        if (event.key.type == SDL_KEYDOWN) {
          ctx->active_state |= DOWN_PRESSED;
        } else if (event.key.type == SDL_KEYUP) {
          ctx->active_state ^= DOWN_PRESSED;
        }
        break;

      case SDLK_LEFT:
          if (event.key.type == SDL_KEYDOWN) {
            ctx->active_state |= LEFT_PRESSED;
          } else if (event.key.type == SDL_KEYUP) {
            ctx->active_state ^= LEFT_PRESSED;
          }
          break;

      case SDLK_RIGHT:
          if (event.key.type == SDL_KEYDOWN) {
            ctx->active_state |= RIGHT_PRESSED;
          } else if (event.key.type == SDL_KEYUP) {
            ctx->active_state ^= RIGHT_PRESSED;
          }
          break;

      default:
          break;
    }
  }

  ctx->paddle_vy = 0;
  ctx->paddle_vx = 0;

  if (ctx->active_state & LEFT_PRESSED) {
    ctx->paddle_vx = -5;
  }
  if (ctx->active_state & RIGHT_PRESSED) {
    ctx->paddle_vx = 5;
  }

  if (ctx->paddle_vx != 0 && ctx->paddle_vy != 0) {
    ctx->paddle_vx /= sqrt(2);
    ctx->paddle_vy /= sqrt(2);
  }
}

/**
 * Loop handler that gets called each animation frame,
 * process the input, update the position of the paddle and 
 * then render the texture
 */
void loop_handler(void *arg) {
  struct context *ctx = arg;
  char text[16];
  process_input(ctx);

  SDL_bool collision = SDL_HasIntersection(&ctx->ball_dest, &ctx->paddle_dest);
  if (collision) {
    ctx->ball_vy *= -1;
  }

  if (ctx->ball_dest.y >= (VIEW_HEIGHT - ctx->ball_tex_height)) {
    ctx->ball_vy *= -1;
  }
  if (ctx->ball_dest.y <= 0) {
    ctx->ball_vy *= -1;
  }
  if (ctx->ball_dest.x >= (VIEW_WIDTH - ctx->ball_tex_width)) {
    ctx->ball_vx *= -1;
  }
  if (ctx->ball_dest.x <= 0) {
    ctx->ball_vx *= -1;
  }

  ctx->paddle_dest.x += ctx->paddle_vx;
  ctx->paddle_dest.y += ctx->paddle_vy;

  ctx->ball_dest.x += ctx->ball_vx;
  ctx->ball_dest.y += ctx->ball_vy;

  SDL_RenderClear(ctx->renderer);
  SDL_RenderCopy(ctx->renderer, ctx->paddle_tex, NULL, &ctx->paddle_dest);
  SDL_RenderCopy(ctx->renderer, ctx->ball_tex, NULL, &ctx->ball_dest);

  sprintf(text, "X: %d Y: %d", ctx->ball_dest.x, ctx->ball_dest.y);
  set_font_text(ctx, text);

  SDL_Rect text_dest = {.x = 0, .y = 0, .w = 0, .h = 0};
  SDL_QueryTexture(ctx->text_tex, NULL, NULL, &text_dest.w, &text_dest.h);
  SDL_RenderCopy(ctx->renderer, ctx->text_tex, NULL, &text_dest);

  SDL_RenderPresent(ctx->renderer);
}

int main() {
  SDL_Window *window;
  struct context ctx;
  int simulate_infinite_loop = 1;
  int fps = 60;

  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();

  SDL_CreateWindowAndRenderer(VIEW_WIDTH, VIEW_HEIGHT, 0, &window, &ctx.renderer);
  SDL_SetRenderDrawColor(ctx.renderer, 255, 255, 255, 255);

  get_paddle_texture(&ctx);
  get_ball_texture(&ctx);
  get_font_texture(&ctx);

  ctx.active_state = NOTHING_PRESSED;
  ctx.paddle_dest.x = VIEW_WIDTH / 2;
  ctx.paddle_dest.y = VIEW_HEIGHT - ctx.paddle_tex_height;
  ctx.paddle_vx = 0;
  ctx.paddle_vy = 0;
  ctx.ball_dest.x = VIEW_WIDTH / 2;
  ctx.ball_dest.y = VIEW_HEIGHT / 2;
  ctx.ball_vx = 5;
  ctx.ball_vy = 5;

  /**
   * Schedule the main loop handler to get 
   * called on each animation frame
   */
  emscripten_set_main_loop_arg(loop_handler, &ctx, fps, simulate_infinite_loop);

  return 0;
}
