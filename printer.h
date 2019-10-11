#ifndef PRINTER_H
#define PRINTER_H

#include <string>
#include <map>
#include <vector>
#include <cstdio>
#include <algorithm>

#ifndef USE_SDL
#include <ncurses.h>
#else
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#endif

///
/// Auxillary structs
///
#ifdef USE_SDL
struct TileColor
{
  int R = 0;
  int G = 0;
  int B = 0;
};

struct TileInfo
{
  int X = 0;
  int Y = 0;
  std::string Data;
};
#else
struct FBPixel
{
  size_t ColorPairHash;
  int Character;
};
#endif

struct NColor
{
  short ColorIndex;
  short R;
  short G;
  short B;
};

struct ColorPair
{
  short PairIndex;
  NColor FgColor;
  NColor BgColor;
};

struct Position
{
  Position() : X(0), Y(0) {}
  Position(int x, int y) : X(x), Y(y) {}
  Position(const Position& from) : X(from.X), Y(from.Y) {}

  void Set(int x, int y)
  {
    X = x;
    Y = y;
  }

  void Set(const Position& from)
  {
    X = from.X;
    Y = from.Y;
  }

  // For use inside std::map as a key
  bool operator< (const Position& rhs) const
  {
    return (X < rhs.X || Y < rhs.Y);
  }

  bool operator== (const Position& rhs) const
  {
    return (X == rhs.X && Y == rhs.Y);
  }

  bool operator!= (const Position& rhs) const
  {
    return (X != rhs.X || Y != rhs.Y);
  }

  // For debug logs
  void Print()
  {
    printf("%s: [%i ; %i]\n", __PRETTY_FUNCTION__, X, Y);
  }

  int X;
  int Y;
};

class Printer
{
  public:
    static const int kAlignLeft = 0;
    static const int kAlignCenter = 1;
    static const int kAlignRight = 2;

    #ifdef USE_SDL
    /// @param[in] renderer Reference to SDL_Renderer.
    /// @param[in] fontImageFilename Font image filename.
    /// @param[in] charImageWidth Font tileset char width in pixels.
    /// @param[in] charImageHeight Font tileset char height in pixels.
    /// @param[in] windowWidth Window created width for framebuffer dimensions calculation.
    /// @param[in] windowHeight Window created height for framebuffer dimensions calculation.
    /// @param[in] globalScale Optional parameter to scale the drawn font.
    bool Init(SDL_Renderer* renderer,
              const std::string& fontImageFilename,
              int charImageWidth,
              int charImageHeight,
              int windowWidth,
              int windowHeight,
              float globalScale = 1.0f);

    bool Init(SDL_Renderer* renderer,
              int windowWidth,
              int windowHeight,
              float globalScale = 1.0f);

    #else
    void Init();
    #endif

    /// Clears framebuffer (ncurses) or renderer (SDL)
    /// Use this before all PrintFB calls
    void Clear();

    /// Prints framebuffer contents to the screen
    /// Call this after all PrintFB calls
    void Render();

    int TerminalWidth() { return _terminalWidth; }
    int TerminalHeight() { return _terminalHeight; }

#ifndef USE_SDL    
    /// Print text at (x, y) directly to the screen,
    /// with (0, 0) at upper left corner and y increases down
    [[deprecated]]
    void Print(const int& x, const int& y,
                const std::string& text,
                int align,
                const std::string& htmlColorFg,
                const std::string& htmlColorBg = "#000000");

    [[deprecated]]
    void Print(const int& x, const int& y,
                const int& ch,
                const std::string& htmlColorFg,
                const std::string& htmlColorBg = "#000000");

    /// Print to "framebuffer" instead of directly to the screen
    void PrintFB(const int& x, const int& y,
                  const int& ch,
                  const std::string& htmlColorFg,
                  const std::string& htmlColorBg = "#000000");

    void PrintFB(const int& x, const int& y,
                  const std::string& text,
                  int align,
                  const std::string& htmlColorFg,
                  const std::string& htmlColorBg = "#000000");

    void DrawWindow(const Position& leftCorner,
                    const Position& size,
                    const std::string& header = std::string{},
                    const std::string& headerFgColor = "#FFFFFF",
                    const std::string& headerBgColor = "#000000",
                    const std::string& borderColor = "#FFFFFF",
                    const std::string& borderBgColor = "#000000",
                    const std::string& bgColor = "#000000");
#else
    void PrintFB(const int& x, const int& y,
                 int image,
                 const std::string& htmlColorFg,
                 const std::string& htmlColorBg = "#000000");

    void PrintFB(const int& x, const int& y,
                 const std::string& text,
                 int align,
                 const std::string& htmlColorFg,
                 const std::string& htmlColorBg = "#000000");

    void DrawImage(const int& x, const int& y, SDL_Texture* tex);
    void DrawImage(const int& x, const int& y, const int& w, const int& h, SDL_Texture* tex);

    void DrawWindow(const Position& leftCorner,
                    const Position& size,
                    const std::string& header = std::string{},
                    const std::string& headerFgColor = "#FFFFFF",
                    const std::string& headerBgColor = "#000000",
                    const std::string& borderColor = "#FFFFFF",
                    const std::string& borderBgColor = "#000000",
                    const std::string& bgColor = "#000000",
                    int variant = 0);

#endif
  private:
    // Width and height of the window in characters
    int _terminalWidth;
    int _terminalHeight;

#ifndef USE_SDL
    bool ContainsColorMap(size_t hashToCheck);
    bool ColorIndexExists(size_t hashToCheck);

    NColor GetNColor(const std::string& htmlColor);
    size_t GetOrSetColor(const std::string& htmlColorFg, const std::string& htmlColorBg);
    std::pair<int, int> AlignText(int x, int y, int align, const std::string& text);

    void PrepareFrameBuffer();

    std::map<size_t, ColorPair> _colorMap;
    std::map<size_t, short> _colorIndexMap;

    short _colorPairsGlobalIndex = 1;
    short _colorGlobalIndex = 8;

    std::vector<std::vector<FBPixel>> _frameBuffer;
#endif
    #ifndef USE_SDL
    void InitForCurses();
    #else

    bool _initialized = false;

    std::vector<TileInfo> _tiles;
    std::map<char, int> _tileIndexByChar;

    SDL_Texture* _tileset = nullptr;
    SDL_Texture* _frameBuffer = nullptr;
    SDL_Renderer* _rendererRef = nullptr;

    std::string _tilesetFilename;

    float _globalScale = 1.0f;

    int _tilesetWidth = 0;
    int _tilesetHeight = 0;

    int _tileWidth = 0;
    int _tileHeight = 0;

    int _tileWidthScaled = 0;
    int _tileHeightScaled = 0;

    int _windowWidth = 0;
    int _windowHeight = 0;

    bool InitForSDL(bool useEmbeddedTileset);

    void DrawTile(int x, int y, int tileIndex);

    TileColor ConvertHtmlToRGB(const std::string& htmlColor);

    const std::string Base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const std::string Tileset8x16Base64 = "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";

    bool IsBase64(unsigned char c);
    std::string Base64_Encode(unsigned char const* bytes_to_encode, unsigned int in_len);
    std::string Base64_Decode(const std::string& encoded_string);
    std::vector<unsigned char> ConvertStringToBytes(const std::string& encodedStr);

    #endif

    std::vector<Position> GetPerimeter(int x, int y, int w, int h, bool includeCorners = true);
};

///
/// Ye olde CP437 glyphs charmap
///
enum class NameCP437
{
  FIRST = 0,
  FACE_1,
  FACE_2,
  HEART,
  DIAMOND,
  CLUB,
  SPADE,
  DOT_1,
  DOT_2,
  SQUARE_1,
  SQUARE_2,
  SIGN_M,
  SIGN_F,
  NOTE_1,
  NOTE_2,
  LAMP,
  RARROW_2,
  LARROW_2,
  UDARROW_1,
  EXCLAIM_DBL,
  PI,
  DOLLAR_1,
  DASH_1,
  UDARROW_2,
  UARROW_1,
  DARROW_1,
  RARROW_1,
  LARROW_1,
  CORNER_SMALL,
  LRARROW,
  UARROW_2,
  DARROW_2,
  ULCORNER_1 = 218,
  DLCORNER_1 = 192,
  URCORNER_1 = 191,
  DRCORNER_1 = 217,
  HBAR_1 = 196,
  VBAR_1 = 179,
  ULCORNER_2 = 201,
  DLCORNER_2 = 200,
  URCORNER_2 = 187,
  DRCORNER_2 = 188,
  HBAR_2 = 205,
  VBAR_2 = 186,
  ULCORNER_3 = 222,
  DLCORNER_3 = 222,
  URCORNER_3 = 221,
  DRCORNER_3 = 221,
  HBAR_3U = 223,
  HBAR_3D = 220,
  VBAR_3L = 222,
  VBAR_3R = 221,
  BLOCK = 219,
  WAVES = 247,
};

static std::map<NameCP437, int> CP437IndexByType =
{
  { NameCP437::FACE_1,       (int)NameCP437::FACE_1       },
  { NameCP437::FACE_2,       (int)NameCP437::FACE_2       },
  { NameCP437::HEART,        (int)NameCP437::HEART        },
  { NameCP437::DIAMOND,      (int)NameCP437::DIAMOND      },
  { NameCP437::CLUB,         (int)NameCP437::CLUB         },
  { NameCP437::SPADE,        (int)NameCP437::SPADE        },
  { NameCP437::DOT_1,        (int)NameCP437::DOT_1        },
  { NameCP437::DOT_2,        (int)NameCP437::DOT_2        },
  { NameCP437::SQUARE_1,     (int)NameCP437::SQUARE_1     },
  { NameCP437::SQUARE_2,     (int)NameCP437::SQUARE_2     },
  { NameCP437::SIGN_M,       (int)NameCP437::SIGN_M       },
  { NameCP437::SIGN_F,       (int)NameCP437::SIGN_F       },
  { NameCP437::NOTE_1,       (int)NameCP437::NOTE_1       },
  { NameCP437::NOTE_2,       (int)NameCP437::NOTE_2       },
  { NameCP437::LAMP,         (int)NameCP437::LAMP         },
  { NameCP437::RARROW_2,     (int)NameCP437::RARROW_2     },
  { NameCP437::LARROW_2,     (int)NameCP437::LARROW_2     },
  { NameCP437::UDARROW_1,    (int)NameCP437::UDARROW_1    },
  { NameCP437::EXCLAIM_DBL,  (int)NameCP437::EXCLAIM_DBL  },
  { NameCP437::PI,           (int)NameCP437::PI           },
  { NameCP437::DOLLAR_1,     (int)NameCP437::DOLLAR_1     },
  { NameCP437::DASH_1,       (int)NameCP437::DASH_1       },
  { NameCP437::UDARROW_2,    (int)NameCP437::UDARROW_2    },
  { NameCP437::UARROW_1,     (int)NameCP437::UARROW_1     },
  { NameCP437::DARROW_1,     (int)NameCP437::DARROW_1     },
  { NameCP437::RARROW_1,     (int)NameCP437::RARROW_1     },
  { NameCP437::LARROW_1,     (int)NameCP437::LARROW_1     },
  { NameCP437::CORNER_SMALL, (int)NameCP437::CORNER_SMALL },
  { NameCP437::LRARROW,      (int)NameCP437::LRARROW      },
  { NameCP437::UARROW_2,     (int)NameCP437::UARROW_2     },
  { NameCP437::DARROW_2,     (int)NameCP437::DARROW_2     },
  { NameCP437::URCORNER_1,   (int)NameCP437::URCORNER_1   },
  { NameCP437::ULCORNER_1,   (int)NameCP437::ULCORNER_1   },
  { NameCP437::DRCORNER_1,   (int)NameCP437::DRCORNER_1   },
  { NameCP437::DLCORNER_1,   (int)NameCP437::DLCORNER_1   },
  { NameCP437::HBAR_1,       (int)NameCP437::HBAR_1       },
  { NameCP437::VBAR_1,       (int)NameCP437::VBAR_1       },
  { NameCP437::URCORNER_2,   (int)NameCP437::URCORNER_2   },
  { NameCP437::ULCORNER_2,   (int)NameCP437::ULCORNER_2   },
  { NameCP437::DRCORNER_2,   (int)NameCP437::DRCORNER_2   },
  { NameCP437::DLCORNER_2,   (int)NameCP437::DLCORNER_2   },
  { NameCP437::HBAR_2,       (int)NameCP437::HBAR_2       },
  { NameCP437::VBAR_2,       (int)NameCP437::VBAR_2       },
  { NameCP437::URCORNER_3,   (int)NameCP437::URCORNER_3   },
  { NameCP437::ULCORNER_3,   (int)NameCP437::ULCORNER_3   },
  { NameCP437::DRCORNER_3,   (int)NameCP437::DRCORNER_3   },
  { NameCP437::DLCORNER_3,   (int)NameCP437::DLCORNER_3   },
  { NameCP437::HBAR_3U,      (int)NameCP437::HBAR_3U      },
  { NameCP437::HBAR_3D,      (int)NameCP437::HBAR_3D      },
  { NameCP437::VBAR_3L,      (int)NameCP437::VBAR_3L      },
  { NameCP437::VBAR_3R,      (int)NameCP437::VBAR_3R      },
  { NameCP437::BLOCK,        (int)NameCP437::BLOCK        },
  { NameCP437::WAVES,        (int)NameCP437::WAVES        }
};

#endif
