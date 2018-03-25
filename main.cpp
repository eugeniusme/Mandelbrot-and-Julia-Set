#include <fstream>
#include <SDL.h>

void die(const char *msg, const char *error)
{
  printf(msg);
  if (error)
    puts(error);
  exit(1);
}

int window_width = 800, window_height = 600;
SDL_Window *window = NULL;
SDL_Texture *texture = NULL;
SDL_Renderer *renderer = NULL;
uint32_t *screen = NULL;

void cleanup()
{
  if (screen)
    delete [] screen;
  if (texture)
    SDL_DestroyTexture(texture);
  if (renderer)
    SDL_DestroyRenderer(renderer);
  if (window)
    SDL_DestroyWindow(window);
}

void reset_screen()
{
  cleanup();

  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    die("Failed to initialize SDL", SDL_GetError());

  window = SDL_CreateWindow("Kursach", SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_RESIZABLE);
  if (!window)
    die("Failed to create window", SDL_GetError());

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer)
    die("Failed to create renderer", SDL_GetError());

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
      SDL_TEXTUREACCESS_STREAMING, window_width, window_height);

  screen = new uint32_t [window_width * window_height];
}

void write_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
  if (x < 0 || x >= window_width || y < 0 || y >= window_height)
    die("Attempt at writing to screen out of its bounds", NULL);
  screen[y * window_width + x] = (r << 24) + (g << 16) + (b << 8);
}

void draw_screen() {
  SDL_UpdateTexture(texture, NULL, screen, window_width * sizeof(uint32_t));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

double map(double value, double from_min, double from_max, double to_min,
    double to_max)
{
  return (to_min + ((to_max - to_min) / (from_max - from_min))
      * (value - from_min));
}

int main(int argc, char **args)
{
  reset_screen();

  bool running = true;
  SDL_Event event;
  while (running) {
    while (SDL_PollEvent(&event) != 0)
      if (event.type == SDL_QUIT)
        running = false;
      else if (event.type == SDL_WINDOWEVENT)
        if (event.window.event == SDL_WINDOWEVENT_RESIZED
            || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
          window_width = event.window.data1;
          window_height = event.window.data2;
          reset_screen();
        }

    // https://en.wikipedia.org/wiki/Mandelbrot_set#Escape_time_algorithm

    uint32_t colors[50];
    for (int i = 0; i < 50; i++)
      colors[i] = map(i+1, 1, 50, 0, 255);

    for (int py = 0; py < window_height; py++) {
      const double y0 = map(py, 0, window_height, -1.25, 1.25);
      for (int px = 0; px < window_width; px++) {
        const double x0 = map(px, 0, window_width, -2.5, 1);
        double x = 0.0, y = 0.0;
        int iteration = 0;
        while (x*x + y*y < 2*2 && iteration < 500) {
          const double xt = x*x - y*y + x0;
          y = 2 * x*  y + y0;
          x = xt;
          iteration++;
        }
        write_pixel(px, py, colors[iteration], colors[iteration], colors[iteration]);
      }
    }

    draw_screen();
  }

  cleanup();
  SDL_Quit();
  return 0;
}
