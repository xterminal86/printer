// 11.10.2019 xterminal86
//
// This program shows how to use Printer class library
// with SDL2 or ncurses build.
// Set the preferred library in 'Projects' tab of QT Creator
// or set the 'USE_SDL' flag accordingly manually.
//

#include "printer.h"

Printer _printer;

const float kGlobalScale = 2.0f;

const int kGlyphWidth = 8;
const int kGlyphHeight = 16;

const int kWindowWidth = 80 * kGlyphWidth * kGlobalScale;
const int kWindowHeight = 24 * kGlyphHeight * kGlobalScale;

#ifdef USE_SDL
const std::string kExitString = "Press 'ESC' to exit";
const std::string khBar       = "===================";
#else
const std::string kExitString = "Press 'q' to exit";
const std::string khBar       = "=================";
#endif

void Display()
{
  // Printer::Clear() should be called before any drawing
  _printer.Clear();

  // Do any drawing related stuff here

  int tw = _printer.TerminalWidth();
  int th = _printer.TerminalHeight();

  std::string dims = std::to_string(tw) + "x" + std::to_string(th);

  _printer.PrintFB(tw - 1, th - 1, dims, Printer::kAlignRight, "#FFFFFF");

  _printer.PrintFB(40, 0, kExitString, Printer::kAlignCenter, "#FFFFFF");
  _printer.PrintFB(40, 1, khBar, Printer::kAlignCenter, "#FFFFFF");

  _printer.PrintFB(40, 4, '|', "#FFFFFF");
  _printer.PrintFB(39, 5, '\\', "#FFFFFF");
  _printer.PrintFB(41, 5, '/', "#FFFFFF");

  _printer.PrintFB(40, 6, "Left Align", Printer::kAlignLeft, "#FFFFFF");
  _printer.PrintFB(40, 7, "Center Align", Printer::kAlignCenter, "#FFFFFF");
  _printer.PrintFB(40, 8, "Right Align", Printer::kAlignRight, "#FFFFFF");

  _printer.DrawWindow({ 0, 5 },
                      { 20, 5 },
                      "Header",
                      "#FFFFFF",
                      "#4444FF",
                      "#FFFFFF",
                      "#004400",
                      "#222222");

  // Printer::Render() should be called at the end of all drawing,
  // just like SDL_RenderPresent()
  _printer.Render();
}

#ifdef USE_SDL
SDL_Renderer* _renderer = nullptr;
SDL_Window* _window = nullptr;

bool SDL2()
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    printf("SDL_Init Error: %s\n", SDL_GetError());
    return 1;
  }

  _window = SDL_CreateWindow("Printer test",
                            0, 0,
                            kWindowWidth, kWindowHeight,
                            SDL_WINDOW_SHOWN);

  int drivers = SDL_GetNumRenderDrivers();

  for (int i = 0; i < drivers; i++)
  {
    SDL_RendererInfo info;
    SDL_GetRenderDriverInfo(i, &info);

    _renderer = SDL_CreateRenderer(_window, i, info.flags);
    if (_renderer != nullptr)
    {
      break;
    }
  }

  SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);

  IMG_Init(IMG_INIT_PNG);

  // Initialize the printer class with a reference
  // to SDL_Renderer, font tileset image filename,
  // font character width and height and optional
  // custom global scale which defaults to 1.0
  if (!_printer.Init(_renderer,
                     "standard_8x16.png",
                     kGlyphWidth, kGlyphHeight,
                     kWindowWidth,
                     kWindowHeight,
                     kGlobalScale))
  {
    return 1;
  }

  bool running = true;
  while (running)
  {
    SDL_PumpEvents();

    auto kbState = SDL_GetKeyboardState(nullptr);
    if (kbState[SDL_SCANCODE_ESCAPE])
    {
      running = false;
    }

    Display();
  }

  IMG_Quit();
  SDL_Quit();

  return 0;
}
#else
bool Curses()
{
  initscr();
  nodelay(stdscr, true);
  keypad(stdscr, true);
  noecho();
  curs_set(false);

  start_color();

  _printer.Init();

  char ch;

  while ((ch = getch()) != 'q')
  {
    Display();
  }

  endwin();

  return 0;
}
#endif

int main()
{
  int ret = 0;
  #ifdef USE_SDL
  ret = SDL2();
  #else
  ret = Curses();
  #endif

  return ret;
}
