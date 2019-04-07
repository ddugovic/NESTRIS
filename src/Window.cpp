#include"Window.h"
#include<cmath>
#include"ConsoleManager.h"
#include<string>
Window::Window(const std::size_t& _width, const std::size_t& _height, sf::Vector2f _scale, const bool& optimized)
{
    bool fourthirds=false;
    sf::Transform state;
    if (fourthirds) {
        _scale.y*=(double)_width/_height*3./4.;
    }
    state.scale(_scale);
    sf::RenderWindow window(sf::VideoMode(_width*_scale.x, _height*_scale.y), "Nestris");
    sf::Vector2u tilesize(8,8);
    const sf::Vector3<std::size_t> extra_render(16,16,64);
    TileRenderer tilerend(_width/8,_height/8,tilesize,TileRenderer::DRAWTEXTURE,extra_render);
    tilerend.load("texturesprite/sprites.txt");
    //tilerend.load("texturesprite/sprites.txtupdated");
    Engine _engine= Engine(tilerend.getTileContainer(),10); //TODO change 10

    sf::Event event;
    sf::Int64 smallesttimeunit=sf::Int64(0);
    sf::Clock onesecondinit;
    sf::Clock elapsedtime;
    while (onesecondinit.getElapsedTime()<sf::Time(sf::milliseconds(100))) {
        elapsedtime.restart();
        sf::sleep(sf::microseconds(1));
        sf::Int64 _timetemp=elapsedtime.getElapsedTime().asMicroseconds();
        if (_timetemp>smallesttimeunit)smallesttimeunit=_timetemp;
    }
    sf::Int64 microsecondsperframe=1000000.L/60.0988L;
    elapsedtime.restart();
    while (window.isOpen()) {
        if (elapsedtime.getElapsedTime().asMicroseconds()>=microsecondsperframe) {

            glb::cm.update<double>("fps",(double)sf::Int64(1000000)/(double)elapsedtime.getElapsedTime().asMicroseconds());

            elapsedtime.restart();
            sf::Int64 delaycalc=0;
            _engine.frame(inputManager.getInput());

            glb::cm.update<sf::Int64>("input delay",elapsedtime.getElapsedTime().asMicroseconds()-delaycalc);
            delaycalc=elapsedtime.getElapsedTime().asMicroseconds();

            window.clear();//adds 15microseconds
            tilerend.drawmod(window, state);

            glb::cm.update<sf::Int64>("draw delay",elapsedtime.getElapsedTime().asMicroseconds()-delaycalc);
            delaycalc=elapsedtime.getElapsedTime().asMicroseconds();

            window.display();

            delaycalc=elapsedtime.getElapsedTime().asMicroseconds();
            glb::cm.update<sf::Int64>("display delay",elapsedtime.getElapsedTime().asMicroseconds()-delaycalc);

            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    window.close();
            }
            glb::cm.print();
        }
        else {
            sf::Int64 delaystart=elapsedtime.getElapsedTime().asMicroseconds();
            if (optimized) {
                sf::Int64 sleeptime=microsecondsperframe-delaystart;
                sleeptime-=smallesttimeunit;
                sf::sleep(sf::microseconds(sleeptime));
                delaystart=elapsedtime.getElapsedTime().asMicroseconds()-delaystart;
                if (delaystart>smallesttimeunit) smallesttimeunit=delaystart;
                //if (elapsedtime.getElapsedTime().asMicroseconds()>microsecondsperframe) smallesttimeunit+=initialsmallesttimeunit;
            }
            else {
                sf::Int64 cycleduration=(microsecondsperframe-delaystart)/smallesttimeunit-1;
                for (sf::Int64 i=0; i<cycleduration&&(elapsedtime.getElapsedTime().asMicroseconds()+smallesttimeunit<microsecondsperframe); ++i) {
                    sf::sleep(sf::microseconds(1));
                    delaystart=elapsedtime.getElapsedTime().asMicroseconds()-delaystart;
                    if (delaystart>smallesttimeunit) smallesttimeunit=delaystart;
                    delaystart=elapsedtime.getElapsedTime().asMicroseconds();
                }
                //if (elapsedtime.getElapsedTime().asMicroseconds()>microsecondsperframe) smallesttimeunit+=initialsmallesttimeunit;
                /*if (microsecondsperframe-elapsedtime.getElapsedTime().asMicroseconds()<smallesttimeunit)
                    sf::sleep(sf::microseconds(1));*/
            }
        }
    }
    glb::cm.update<std::string>("system",std::string("Window terminating"));
    glb::cm.print(true);
}


/* data analysis
sf::Int64 smallesttimeunit=sf::Int64(0);
    sf::Int64 totaltimewaited=0;
    sf::Clock onesecondinit;
    sf::Clock elapsedtime=sf::Clock();
    sf::Int64 testnumber=0;
    std::vector<sf::Int64> timetests;
    while (onesecondinit.getElapsedTime()<sf::Time(sf::milliseconds(1000))) {
        elapsedtime.restart();
        sf::sleep(sf::microseconds(1));
        sf::Int64 _timetemp=elapsedtime.getElapsedTime().asMicroseconds();
        timetests.push_back(_timetemp);
        //printf("%d ",(int)_timetemp);
        totaltimewaited+= _timetemp;
        if (_timetemp>smallesttimeunit)smallesttimeunit=_timetemp;
        ++testnumber;
    }
    double mean=(double)totaltimewaited/(double)testnumber;
    double variance=0;
    for (sf::Int64 i=0; i<testnumber; ++i) {
        variance+=(timetests[i]-mean)*(timetests[i]-mean);
    }
    variance/=(double)testnumber-1;
    double sd=sqrt(variance);
    printf("mean =%f, sd=%f, variance=%f, biggest hiccup=%d\n",mean,sd,variance,(int)smallesttimeunit);
    */

/* data renderer
sf::Int64 datawidth;
    window.clear();
    for (datawidth=0; datawidth<_width;++datawidth) {
        int rectheight=0;
        sf::RectangleShape istogramma;
        sf::Int64 widthtemp=smallesttimeunit/_width;
        sf::Int64 left=datawidth*widthtemp;
        sf::Int64 right=datawidth*(widthtemp+1);
        for (std::size_t i=0; i<timetests.size(); ++i) {
            if (timetests[i]>=left&&timetests[i]<right) {
                rectheight++;
            }
        }
        istogramma.setSize(sf::Vector2f(2,rectheight));
        istogramma.setFillColor(sf::Color(244,244,244,244));
        istogramma.move(sf::Vector2f(2*datawidth,0));
        window.draw(istogramma);
    }
window.display();
scanf("%d",&datawidth);
*/

/*
Nes-like Graphics Emulator
I'm working on a nes project and taking inspiration from the tutorials I tried to create a tilemap[hyperlink]. The problem is that nes tiles work like this:link. This means that each tile in the texture has a variable color and I couldn't think of a smart way to convert the texture to the color I want each time, it seems impossible with my sf::Color and sf::Texture. I decided to build an sf::Image, convert it to a texture and draw it each frame, which resulted in 4fps. I thought that updating the whole image every frame was the culprit, but even after removing that part in the program the fps only went up to ~10fps. I resolved to using textures and I had this idea: since creating a texture for each color (50 different colors on the NES, 4 colors per tile, hundreds of Sprites) is impossible, I thought of creating a dynamically updated texture that updates when a new tile/color combination is requested, managed with a hash map or a container that lets me find the required texture in O(1) time. The problem is that I couldn't find a way to dynamically update a texture. Afterwards I realized that there's probably a way to render from multiple textures, should I create a Sprite for each tile and
 tie it to its texture?
Does anybody have a better idea?
For reference the nes (ntsc at least)

// NO DISTRIBUTION
Window::Window(const std::size_t& _width, const std::size_t& _height, const bool& optimized)
{
    //initWindow(_width,_height);
    //inputManager=initInput();
    sf::RenderWindow window(sf::VideoMode(_width, _height), "Nestris");
    TileRenderer tilerend(_width/8,_height/8);
    tilerend.load("Sprites.txt");
    Engine _engine= Engine(tilerend.getTileContainer(),10); //TODO change 10
    sf::Event event;
    sf::Int64 smallesttimeunit=sf::Int64(0);
    sf::Clock onesecondinit;
    sf::Clock elapsedtime=sf::Clock();
    while (onesecondinit.getElapsedTime()<sf::Time(sf::milliseconds(1000))) {
        elapsedtime.restart();
        sf::sleep(sf::microseconds(1));
        sf::Int64 _timetemp = elapsedtime.getElapsedTime().asMicroseconds();
        if (_timetemp>smallesttimeunit) smallesttimeunit=_timetemp;
    }
    sf::Int64 microsecondsperframe=1000000./60.0988;
    printf("%d\n",(int)smallesttimeunit);
    elapsedtime.restart();
    while (window.isOpen()) {
        if (optimized) {
            sf::Int64 sleeptime=microsecondsperframe-elapsedtime.getElapsedTime().asMicroseconds();
            sleeptime-=smallesttimeunit;
            sf::sleep(sf::microseconds(sleeptime));
        }
        else {
            sf::Int64 cycleduration=(microsecondsperframe-elapsedtime.getElapsedTime().asMicroseconds())/smallesttimeunit;
            for (sf::Int64 i=0; i<cycleduration; ++i)
                sf::sleep(sf::microseconds(1));
            /COMMENTSTARTif (microsecondsperframe-elapsedtime.getElapsedTime().asMicroseconds()<smallesttimeunit)
                sf::sleep(sf::microseconds(1));COMMENTEND/
        }
        if (elapsedtime.getElapsedTime().asMicroseconds()+sf::Int64(1)>=microsecondsperframe) {
            printf("fps=%f\n",(double)sf::Int64(1000000)/(double)elapsedtime.getElapsedTime().asMicroseconds());
            elapsedtime.restart();
            _engine.frame(inputManager.getInput());
            //SFML update window
            window.clear();
            tilerend.drawmod(window, sf::RenderStates());
            //window.draw(tilerend);
            window.display();
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    window.close();
            }
        }
    }
    printf("hello\n");
}
*/
