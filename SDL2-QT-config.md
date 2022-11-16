If SDL2 is not found during CMake config on Windows in QT Creator, 
manually add these in Projects mode.  

```
SDL2_DIR:PATH=SDL2
SDL2MAIN_LIBRARY:FILEPATH=SDL2/x86_64-w64-mingw32/lib/libSDL2main.a
SDL2_INCLUDE_DIR:PATH=SDL2/x86_64-w64-mingw32/include
SDL2_LIBRARY:FILEPATH=SDL2/x86_64-w64-mingw32/lib/libSDL2.dll.a
```