C:/"Program Files"/Git/mingw32/bin/g++ -O3 -std=c++17 -g -DSFML_STATIC -Iinclude -Ivendor/sfml/include -Ivendor/fmt/include -Ivendor/glm -Ivendor/stb -Ivendor/tinyobjloader -c src/main.cpp -o main.o
C:/"Program Files"/Git/mingw32/bin/g++ -Lvendor/sfml/lib main.o -o run.exe -static-libgcc -static-libstdc++ -lsfml-main -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lopengl32 -lgdi32 -lfreetype -lwinmm
