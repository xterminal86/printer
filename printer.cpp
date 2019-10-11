#include "printer.h"

#ifdef USE_SDL

//
// *** https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
//
bool Printer::IsBase64(unsigned char c)
{
  auto res = std::find(kBase64Chars.begin(),
                       kBase64Chars.end(),
                       c);

  return (res != kBase64Chars.end());
}

std::string Printer::Base64_Encode(unsigned char const* bytes_to_encode, unsigned int in_len)
{
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--)
  {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3)
    {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i < 4) ; i++)
      {
        ret += kBase64Chars[char_array_4[i]];
      }

      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
    {
      char_array_3[j] = '\0';
    }

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
    {
      ret += kBase64Chars[char_array_4[j]];
    }

    while((i++ < 3))
    {
      ret += '=';
    }
  }

  return ret;
}

std::string Printer::Base64_Decode(const std::string& encoded_string)
{
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && IsBase64(encoded_string[in_]))
  {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4)
    {
      for (i = 0; i <4; i++)
      {
        char_array_4[i] = kBase64Chars.find(char_array_4[i]);
      }

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
      {
        ret += char_array_3[i];
      }

      i = 0;
    }
  }

  if (i)
  {
    for (j = i; j <4; j++)
    {
      char_array_4[j] = 0;
    }

    for (j = 0; j <4; j++)
    {
      char_array_4[j] = kBase64Chars.find(char_array_4[j]);
    }

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++)
    {
      ret += char_array_3[j];
    }
  }

  return ret;
}

std::vector<unsigned char> Printer::ConvertStringToBytes(const std::string& encodedStr)
{
  std::vector<unsigned char> byteArray;
  for (auto& c : encodedStr)
  {
      byteArray.push_back(c);
  }

  return byteArray;
}

bool Printer::Init(SDL_Renderer* renderer,
                   int windowWidth,
                   int windowHeight,
                   float globalScale)
{
  if (_initialized)
  {
    printf("%s - already initialized!\n", __PRETTY_FUNCTION__);
    return _initialized;
  }

  _rendererRef = renderer;
  _tileWidth = 8;
  _tileHeight = 16;
  _globalScale = globalScale;
  _windowWidth = windowWidth;
  _windowHeight = windowHeight;

  if (InitForSDL(true))
  {
    _initialized = true;
  }

  return _initialized;
}

bool Printer::Init(SDL_Renderer* renderer,
                   const std::string& fontImageFilename,
                   int charImageWidth,
                   int charImageHeight,
                   int windowWidth,
                   int windowHeight,
                   float globalScale)
{
  if (_initialized)
  {
    printf("%s - already initialized!\n", __PRETTY_FUNCTION__);
    return _initialized;
  }

  _rendererRef = renderer;
  _tilesetFilename = fontImageFilename;
  _tileWidth = charImageWidth;
  _tileHeight = charImageHeight;
  _globalScale = globalScale;
  _windowWidth = windowWidth;
  _windowHeight = windowHeight;

  if (InitForSDL(false))
  {
    _initialized = true;
  }

  return _initialized;
}

bool Printer::InitForSDL(bool useEmbeddedTileset)
{
  if (!useEmbeddedTileset)
  {
    SDL_Surface* surf = IMG_Load(_tilesetFilename.data());
    if (surf)
    {
      _tileset = SDL_CreateTextureFromSurface(_rendererRef, surf);
      SDL_FreeSurface(surf);
    }
    else
    {
      printf("***** Could not load tileset %s - %s!\n", _tilesetFilename.data(), SDL_GetError());
      return false;
    }
  }
  else
  {
    auto res = Base64_Decode(kTileset8x16Base64);
    auto bytes = ConvertStringToBytes(res);
    SDL_RWops* data = SDL_RWFromMem(bytes.data(), bytes.size());
    SDL_Surface* surf = IMG_Load_RW(data, 1);
    if (!surf)
    {
      printf("***** Could not load from memory: %s *****\n", IMG_GetError());
      return false;
    }

    _tileset = SDL_CreateTextureFromSurface(_rendererRef, surf);
    SDL_FreeSurface(surf);
  }

  _tileWidthScaled = _tileWidth * _globalScale;
  _tileHeightScaled = _tileHeight * _globalScale;

  _terminalWidth = _windowWidth / _tileWidthScaled;
  _terminalHeight = _windowHeight / _tileHeightScaled;

  int w = 0, h = 0;
  SDL_QueryTexture(_tileset, nullptr, nullptr, &w, &h);

  _tilesetWidth = w;
  _tilesetHeight = h;

  _frameBuffer = SDL_CreateTexture(_rendererRef,
                                   SDL_PIXELFORMAT_RGBA32,
                                   SDL_TEXTUREACCESS_TARGET,
                                   _windowWidth,
                                   _windowHeight);

  char asciiIndex = 0;
  int tileIndex = 0;
  for (int y = 0; y < h; y += _tileHeight)
  {
    for (int x = 0; x < w; x += _tileWidth)
    {
      TileInfo ti;
      ti.X = x;
      ti.Y = y;
      _tiles.push_back(ti);

      _tileIndexByChar[asciiIndex] = tileIndex;

      asciiIndex++;
      tileIndex++;
    }
  }

  return true;
}
#else
void Printer::Init()
{
  InitForCurses();
}
#endif

#ifdef USE_SDL
void Printer::DrawWindow(const Position& leftCorner,
                         const Position& size,
                         const std::string& header,
                         const std::string& headerFgColor,
                         const std::string& headerBgColor,
                         const std::string& borderColor,
                         const std::string& borderBgColor,
                         const std::string& bgColor,
                         int variant)
{
  auto res = GetPerimeter(leftCorner.X, leftCorner.Y,
                          size.X, size.Y, true);

  int x = leftCorner.X;
  int y = leftCorner.Y;

  int ulCorner;
  int urCorner;
  int dlCorner;
  int drCorner;
  int hBarU;
  int hBarD;
  int vBarL;
  int vBarR;

  if (variant == 0)
  {
    ulCorner = CP437IndexByType[NameCP437::ULCORNER_2];
    urCorner = CP437IndexByType[NameCP437::URCORNER_2];
    dlCorner = CP437IndexByType[NameCP437::DLCORNER_2];
    drCorner = CP437IndexByType[NameCP437::DRCORNER_2];
    hBarU = CP437IndexByType[NameCP437::HBAR_2];
    hBarD = CP437IndexByType[NameCP437::HBAR_2];
    vBarL = CP437IndexByType[NameCP437::VBAR_2];
    vBarR = CP437IndexByType[NameCP437::VBAR_2];
  }
  else
  {
    ulCorner = CP437IndexByType[NameCP437::ULCORNER_3];
    urCorner = CP437IndexByType[NameCP437::URCORNER_3];
    dlCorner = CP437IndexByType[NameCP437::DLCORNER_3];
    drCorner = CP437IndexByType[NameCP437::DRCORNER_3];
    hBarU = CP437IndexByType[NameCP437::HBAR_3U];
    hBarD = CP437IndexByType[NameCP437::HBAR_3D];
    vBarL = CP437IndexByType[NameCP437::VBAR_3L];
    vBarR = CP437IndexByType[NameCP437::VBAR_3R];
  }

  // Fill background

  for (int i = x + 1; i < x + size.X; i++)
  {
    for (int j = y + 1; j < y + size.Y; j++)
    {
      PrintFB(i, j, ' ', "#000000", bgColor);
    }
  }

  // Corners

  PrintFB(x, y, ulCorner, borderColor, borderBgColor);
  PrintFB(x + size.X, y, urCorner, borderColor, borderBgColor);
  PrintFB(x, y + size.Y, dlCorner, borderColor, borderBgColor);
  PrintFB(x + size.X, y + size.Y, drCorner, borderColor, borderBgColor);

  // Horizontal bars

  for (int i = x + 1; i < x + size.X; i++)
  {
    PrintFB(i, y, hBarU, borderColor, borderBgColor);
    PrintFB(i, y + size.Y, hBarD, borderColor, borderBgColor);
  }

  // Vertical bars

  for (int i = y + 1; i < y + size.Y; i++)
  {
    PrintFB(x, i, vBarL, borderColor, borderBgColor);
    PrintFB(x + size.X, i, vBarR, borderColor, borderBgColor);
  }

  if (header.length() != 0)
  {
    std::string lHeader = header;
    lHeader.insert(0, " ");
    lHeader.append(" ");

    int stringPixelWidth = (lHeader.length() * _tileWidthScaled);
    int headerPosX = x * _tileWidthScaled;
    int headerPosY = y * _tileHeightScaled;

    // size.X actually gives + 1 (see corners section of PrintFBs above),
    // so if size.X = 4 it means span from x to x + 4 end point.
    // Thus, we either must align to string.length() / 2, or
    // adjust header's position by adding additional _tileWidthScaled / 2,
    // depending on size.X
    //
    // It's too long to explain thoroughly,
    // just open some graphics editor and see for yourself.

    if (size.X % 2 != 0)
    {
      int toAdd = ((size.X + 1) / 2) * _tileWidthScaled;
      headerPosX += toAdd;
      headerPosX -= stringPixelWidth / 2;
    }
    else
    {
      int toAdd = (size.X / 2) * _tileWidthScaled;
      headerPosX += toAdd;
      headerPosX -= stringPixelWidth / 2;
      headerPosX += _tileWidthScaled / 2;
    }

    for (auto& c : lHeader)
    {
      TileColor tc = ConvertHtmlToRGB(headerBgColor);
      SDL_SetTextureColorMod(_tileset, tc.R, tc.G, tc.B);
      DrawTile(headerPosX, headerPosY, 219);

      tc = ConvertHtmlToRGB(headerFgColor);
      SDL_SetTextureColorMod(_tileset, tc.R, tc.G, tc.B);
      DrawTile(headerPosX, headerPosY, c);

      headerPosX += _tileWidthScaled;
    }
  }
}

void Printer::DrawImage(const int& x, const int& y, const int& w, const int& h, SDL_Texture* tex)
{
  int tw, th;
  SDL_QueryTexture(tex, nullptr, nullptr, &tw, &th);

  SDL_Rect src;
  src.x = 0;
  src.y = 0;
  src.w = tw;
  src.h = th;

  SDL_Rect dst;
  dst.x = x;
  dst.y = y;
  dst.w = w;
  dst.h = h;

  if (SDL_GetRenderTarget(_rendererRef) == nullptr)
  {
    SDL_SetRenderTarget(_rendererRef, _frameBuffer);
  }

  SDL_RenderCopy(_rendererRef, tex, &src, &dst);
}

void Printer::DrawImage(const int& x, const int& y, SDL_Texture* tex)
{
  int tw, th;
  SDL_QueryTexture(tex, nullptr, nullptr, &tw, &th);

  SDL_Rect src;
  src.x = 0;
  src.y = 0;
  src.w = tw;
  src.h = th;

  SDL_Rect dst;
  dst.x = x;
  dst.y = y;
  dst.w = tw;
  dst.h = th;

  if (SDL_GetRenderTarget(_rendererRef) == nullptr)
  {
    SDL_SetRenderTarget(_rendererRef, _frameBuffer);
  }

  SDL_RenderCopy(_rendererRef, tex, &src, &dst);
}

void Printer::DrawTile(int x, int y, int tileIndex)
{
  TileInfo& tile = _tiles[tileIndex];

  SDL_Rect src;
  src.x = tile.X;
  src.y = tile.Y;
  src.w = _tileWidth;
  src.h = _tileHeight;

  SDL_Rect dst;
  dst.x = x;
  dst.y = y;
  dst.w = _tileWidthScaled;
  dst.h = _tileHeightScaled;

  if (SDL_GetRenderTarget(_rendererRef) == nullptr)
  {
    SDL_SetRenderTarget(_rendererRef, _frameBuffer);
  }

  SDL_RenderCopy(_rendererRef, _tileset, &src, &dst);
}

void Printer::PrintFB(const int& x, const int& y,
                      int image,
                      const std::string& htmlColorFg,
                      const std::string& htmlColorBg)
{
  TileColor tc;

  int tileIndex = image;

  int posX = x * _tileWidthScaled;
  int posY = y * _tileHeightScaled;

  if (htmlColorBg.length() != 0)
  {
    tc = ConvertHtmlToRGB(htmlColorBg);
    SDL_SetTextureColorMod(_tileset, tc.R, tc.G, tc.B);
    DrawTile(posX, posY, 219);
  }

  tc = ConvertHtmlToRGB(htmlColorFg);
  SDL_SetTextureColorMod(_tileset, tc.R, tc.G, tc.B);
  DrawTile(posX, posY, image);
}

void Printer::PrintFB(const int& x, const int& y,
                      const std::string& text,
                      int align,
                      const std::string& htmlColorFg,
                      const std::string& htmlColorBg)
{
  int px = x * _tileWidthScaled;
  int py = y * _tileHeightScaled;

  switch (align)
  {
    case kAlignCenter:
    {
      int pixelWidth = text.length() * _tileWidthScaled;
      px -= pixelWidth / 2;      
    }
    break;

    case kAlignRight:
    {
      int pixelWidth = text.length() * _tileWidthScaled;
      px -= pixelWidth;
    }
    break;
  }

  for (auto& c : text)
  {
    TileColor tc;

    if (htmlColorBg.length() != 0)
    {
      tc = ConvertHtmlToRGB(htmlColorBg);
      SDL_SetTextureColorMod(_tileset, tc.R, tc.G, tc.B);
      DrawTile(px, py, 219);
    }

    tc = ConvertHtmlToRGB(htmlColorFg);
    SDL_SetTextureColorMod(_tileset, tc.R, tc.G, tc.B);
    DrawTile(px, py, c);

    px += _tileWidthScaled;
  }
}

TileColor Printer::ConvertHtmlToRGB(const std::string& htmlColor)
{
  TileColor res;

  std::string hexR = { htmlColor[1], htmlColor[2] };
  std::string hexG = { htmlColor[3], htmlColor[4] };
  std::string hexB = { htmlColor[5], htmlColor[6] };

  int valueR = strtol(hexR.data(), nullptr, 16);
  int valueG = strtol(hexG.data(), nullptr, 16);
  int valueB = strtol(hexB.data(), nullptr, 16);

  res.R = valueR;
  res.G = valueG;
  res.B = valueB;

  return res;
}
#endif

#ifndef USE_SDL
void Printer::InitForCurses()
{
  int mx = 0;
  int my = 0;

  getmaxyx(stdscr, my, mx);

  _terminalWidth = mx;
  _terminalHeight = my;

  // Enforce colors of standard ncurses colors
  // because some colors aren't actually correspond to their
  // "names", e.g. COLOR_BLACK isn't actually black, but grey,
  // so we redefine it.

  init_color(COLOR_BLACK, 0, 0, 0);
  init_color(COLOR_WHITE, 1000, 1000, 1000);
  init_color(COLOR_RED, 1000, 0, 0);
  init_color(COLOR_GREEN, 0, 1000, 0);
  init_color(COLOR_BLUE, 0, 0, 1000);
  init_color(COLOR_CYAN, 0, 1000, 1000);
  init_color(COLOR_MAGENTA, 1000, 0, 1000);
  init_color(COLOR_YELLOW, 1000, 1000, 0);

  PrepareFrameBuffer();
}

bool Printer::ContainsColorMap(size_t hashToCheck)
{
  for (auto& h : _colorMap)
  {
    if (h.first == hashToCheck)
    {
      return true;
    }
  }

  return false;
}

bool Printer::ColorIndexExists(size_t hashToCheck)
{
  for (auto& h : _colorIndexMap)
  {
    if (h.first == hashToCheck)
    {
      return true;
    }
  }

  return false;
}

NColor Printer::GetNColor(const std::string& htmlColor)
{
  NColor ret;

  std::string hexR = { htmlColor[1], htmlColor[2] };
  std::string hexG = { htmlColor[3], htmlColor[4] };
  std::string hexB = { htmlColor[5], htmlColor[6] };

  int valueR = strtol(hexR.data(), nullptr, 16);    
  int valueG = strtol(hexG.data(), nullptr, 16);    
  int valueB = strtol(hexB.data(), nullptr, 16);    

  // ncurses color component has range from 0 to 1000

  int scaledValueR = (valueR / 255.0f) * 1000;
  int scaledValueG = (valueG / 255.0f) * 1000;
  int scaledValueB = (valueB / 255.0f) * 1000;
  
  //printf("%s %s %s => %i %i %i\n", hexR.data(), hexG.data(), hexB.data(), c.R, c.G, c.B);

  ret.R = scaledValueR;
  ret.G = scaledValueG;
  ret.B = scaledValueB;

  return ret;
}

size_t Printer::GetOrSetColor(const std::string& htmlColorFg, const std::string& htmlColorBg)
{
  std::string composition = htmlColorFg + htmlColorBg;
  std::hash<std::string> hasher;

  size_t hash = hasher(composition);

  if (!ContainsColorMap(hash))
  {
    auto fg = GetNColor(htmlColorFg);
    auto bg = GetNColor(htmlColorBg);

    short hashFg = hasher(htmlColorFg);
    short hashBg = hasher(htmlColorBg);

    if (!ColorIndexExists(hashFg))
    {
      fg.ColorIndex = _colorGlobalIndex;
      _colorIndexMap[hashFg] = _colorGlobalIndex++;
      init_color(fg.ColorIndex, fg.R, fg.G, fg.B);
    }
    else
    {
      fg.ColorIndex = _colorIndexMap[hashFg];
    }

    if (!ColorIndexExists(hashBg))
    {
      bg.ColorIndex = _colorGlobalIndex;
      _colorIndexMap[hashBg] = _colorGlobalIndex++;
      init_color(bg.ColorIndex, bg.R, bg.G, bg.B);
    }
    else
    {
      bg.ColorIndex = _colorIndexMap[hashBg];
    }

    ColorPair cp = { _colorPairsGlobalIndex++, fg, bg };
    _colorMap[hash] = cp;

    init_pair(_colorMap[hash].PairIndex, _colorIndexMap[hashFg], _colorIndexMap[hashBg]);
  }

  return hash;
}

std::pair<int, int> Printer::AlignText(int x, int y, int align, const std::string& text)
{
  std::pair<int, int> res;

  int tx = x;
  int ty = y;

  switch (align)
  {
    case kAlignRight:
      // We have to compensate for new position after shift.
      //
      // E.g., print (80, 10, kAlignRight, "Bees")
      // will start from 76 position (tx -= text.length())
      // so it will actually end at 76 (B), 77 (e), 78 (e), 79 (s)
      // This way we either should not subtract 1 from TerminalWidth
      // when printing right aligned text at the end of the screen,
      // or make this hack.
      tx++;

      tx -= text.length();
      break;

    case kAlignCenter:
      tx -= text.length() / 2;
      break;

    // Defaulting to left alignment
    default:
      break;
  }

  res.first = ty;
  res.second = tx;

  return res;
}

void Printer::DrawWindow(const Position& leftCorner,
                         const Position& size,
                         const std::string& header,
                         const std::string& headerFgColor,
                         const std::string& headerBgColor,
                         const std::string& borderColor,
                         const std::string& borderBgColor,
                         const std::string& bgColor)
{
  int x = leftCorner.X;
  int y = leftCorner.Y;

  // Fill background

  for (int i = x + 1; i < x + size.X; i++)
  {
    for (int j = y + 1; j < y + size.Y; j++)
    {
      PrintFB(i, j, ' ', "#000000", bgColor);
    }
  }

  // Corners

  PrintFB(x, y, '+', borderColor, borderBgColor);
  PrintFB(x + size.X, y, '+', borderColor, borderBgColor);
  PrintFB(x, y + size.Y, '+', borderColor, borderBgColor);
  PrintFB(x + size.X, y + size.Y, '+', borderColor, borderBgColor);

  // Horizontal bars

  for (int i = x + 1; i < x + size.X; i++)
  {
    PrintFB(i, y, '-', borderColor, borderBgColor);
    PrintFB(i, y + size.Y, '-', borderColor, borderBgColor);
  }

  // Vertical bars

  for (int i = y + 1; i < y + size.Y; i++)
  {
    PrintFB(x, i, '|', borderColor, borderBgColor);
    PrintFB(x + size.X, i, '|', borderColor, borderBgColor);
  }

  if (header.length() != 0)
  {
    std::string lHeader = header;
    lHeader.insert(0, " ");
    lHeader.append(" ");

    int headerPosX = ((size.X - x) / 2) - lHeader.length() / 2;
    int headerPosY = y;

    for (auto& c : lHeader)
    {
      PrintFB(headerPosX, headerPosY, c, headerFgColor, headerBgColor);

      headerPosX++;
    }
  }
}

void Printer::Print(const int& x, const int& y, const std::string& text, int align, const std::string& htmlColorFg, const std::string& htmlColorBg)
{
  size_t hash = GetOrSetColor(htmlColorFg, htmlColorBg);
  auto textPos = AlignText(x, y, align, text);

  attron(COLOR_PAIR(_colorMap[hash].PairIndex));
  mvprintw(textPos.first, textPos.second, text.data());
  attroff(COLOR_PAIR(_colorMap[hash].PairIndex));   
}

void Printer::Print(const int& x, const int& y, const int& ch, const std::string& htmlColorFg, const std::string& htmlColorBg)
{
  size_t hash = GetOrSetColor(htmlColorFg, htmlColorBg);

  attron(COLOR_PAIR(_colorMap[hash].PairIndex));
  mvaddch(y, x, ch);
  attroff(COLOR_PAIR(_colorMap[hash].PairIndex));
}

void Printer::PrintFB(const int& x, const int& y,
                                 const int& ch,
                                 const std::string& htmlColorFg,
                                 const std::string& htmlColorBg)
{
  if (x < 0 || x > _terminalWidth - 1
   || y < 0 || y > _terminalHeight - 1)
  {
    return;
  }

  // Black & White mode for Windows due to PDCurses not handling colors correctly

  #if !(defined(__unix__) || defined(__linux__))

  std::string tmpFg;
  std::string tmpBg;

  if (htmlColorFg == "#000000" && htmlColorBg == "#000000")
  {
    // For invisible and not discovered tiles

    tmpFg = "#000000";
    tmpBg = "#000000";
  }
  else if (htmlColorBg != "#000000")
  {
    tmpFg = "#000000";
    tmpBg = (htmlColorBg == GlobalConstants::FogOfWarColor) ? GlobalConstants::FogOfWarColor : "#FFFFFF";
  }
  else
  {
    tmpFg = (htmlColorFg == GlobalConstants::FogOfWarColor) ? GlobalConstants::FogOfWarColor : "#FFFFFF";
    tmpBg = "#000000";
  }

  size_t hash = GetOrSetColor(tmpFg, tmpBg);

  #else

  size_t hash = GetOrSetColor(htmlColorFg, htmlColorBg);

  #endif

  _frameBuffer[x][y].Character = ch;
  _frameBuffer[x][y].ColorPairHash = hash;
}

void Printer::PrintFB(const int& x, const int& y,
                                 const std::string& text,
                                 int align,
                                 const std::string& htmlColorFg,
                                 const std::string& htmlColorBg)
{
  auto textPos = AlignText(x, y, align, text);

  int xOffset = 0;
  for (auto& c : text)
  {
    // Coordinates are swapped because
    // in framebuffer we don't work in ncurses coordinate system
    PrintFB(textPos.second + xOffset, textPos.first, c, htmlColorFg, htmlColorBg);
    xOffset++;
  }
}

void Printer::PrepareFrameBuffer()
{
  for (int x = 0; x < _terminalWidth; x++)
  {
    std::vector<FBPixel> row;

    for (int y = 0; y < _terminalHeight; y++)
    {
      FBPixel s;

      s.ColorPairHash = -1;
      s.Character = ' ';

      row.push_back(s);
    }

    _frameBuffer.push_back(row);
  }
}
#endif

void Printer::Clear()
{
#ifndef USE_SDL
  for (int x = 0; x < _terminalWidth; x++)
  {
    for (int y = 0; y < _terminalHeight; y++)
    {
      PrintFB(x, y, ' ', "#000000");
    }
  }
#else
  SDL_SetRenderTarget(_rendererRef, _frameBuffer);
  SDL_RenderClear(_rendererRef);
#endif
}

void Printer::Render()
{
#ifndef USE_SDL
  for (int x = 0; x < _terminalWidth; x++)
  {
    for (int y = 0; y < _terminalHeight; y++)
    {
      attron(COLOR_PAIR(_colorMap[_frameBuffer[x][y].ColorPairHash].PairIndex));
      mvaddch(y, x, _frameBuffer[x][y].Character);
      attroff(COLOR_PAIR(_colorMap[_frameBuffer[x][y].ColorPairHash].PairIndex));
    }
  }

  refresh();
#else
  SDL_SetRenderTarget(_rendererRef, nullptr);
  SDL_RenderClear(_rendererRef);
  SDL_RenderCopy(_rendererRef, _frameBuffer, nullptr, nullptr);
  SDL_RenderPresent(_rendererRef);
#endif
}

std::vector<Position> Printer::GetPerimeter(int x, int y, int w, int h, bool includeCorners)
{
  std::vector<Position> res;

  int x1 = x;
  int x2 = x + w;
  int y1 = y;
  int y2 = y + h;

  for (int i = x1; i <= x2; i++)
  {
    for (int j = y1; j <= y2; j++)
    {
      bool condCorners = (i == x1 && j == y1)
                      || (i == x1 && j == y2)
                      || (i == x2 && j == y1)
                      || (i == x2 && j == y2);

      if (!includeCorners && condCorners)
      {
        continue;
      }

      bool cond = (i == x1 || i == x2 || j == y1 || j == y2);
      if (cond)
      {
        res.push_back(Position(i, j));
      }
    }
  }

  return res;
}
