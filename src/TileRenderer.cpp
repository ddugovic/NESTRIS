#include "TileRenderer.h"
#include"ConsoleManager.h"
#include"enums.h"
#include<string>
#include<fstream>
constexpr size_t itox(const size_t& i, const size_t& width, const size_t& height) {
    return i%width;
}
constexpr size_t itoy(const size_t& i, const size_t& width, const size_t& height) {
    return i/width;
}
constexpr size_t itoxrect(const size_t& i, const size_t& width, const size_t& height, const size_t& rectwidth, const size_t& rectheight) {
    return itox(i,width/rectwidth,height/rectheight)*rectwidth;
}
constexpr size_t itoyrect(const size_t& i, const size_t& width, const size_t& height, const size_t& rectwidth, const size_t& rectheight) {
    //constexpr size_t rectx=itox(i,width/rectwidth,height/rectheight)*rectwidth;
    return itoy(i,width/rectwidth,height/rectheight)*rectheight;
}

constexpr size_t xytoi(const size_t& x, const size_t& y, const size_t& width, const size_t& height) {
    return x+y*width;
}
constexpr size_t xytoi(const size_t& x, const size_t& y, const size_t& width, const size_t& height, const size_t& rectwidth, const size_t& rectheight) {
    return x+y*width;
}
TileRenderer::TileRenderer(const size_t& _width, const size_t& _height, sf::Vector2u _tilesize, const int& _drawmethod)
:tilecont(_width,_height),
width(_width),
height(_height),
drawmethod(_drawmethod),
quadretti(NULL),
tilesize(_tilesize),
texturenumber(0)
{
    if (drawmethod==DRAWSPRITE) {
        quadretti=new uint8container[width*height];
        if (!temptexclass.create(width*tilesize.x,height*tilesize.y)) {
            //error
        }
        tempspriteclass.setTexture(temptexclass, true);
    }
    else if (drawmethod==DRAWIMAGE) {
        finalimageclass.create(width*tilesize.x,height*tilesize.y);
        temptexclass.loadFromImage(finalimageclass);
        tempspriteclass.setTexture(temptexclass, true);
    }
    else if (drawmethod==DRAWVERTEX){
        verteximage=sf::VertexArray(sf::Quads,width*height*4*tilesize.x*tilesize.y);
        for (size_t i=0; i<width*height*4*tilesize.x*tilesize.y; i+=4) {
            verteximage[i].position=sf::Vector2f(itox(i/4,width*tilesize.x,height*tilesize.y),itoy(i/4,width*tilesize.x,height*tilesize.y));
            verteximage[i+1].position=sf::Vector2f(itox(i/4,width*tilesize.x,height*tilesize.y)+1,itoy(i/4,width*tilesize.x,height*tilesize.y));
            verteximage[i+2].position=sf::Vector2f(itox(i/4,width*tilesize.x,height*tilesize.y)+1,itoy(i/4,width*tilesize.x,height*tilesize.y)+1);
            verteximage[i+3].position=sf::Vector2f(itox(i/4,width*tilesize.x,height*tilesize.y),itoy(i/4,width*tilesize.x,height*tilesize.y)+1);
            verteximage[i].color=verteximage[i+1].color=verteximage[i+2].color=verteximage[i+3].color=sf::Color::Black;
        }
    }
    else if (drawmethod==DRAWTEXTURE) { //TODO DYNAMICALLY CHOOSE SIZE && check if size<8
        texturesize=sf::Texture::getMaximumSize()<512?sf::Texture::getMaximumSize():512;
        if (!tiletexture.create(texturesize,texturesize)) {
            //TODO ERROR
        }
        verteximage=sf::VertexArray(sf::Quads,width*height*4);
        for (size_t i=0; i<width; ++i) {
            for (size_t j=0; j<height; ++j) {

                sf::Vertex* quad = &verteximage[(i + j * width) * 4];

                quad[0].position = sf::Vector2f(i * tilesize.x, j * tilesize.y);
                quad[1].position = sf::Vector2f((i + 1) * tilesize.x, j * tilesize.y);
                quad[2].position = sf::Vector2f((i + 1) * tilesize.x, (j + 1) * tilesize.y);
                quad[3].position = sf::Vector2f(i * tilesize.x, (j + 1) * tilesize.y);
            }
        }
    }
}

TileContainer* TileRenderer::getTileContainer() {
    return &tilecont;
}

bool TileRenderer::load(const std::string& tilefile){
    std::unordered_map<Sprite,size_t,std::hash<Sprite>,SpriteEqual> spritemap;
    FILE * Spritefile=fopen(tilefile.c_str(),"r");
    if (!Spritefile) return false;
    size_t spritenumber=0;
    while (!feof(Spritefile)) {
        size_t characters=0;
        Sprite newsprite;
        for (characters=0; characters<tilesize.y*2&&!feof(Spritefile); ++characters) {
            unsigned int hex;
            fscanf(Spritefile,"%x",&hex);
            if (characters<tilesize.y) {
                for (size_t i=0; i<8; ++i)
                newsprite.arr[tilesize.x-i-1][characters]=(hex >> i) & 1U;
            }
            else {
                for (size_t i=0; i<tilesize.y; ++i)
                newsprite.arr[tilesize.x-i-1][characters-8]+=((hex >> i) & 1U)<<1;
            }
        }
        if (characters>=tilesize.y*2) {
            if (glb::spritemode==1) {
                if (spritemap.find(newsprite)==spritemap.end()) {
                    spritemap[newsprite]=spritenumber;
                    spritevector.push_back(newsprite);
                    spritenumber++; //what if pushback fails?
                }
            }
            else if (glb::spritemode==0) {
                if (spritemap.find(newsprite)==spritemap.end()) {
                    spritemap[newsprite]=spritenumber;
                }
                spritevector.push_back(newsprite);
                spritenumber++;
            }
        }
    }
    if (drawmethod==DRAWTEXTURE) add_frequent_textures(); //puts blocks and characters
    return true;
}

size_t TileRenderer::add_or_find_texture(const tiletype& newtile, sf::Image* prerendering=NULL) {
    size_t whereisthistexture=0;
    tiletype temptile=newtile;
    if ((temptile.palette_color[0]&0x0F)>=0x0D) temptile.palette_color[0]=0x0D;
    if ((temptile.palette_color[1]&0x0F)>=0x0D) temptile.palette_color[1]=0x0D;
    if ((temptile.palette_color[2]&0x0F)>=0x0D) temptile.palette_color[2]=0x0D;
    if ((temptile.palette_color[3]&0x0F)>=0x0D) temptile.palette_color[3]=0x0D;

    if (texturemap.find(temptile)==texturemap.end()) { //new texture

        if (itoyrect(texturenumber,texturesize,texturesize,tilesize.x,tilesize.x)>texturesize) {
            glb::cm.update<std::string>("error",std::string("Too many textures"));
            throw texturenumber; //TODOBETTER
        }

        glb::cm.update<std::string>("system",std::string("Creating new texture"));

        if (!prerendering) { //TODO USE OFSTREAM
            fprintf(newtextures,"%d %x %x %x %x\n",temptile.tilenumber,temptile.palette_color[0],temptile.palette_color[1],temptile.palette_color[2],temptile.palette_color[3]);
        }

        uint8container newtexture;
        const int primacifra[4]={
            temptile.palette_color[0]/16,
            temptile.palette_color[1]/16,
            temptile.palette_color[2]/16,
            temptile.palette_color[3]/16
        };
        const int secondacifra[4]={
            temptile.palette_color[0]%16,
            temptile.palette_color[1]%16,
            temptile.palette_color[2]%16,
            temptile.palette_color[3]%16
        };
        const unsigned int color[4]={
            palette[primacifra[0]][secondacifra[0]],
            palette[primacifra[1]][secondacifra[1]],
            palette[primacifra[2]][secondacifra[2]],
            palette[primacifra[3]][secondacifra[3]]
        };
        sf::Uint8 rgba[4][4];
        for (size_t colorcounter=0; colorcounter<4;++colorcounter) {
            int colortemp=color[colorcounter];
            for (size_t rgbacounter=0; rgbacounter<4;++rgbacounter) {
                rgba[colorcounter][4-rgbacounter-1]=colortemp&0xff;
                colortemp=colortemp>>8;
            }
        }
        Sprite* spritetemp=&spritevector[temptile.tilenumber];
        if (!prerendering) {
            sf::Uint8* tempquadretto=newtexture.getQuadretto(); //quadretto means small square in Italian
            for (size_t pixelx=0; pixelx<tilesize.x; ++pixelx) {
                for (size_t pixely=0; pixely<tilesize.y; ++pixely) {
                    size_t tempi=xytoi(pixelx,pixely,tilesize.x,tilesize.y)*4;
                    nes_uchar tiletypetemp=spritetemp->arr[pixelx][pixely];
                    tempquadretto[tempi]=rgba[tiletypetemp][0];
                    tempquadretto[tempi+1]=rgba[tiletypetemp][1];
                    tempquadretto[tempi+2]=rgba[tiletypetemp][2];
                    tempquadretto[tempi+3]=rgba[tiletypetemp][3];
                }
            }
            tiletexture.update(tempquadretto,
                               tilesize.x,
                               tilesize.y,
                               itoxrect(texturenumber,texturesize,texturesize,tilesize.x,tilesize.y),
                               itoyrect(texturenumber,texturesize,texturesize,tilesize.x,tilesize.y));
        }
        else {
            for (size_t pixelx=0; pixelx<tilesize.x; ++pixelx) {
                for (size_t pixely=0; pixely<tilesize.y; ++pixely) {
                    nes_uchar tiletypetemp=spritetemp->arr[pixelx][pixely];
                    prerendering->setPixel(pixelx+itoxrect(texturenumber,texturesize,texturesize,tilesize.x,tilesize.y),
                                        pixely+itoyrect(texturenumber,texturesize,texturesize,tilesize.x,tilesize.y),
                                        sf::Color(rgba[tiletypetemp][0],
                                                  rgba[tiletypetemp][1],
                                                  rgba[tiletypetemp][2],
                                                  rgba[tiletypetemp][3]));
                }
            }
        }
        whereisthistexture=texturenumber;
        texturemap[temptile]=whereisthistexture;
        ++texturenumber;
    }
    else {
        whereisthistexture=texturemap[temptile];
    }
    return whereisthistexture;
}

void TileRenderer::drawtexture(sf::RenderTarget& target, sf::RenderStates states){

    sf::Clock trclock;
    for (size_t x=0; x<width; ++x) {
        for (size_t y=0; y<height; ++y) {
            if (tilecont.updated(x,y)) {
                size_t whereisthistexture=add_or_find_texture(tilecont.atconst(x,y));
                const size_t tempi=xytoi(x,y,width,height)*4;
                const sf::Vector2u texturesize=tiletexture.getSize();
                size_t tu = whereisthistexture % (texturesize.x / tilesize.x);
                size_t tv = whereisthistexture / (texturesize.x / tilesize.x);

                verteximage[tempi].texCoords=sf::Vector2f(tu*tilesize.x,tv*tilesize.x);
                verteximage[tempi+1].texCoords=sf::Vector2f((tu+1)*tilesize.x,tv*tilesize.x);
                verteximage[tempi+2].texCoords=sf::Vector2f((tu+1)*tilesize.x,(tv+1)*tilesize.x);
                verteximage[tempi+3].texCoords=sf::Vector2f(tu*tilesize.x,(tv+1)*tilesize.x);
            }
        }
    }
    states.transform *= getTransform();
    states.texture = &tiletexture;
    target.draw(verteximage,states);
    tilecont.resetupdated();
}

void TileRenderer::drawvertex(sf::RenderTarget& target, sf::RenderStates states){

    sf::Clock trclock;
    for (size_t x=0; x<width; ++x) {
        for (size_t y=0; y<height; ++y) {
            if (tilecont.updated(x,y)) {
                const int primacifra[4]={ //could be made into a routine to clear some of this code
                    tilecont.atconst(x,y).palette_color[0]/16,
                    tilecont.atconst(x,y).palette_color[1]/16,
                    tilecont.atconst(x,y).palette_color[2]/16,
                    tilecont.atconst(x,y).palette_color[3]/16
                };
                const int secondacifra[4]={
                    tilecont.atconst(x,y).palette_color[0]%16,
                    tilecont.atconst(x,y).palette_color[1]%16,
                    tilecont.atconst(x,y).palette_color[2]%16,
                    tilecont.atconst(x,y).palette_color[3]%16
                };
                const unsigned int color[4]={
                    palette[primacifra[0]][secondacifra[0]],
                    palette[primacifra[1]][secondacifra[1]],
                    palette[primacifra[2]][secondacifra[2]],
                    palette[primacifra[3]][secondacifra[3]]
                };
                const sf::Color sfcolor[4]={
                    sf::Color(color[0]),
                    sf::Color(color[1]),
                    sf::Color(color[2]),
                    sf::Color(color[3])
                };
                Sprite* spritetemp=&spritevector[tilecont.atconst(x,y).tilenumber];
                for (size_t pixelx=0; pixelx<tilesize.x; ++pixelx) {
                    for (size_t pixely=0; pixely<tilesize.y; ++pixely) {
                        nes_uchar tiletypetemp=spritetemp->arr[pixelx][pixely];
                        //if (colortemp.a!=0)
                        const size_t tempi=xytoi(x*tilesize.x+pixelx,y*tilesize.y+pixely,width*tilesize.x,height*tilesize.y)*4;
                        verteximage[tempi].color=sfcolor[tiletypetemp];
                        verteximage[tempi+1].color=sfcolor[tiletypetemp];
                        verteximage[tempi+2].color=sfcolor[tiletypetemp];
                        verteximage[tempi+3].color=sfcolor[tiletypetemp];
                    }
                }
            }
        }
    }
    target.draw(verteximage,states);
    tilecont.resetupdated();
}

void TileRenderer::drawsprite(sf::RenderTarget& target, sf::RenderStates states){
    for (size_t x=0; x<width; ++x) {
        for (size_t y=0; y<height; ++y) {
            if (tilecont.updated(x,y)) {
                const int primacifra[4]={
                    tilecont.atconst(x,y).palette_color[0]/16,
                    tilecont.atconst(x,y).palette_color[1]/16,
                    tilecont.atconst(x,y).palette_color[2]/16,
                    tilecont.atconst(x,y).palette_color[3]/16
                };
                const int secondacifra[4]={
                    tilecont.atconst(x,y).palette_color[0]%16,
                    tilecont.atconst(x,y).palette_color[1]%16,
                    tilecont.atconst(x,y).palette_color[2]%16,
                    tilecont.atconst(x,y).palette_color[3]%16
                };
                const unsigned int color[4]={
                    palette[primacifra[0]][secondacifra[0]],
                    palette[primacifra[1]][secondacifra[1]],
                    palette[primacifra[2]][secondacifra[2]],
                    palette[primacifra[3]][secondacifra[3]]
                };
                sf::Uint8 rgba[4][4];
                for (size_t colorcounter=0; colorcounter<4;++colorcounter) {
                    int colortemp=color[colorcounter];
                    for (size_t rgbacounter=0; rgbacounter<4;++rgbacounter) {
                        rgba[colorcounter][4-rgbacounter-1]=colortemp&0xff;
                        colortemp=colortemp>>8;
                    }
                }
                Sprite* spritetemp=&spritevector[tilecont.atconst(x,y).tilenumber];
                sf::Uint8* tempquadretto=quadretti[xytoi(x,y,width,height)].getQuadretto();
                for (size_t pixelx=0; pixelx<tilesize.x; ++pixelx) {
                    for (size_t pixely=0; pixely<tilesize.y; ++pixely) {
                        size_t tempi=xytoi(pixelx,pixely,tilesize.x,tilesize.y)*4;
                        nes_uchar tiletypetemp=spritetemp->arr[pixelx][pixely];
                        tempquadretto[tempi]=rgba[tiletypetemp][0];
                        tempquadretto[tempi+1]=rgba[tiletypetemp][1];
                        tempquadretto[tempi+2]=rgba[tiletypetemp][2];
                        tempquadretto[tempi+3]=rgba[tiletypetemp][3];
                    }
                }
                temptexclass.update(tempquadretto,tilesize.x,tilesize.y,x*tilesize.x,y*tilesize.y);
            }
        }
    }
    target.draw(tempspriteclass,states);
    tilecont.resetupdated();
}

void TileRenderer::drawimage(sf::RenderTarget& target, sf::RenderStates states){
    for (size_t x=0; x<width; ++x) {
        for (size_t y=0; y<height; ++y) {
            if (tilecont.updated(x,y)) {
                const int primacifra[4]={
                    tilecont.atconst(x,y).palette_color[0]/16,
                    tilecont.atconst(x,y).palette_color[1]/16,
                    tilecont.atconst(x,y).palette_color[2]/16,
                    tilecont.atconst(x,y).palette_color[3]/16
                };
                const int secondacifra[4]={
                    tilecont.atconst(x,y).palette_color[0]%16,
                    tilecont.atconst(x,y).palette_color[1]%16,
                    tilecont.atconst(x,y).palette_color[2]%16,
                    tilecont.atconst(x,y).palette_color[3]%16
                };
                const unsigned int color[4]={
                    palette[primacifra[0]][secondacifra[0]],
                    palette[primacifra[1]][secondacifra[1]],
                    palette[primacifra[2]][secondacifra[2]],
                    palette[primacifra[3]][secondacifra[3]]
                };
                const sf::Color sfcolor[4]={
                    sf::Color(color[0]),
                    sf::Color(color[1]),
                    sf::Color(color[2]),
                    sf::Color(color[3])
                };
                Sprite *spritetemp=&spritevector[tilecont.atconst(x,y).tilenumber];
                for (size_t pixelx=0; pixelx<tilesize.x; ++pixelx) {
                    for (size_t pixely=0; pixely<tilesize.y; ++pixely) {
                        nes_uchar tiletypetemp=spritetemp->arr[pixelx][pixely];
                        //if (colortemp.a!=0)
                        finalimageclass.setPixel(x*tilesize.x+pixelx,y*tilesize.y+pixely,sfcolor[tiletypetemp]); //sf::Color(color[Spritevector[tilecont(x,y).tilenumber].arr[pixelx][pixely]]
                    }
                }
            }
        }
    }
    temptexclass.update(finalimageclass);
    target.draw(tempspriteclass,states);
    tilecont.resetupdated();
}

void TileRenderer::add_frequent_textures() {
    std::ifstream previous_textures("Pre-rendered textures.txt");
    if (!previous_textures) {
        glb::cm.update<std::string>("error","Couldn't open previous textures");
    }

    sf::Image texture_image;
    texture_image.create(texturesize, texturesize);
    size_t tilenumber;
    unsigned int c1,c2,c3,c4; //using char would read 1 char at a time e.g. 2a read as 2
    while ((previous_textures>>std::dec>>tilenumber>>std::hex>>c1>>c2>>c3>>c4>>std::dec)) {
        add_or_find_texture(tiletype(tilenumber,c1,c2,c3,c4),&texture_image);
    }
    previous_textures.close();
    tiletype block;
    add_or_find_texture(block, &texture_image);
    block=tiletype(0,0);
    add_or_find_texture(block, &texture_image);
    for (size_t level=0; level<10; ++level) {
        block=tiletype(level,1);
        add_or_find_texture(block, &texture_image);
        block=tiletype(level,2);
        add_or_find_texture(block, &texture_image);
        block=tiletype(level,3);
        add_or_find_texture(block, &texture_image);
    }
    tiletexture.loadFromImage(texture_image);
    newtextures=fopen("Pre-rendered textures.txt","a");
}

void TileRenderer::drawmod(sf::RenderTarget& target, sf::RenderStates states)
{
    switch(drawmethod) {
    case DRAWIMAGE:
        drawimage(target,states);
        break;
    case DRAWSPRITE:
        drawsprite(target,states);
        break;
    case DRAWTEXTURE:
        drawtexture(target,states);
        break;
    case DRAWVERTEX:
        drawvertex(target, states);
        break;
    default:
        glb::cm.update<std::string>("error","Warning, default drawmod");
        drawimage(target,states);
        break;
    }
}

TileRenderer::~TileRenderer() {
    if (newtextures) fclose(newtextures);
    if (quadretti) delete[]quadretti;
}


unsigned char TileRenderer::colors[10][4]={
    {0x0D ,0x30 ,0x21 ,0x12},
    {0x0D ,0x30 ,0x29 ,0x1A},
    {0x0D ,0x30 ,0x24 ,0x14},
    {0x0D ,0x30 ,0x2A ,0x12},
    {0x0D ,0x30 ,0x2B ,0x15},
    {0x0D ,0x30 ,0x22 ,0x2B},
    {0x0D ,0x30 ,0x00 ,0x16},
    {0x0D ,0x30 ,0x05 ,0x13},
    {0x0D ,0x30 ,0x16 ,0x12},
    {0x0D ,0x30 ,0x27 ,0x16}
};

sf::Uint32 TileRenderer::palette[4][16]= { //RGB PALLETE
	{0x6d6d6dff,0x2492ff,0xdbff,0x6d49dbff,0x92006dff,0xb6006dff,0xb62400ff,0x924900ff,0x6d4900ff,0x244900ff,0x6d24ff,0x9200ff,0x4949ff,0xff,0xff,0xff},
	{0xb6b6b6ff,0x6ddbff,0x49ffff,0x9200ffff,0xb600ffff,0xff0092ff,0xff0000ff,0xdb6d00ff,0x926d00ff,0x249200ff,0x9200ff,0xb66dff,0x9292ff,0x242424ff,0xff,0xff},
	{0xffffffff,0x6db6ffff,0x9292ffff,0xdb6dffff,0xff00ffff,0xff6dffff,0xff9200ff,0xffb600ff,0xdbdb00ff,0x6ddb00ff,0xff00ff,0x49ffdbff,0xffffff,0x494949ff,0xff,0xff},
	{0xffffffff,0xb6dbffff,0xdbb6ffff,0xffb6ffff,0xff92ffff,0xffb6b6ff,0xffdb92ff,0xffff49ff,0xffff6dff,0xb6ff49ff,0x92ff6dff,0x49ffdbff,0x92dbffff,0x929292ff,0xff,0xff}
};

/*sf::Uint32 TileRenderer::palette[4][16]= { //idk palette
    {0x7C7C7CFF ,0x0000FCFF ,0x0000BCFF ,0x4428BCFF ,0x940084FF ,0xA80020FF ,0xA81000FF ,0x881400FF ,0x503000FF ,0x007800FF ,0x006800FF ,0x005800FF ,0x004058FF ,0x000000FF ,0x000000FF ,0x000000FF},
    {0xBCBCBCFF ,0x0078F8FF ,0x0058F8FF ,0x6844FCFF ,0xD800CCFF ,0xE40058FF ,0xF83800FF ,0xE45C10FF ,0xAC7C00FF ,0x00B800FF ,0x00A800FF ,0x00A844FF ,0x008888FF ,0x00000000 ,0x000000FF ,0x000000FF},
    {0xF8F8F8FF ,0x3CBCFCFF ,0x6888FCFF ,0x9878F8FF ,0xF878F8FF ,0xF85898FF ,0xF87858FF ,0xFCA044FF ,0xF8B800FF ,0xB8F818FF ,0x58D854FF ,0x58F898FF ,0x00E8D8FF ,0x787878FF ,0x000000FF ,0x000000FF},
    {0xFCFCFCFF ,0xA4E4FCFF ,0xB8B8F8FF ,0xD8B8F8FF ,0xF8B8F8FF ,0xF8A4C0FF ,0xF0D0B0FF ,0xFCE0A8FF ,0xF8D878FF ,0xD8F878FF ,0xB8F8B8FF ,0xB8F8D8FF ,0x00FCFCFF ,0xF8D8F8FF ,0x000000FF ,0x000000FF}
};*/

/*sf::Uint32 TileRenderer::palette[4][16]= {
    {0x7C7C7C ,0x0000FC ,0x0000BC ,0x4428BC ,0x940084 ,0xA80020 ,0xA81000 ,0x881400 ,0x503000 ,0x007800 ,0x006800 ,0x005800 ,0x004058 ,0x000000 ,0x000000 ,0x000000},
    {0xBCBCBC ,0x0078F8 ,0x0058F8 ,0x6844FC ,0xD800CC ,0xE40058 ,0xF83800 ,0xE45C10 ,0xAC7C00 ,0x00B800 ,0x00A800 ,0x00A844 ,0x008888 ,0x000000 ,0x000000 ,0x000000},
    {0xF8F8F8 ,0x3CBCFC ,0x6888FC ,0x9878F8 ,0xF878F8 ,0xF85898 ,0xF87858 ,0xFCA044 ,0xF8B800 ,0xB8F818 ,0x58D854 ,0x58F898 ,0x00E8D8 ,0x787878 ,0x000000 ,0x000000},
    {0xFCFCFC ,0xA4E4FC ,0xB8B8F8 ,0xD8B8F8 ,0xF8B8F8 ,0xF8A4C0 ,0xF0D0B0 ,0xFCE0A8 ,0xF8D878 ,0xD8F878 ,0xB8F8B8 ,0xB8F8D8 ,0x00FCFC ,0xF8D8F8 ,0x000000 ,0x000000}
};*/