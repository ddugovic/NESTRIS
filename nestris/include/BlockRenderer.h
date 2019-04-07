#ifndef BLOCKRENDERER_H
#define BLOCKRENDERER_H
#include "SDL.h"
#include "enums.h"

class BlockRenderer
{
    public:
        BlockRenderer();
        static void block(SDL_Surface* renderSurface, const nes_uchar& blocktype, const size_t& level, const size_t& x, const size_t& y);
    protected:

    private:
        static unsigned char colors[10][4];
        static unsigned int palette[4][16];
};

#endif // BLOCKRENDERER_H