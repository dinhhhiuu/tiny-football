@echo off
g++ src/main.cpp -o game ^
-I lib/SDL2/include ^
-L lib/SDL2/lib ^
-lmingw32 -lSDL2main -lSDL2

copy lib\SDL2\bin\SDL2.dll .
echo Build done!
