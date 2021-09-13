#! /bin/bash
Echo building project
g++ SDL_Renderer.cpp $1.cpp -o $1.out -lsdl2 -std=c++17