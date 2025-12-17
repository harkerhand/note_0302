WAVE_INCLUDE="/home/harkerhand/classes/note_0302/DIP/exp5/1/wavelib/bin/include"
WAVE_LIB_DIR="/home/harkerhand/classes/note_0302/DIP/exp5/1/wavelib/bin/lib"

g++ -std=c++20 denoise.cpp \
    -I$WAVE_INCLUDE \
    -L$WAVE_LIB_DIR \
    -lwavelib -lwauxlib -lm \
    $(pkg-config --cflags --libs opencv4) \
    -o denoise.out

./denoise.out