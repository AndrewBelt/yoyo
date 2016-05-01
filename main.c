#include <SDL2/SDL.h>
#include <stdio.h>
#include "yoyo.h"
#include "mvp.h"

void quit(int retval, SDL_Texture *texture, SDL_Window *window,
          SDL_Renderer *renderer)
{
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_VideoQuit();
  SDL_Quit();
  exit(retval);
}

void print_pixelformat(Uint32 format) {
  if(SDL_PIXELTYPE(format) ==SDL_PIXELTYPE_ARRAYF32) {
    printf("ARRAYF32,");
  } else if(SDL_PIXELTYPE(format) ==SDL_PIXELTYPE_ARRAYF16) {
    printf("ARRAYF16,");
  } else if(SDL_PIXELTYPE(format) ==SDL_PIXELTYPE_ARRAYU32) {
    printf("ARRAYU32,");
  } else if(SDL_PIXELTYPE(format) ==SDL_PIXELTYPE_ARRAYU16) {
    printf("ARRAYU16,");
  } else if(SDL_PIXELTYPE(format) ==SDL_PIXELTYPE_ARRAYU8) {
    printf("ARRAYU8,");
  } else if(SDL_PIXELTYPE(format) ==SDL_PIXELTYPE_PACKED32) {
    printf("PACKED32,");
  } else if(SDL_PIXELTYPE(format) ==SDL_PIXELTYPE_PACKED16) {
    printf("PACKED16,");
  } else if(SDL_PIXELTYPE(format) ==SDL_PIXELTYPE_PACKED8) {
    printf("PACKED8,");
  } else if(SDL_PIXELTYPE(format) ==SDL_PIXELTYPE_INDEX8) {
    printf("INDEX8,");
  } else if(SDL_PIXELTYPE(format) ==SDL_PIXELTYPE_INDEX4) {
    printf("INDEX4,");
  } else if(SDL_PIXELTYPE(format) ==SDL_PIXELTYPE_INDEX1) {
    printf("INDEX1,");
  }

  if(format & SDL_PACKEDORDER_BGRA) {
    printf("BGRA,");
  } else if(SDL_PIXELORDER(format) ==SDL_PACKEDORDER_ABGR) {
    printf("ABGR,");
  } else if(SDL_PIXELORDER(format) ==SDL_PACKEDORDER_BGRX) {
    printf("BGRX,");
  } else if(SDL_PIXELORDER(format) ==SDL_PACKEDORDER_XBGR) {
    printf("XBGR,");
  } else if(SDL_PIXELORDER(format) ==SDL_PACKEDORDER_RGBA) {
    printf("RGBA,");
  } else if(SDL_PIXELORDER(format) ==SDL_PACKEDORDER_ARGB) {
    printf("ARGB,");
  } else if(SDL_PIXELORDER(format) ==SDL_PACKEDORDER_RGBX) {
    printf("RGBX,");
  } else if(SDL_PIXELORDER(format) ==SDL_PACKEDORDER_XRGB) {
    printf("XRGB,");
  } else if(SDL_PIXELORDER(format) ==SDL_PACKEDORDER_NONE) {
    printf("None,");
  }

  if(SDL_PIXELLAYOUT(format) ==SDL_PACKEDLAYOUT_1010102) {
    printf("1010102.\n");
  } else if(SDL_PIXELLAYOUT(format) ==SDL_PACKEDLAYOUT_2101010) {
    printf("2101010.\n");
  } else if(SDL_PIXELLAYOUT(format) ==SDL_PACKEDLAYOUT_8888) {
    printf("8888.\n");
  } else if(SDL_PIXELLAYOUT(format) ==SDL_PACKEDLAYOUT_565) {
    printf("565.\n");
  } else if(SDL_PIXELLAYOUT(format) ==SDL_PACKEDLAYOUT_5551) {
    printf("5551.\n");
  } else if(SDL_PIXELLAYOUT(format) ==SDL_PACKEDLAYOUT_1555) {
    printf("1555.\n");
  } else if(SDL_PIXELLAYOUT(format) ==SDL_PACKEDLAYOUT_4444) {
    printf("4444.\n");
  } else if(SDL_PIXELLAYOUT(format) ==SDL_PACKEDLAYOUT_332) {
    printf("332.\n");
  }
}

int main()
{
  /* yoyo width and height, renderer width and height */
  /* yoyo x and y position */
  int yw, yh, rw, rh, i, pitch, x, y;
  unsigned int *bytes;

  /* Angle at which rotated */
  double theta = 0.0, dt = 1 / 60.0;

  /* Initialize SDL */
  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "SDL could not initialize! Error: %s.\n",
            SDL_GetError());
    exit(1);
  }


  /* Initialize the window */
  SDL_Window *window = SDL_CreateWindow("SDL Tutorial",
                                    SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED,
                                    100, 100,
                                    SDL_WINDOW_OPENGL |
                                    SDL_WINDOW_RESIZABLE);
  if(window == NULL) {
    fprintf(stderr, "Window could not be created! Error: %s.\n",
            SDL_GetError());
    exit(1);
  }

  /* Initialize the renderer */
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
                                    SDL_RENDERER_ACCELERATED |
                                    SDL_RENDERER_PRESENTVSYNC);
  if(renderer == NULL) {
    fprintf(stderr,
            "Renderer could not be initialized! Error: %s.\n",
            SDL_GetError());
    exit(1);
  }

  /* Read in the image (a circle with transparency) */
  Uint32 format;
  int w, h, access;
  SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888,
                                           SDL_TEXTUREACCESS_STREAMING,
                                           data.width, data.height);
  if(texture == NULL) {
    fprintf(stderr,
            "Texture could not be initialized! Error: %s.\n",
            SDL_GetError());
    exit(1);
  }

  /* Figure out which pixel mode they're using */
  SDL_QueryTexture(texture, &format, &access, &w, &h);
  print_pixelformat(format);


  if(SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND) < 0) {
    fprintf(stderr, "Texture blend mode could not be set! Error: %s.\n",
            SDL_GetError());
    exit(1);
  }
  if(SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND) < 0) {
    fprintf(stderr, "Texture blend mode could not be set! Error: %s.\n",
            SDL_GetError());
    exit(1);
  }

  /* Fill in the texture with the data */
  SDL_LockTexture(texture, NULL, (void **)&bytes, &pitch);
  memcpy(bytes, &data.pixel_data, data.height * pitch);
  SDL_UnlockTexture(texture);


  /* Size of the yo-yo in pixels */
  yw = 200;
  yh = 200;

  /* Set the initial yo-yo position */
  SDL_GetWindowSize(window, &rw, &rh);
  Vec pos;
  Vec str_pos1, str_pos2;
  str_pos1.x = -1; str_pos1.y = -1;
  str_pos2.x = -1; str_pos2.y = -1;

  /* Set up the physics shit */
  init();
  SDL_Point *strings = malloc(sizeof(SDL_Point) * N);

  while(1) {
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
      if(e.type == SDL_QUIT) {
        quit(0, texture, window, renderer);
      } else if(e.type == SDL_KEYDOWN) {
        if(e.key.keysym.sym == SDLK_ESCAPE) {
          quit(0, texture, window, renderer);
        }
      } else if(e.type == SDL_MOUSEBUTTONDOWN) {
        printf("Mouse click at %d, %d.\n", e.button.x, e.button.y);
        for(i = 0; i < N - 1; i++) {
          x = strings[i].x; y = strings[i].y;
          if((e.button.x >= x - 10) && (e.button.x <= x + 10) &&
             (e.button.y >= y - 10) && (e.button.y <= y + 10)) {
            printf("Clicked the string.\n");
            break;
          }
        }
        fix_x0((Vec) {(double) e.button.x / rw, (double) e.button.y / rh});
      } else if(e.type == SDL_MOUSEMOTION) {
        if(e.motion.state & SDL_BUTTON_LMASK) {
          /* The user has moved the mouse while the button is pressed */
          fix_x0((Vec) {(double) e.button.x / rw, (double) e.button.y / rh});
        }
      }
    }

    /* Get the location of the yo-yo body */
    step(dt);
    pos = get_X();

    /* Get new window size */
    SDL_GetWindowSize(window, &rw, &rh);

    /* Clear the screen with black */
    SDL_SetRenderDrawColor(renderer,
                           0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);

    /* Get the positions of the lines in the string */
    for(i = 0; i < N - 1; i++) {
      str_pos1 = get_x(i);
      str_pos2 = get_x(i + 1);
      strings[i].x = str_pos1.x * rw; strings[i].y = str_pos1.y * rh;
      strings[i+1].x = str_pos2.x * rw; strings[i+1].y = str_pos2.y * rh;
    }

    /* Draw the yo-yo */
    SDL_Rect rect = {pos.x * rw - (yw / 2),
                     pos.y * rh - (yh / 2), yw, yh};
    SDL_RenderCopyEx(renderer, texture, NULL, &rect,
                     -theta * 180 / M_PI, NULL, SDL_FLIP_NONE);
    /* Draw the string */
    SDL_SetRenderDrawColor(renderer,
                           0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderDrawLines(renderer, strings, N);


    /* Spin dat shit */
    theta += dt * get_omega();

    /* Render everything to the screen */
    SDL_RenderPresent(renderer);
  }

  quit(0, texture, window, renderer);
}
