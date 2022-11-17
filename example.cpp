//
// 25.10.2022 xterminal86
//
// This program shows how to use Printer class library
// with SDL2 or ncurses build.
// Set the preferred library in 'Projects' tab of QT Creator
// or set the 'USE_SDL' flag accordingly manually.
//
// Under Windows you may have to add manually
// 'Add - File' in QT Creator named 'SDL2_LIBRARY'
// and point it to 'libSDL2.dll.a'. Otherwise you'll get
// undefined references to SDL2 calls.
//
#include "printer.h"

TG::Printer _printer;

const float kGlobalScale = 1.0f;

const int kGlyphWidth  = 8;
const int kGlyphHeight = 16;

const int kWindowWidth  = 80 * kGlyphWidth * kGlobalScale;
const int kWindowHeight = 24 * kGlyphHeight * kGlobalScale;

#ifdef USE_SDL
const std::string kExitString = "Press 'ESC' to exit";
const std::string khBar       = "===================";
const std::string kTrollFace  = "trollface.bmp";
const std::string kPlus       = "plus.bmp";
int _trollface = -1;
int _plus = -1;
int angle = 0;
#else
const std::string kExitString = "Press 'q' to exit";
const std::string khBar       = "=================";
#endif

void Display()
{
  //
  // Printer::Clear() should be called before any drawing.
  //
  _printer.Clear();

  //
  // Do any drawing related stuff here.
  //
  int tw = _printer.TerminalWidth();
  int th = _printer.TerminalHeight();

  std::string dims = std::to_string(tw) + "x" + std::to_string(th);

  _printer.PrintFB(tw - 1,
                   th - 1,
                   dims,
                   TG::Printer::kAlignRight,
                   TG::Colors::White);

  _printer.PrintFB(40, 0, kExitString, TG::Printer::kAlignCenter, TG::Colors::White);
  _printer.PrintFB(40, 1, khBar,       TG::Printer::kAlignCenter, TG::Colors::White);

  _printer.PrintFB(40, 4, '|',  TG::Colors::White);
  _printer.PrintFB(39, 5, '\\', TG::Colors::White);
  _printer.PrintFB(41, 5, '/',  TG::Colors::White);

  _printer.PrintFB(40, 6, "Left Align",   TG::Printer::kAlignLeft,   TG::Colors::White);
  _printer.PrintFB(40, 7, "Center Align", TG::Printer::kAlignCenter, TG::Colors::White);
  _printer.PrintFB(40, 8, "Right Align",  TG::Printer::kAlignRight,  TG::Colors::White);

  _printer.DrawWindow({ 20, 10 },
                      { 40,  6 },
                      "Header",
                      0xFFFFFF,
                      0x4444FF,
                      0xFFFFFF,
                      0x004400,
                      0x222222);

  _printer.PrintFB(40,
                   13,
                   "Hello World!",
                   TG::Printer::kAlignCenter,
                   TG::Colors::White);

  #ifdef USE_SDL
  //
  // Can do images too, but SDL only.
  //
  _printer.PrintFB(70, 5, "Transparency", TG::Printer::kAlignCenter, 0xFFFF00);
  _printer.DrawImage(_plus, { kWindowWidth - 128 - 10, 10, 128, 128 }, angle++);

  _printer.DrawImage(_trollface, { 10, 10, 64, 64 });
  _printer.DrawImage(_trollface, { 84, 10, 64, 64 }, 0, SDL_RendererFlip::SDL_FLIP_HORIZONTAL);
  _printer.DrawImage(_trollface, { 48, 84, 64, 64 }, 0, SDL_RendererFlip::SDL_FLIP_VERTICAL);

  _printer.PrintFB(10, 10, "Images too!", TG::Printer::kAlignCenter, 0x00FF00);
  #endif

  //
  // Printer::Render() should be called at the end of all drawing,
  // just like SDL_RenderPresent()
  //
  _printer.Render();
}

#ifdef USE_SDL
SDL_Renderer* _renderer = nullptr;
SDL_Window* _window     = nullptr;

bool SDL2()
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    printf("SDL_Init Error: %s\n", SDL_GetError());
    return 1;
  }

  _window = SDL_CreateWindow("Printer test",
                            100, 100,
                            kWindowWidth, kWindowHeight,
                            SDL_WINDOW_SHOWN);

  int drivers = SDL_GetNumRenderDrivers();

  for (int i = 0; i < drivers; i++)
  {
    SDL_RendererInfo info;
    SDL_GetRenderDriverInfo(i, &info);

    //
    // This is important since printer uses render to texture
    //
    if (info.flags & SDL_RENDERER_TARGETTEXTURE)
    {
      _renderer = SDL_CreateRenderer(_window, i, info.flags);
      if (_renderer != nullptr)
      {
        break;
      }
    }
  }

  if (_renderer == nullptr)
  {
    printf("Couldn't create renderer: maybe render to texture isn't supported.\n");
    return 1;
  }

  SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);

  //
  // Initialize the printer class with a reference
  // to SDL_Renderer, font tileset image filename,
  // font character width and height and optional
  // custom global scale which defaults to 1.0.
  //
  // Transparent color key is magenta (0xFF, 0x00, 0xFF), hardcoded.
  //

  /*
  if (!_printer.Init(_renderer,
                     "acorn-8x16.bmp",
                     kGlyphWidth, kGlyphHeight,
                     kWindowWidth,
                     kWindowHeight,
                     kGlobalScale))
  {
    return 1;
  }
  */

  //
  // Or you can use embedded tileset.
  //
  if (!_printer.Init(_renderer,
                     kWindowWidth,
                     kWindowHeight,
                     kGlobalScale))
  {
    return 1;
  }

  _trollface = _printer.LoadImage(kTrollFace);
  _plus      = _printer.LoadImage(kPlus, 0xFF00FF);

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

int main(int argc, char* argv[])
{
  std::ignore = argc;
  std::ignore = argv;

  int ret = 0;
  #ifdef USE_SDL
  ret = SDL2();
  #else
  ret = Curses();
  #endif

  return ret;
}
