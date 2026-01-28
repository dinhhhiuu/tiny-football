@echo off

g++ src/main.cpp -o game ^
-I lib/SDL2/include ^
-L lib/SDL2/lib ^
-lmingw32 -lSDL2main -lSDL2 -lSDL2_image

copy lib\SDL2\bin\SDL2.dll .
copy lib\SDL2\bin\SDL2_image.dll .

echo Build done!
pause
