#include "Engine.h"
#include<cstdio>
#include"random.h"
#include"ConsoleManager.h"
#include<string>
Engine::Engine(TileContainer* _tilecont, const MenuType& _startingmenu):
tilecont(_tilecont),
currentmenu(_startingmenu),
RLS(_tilecont,0,0),
RPF(_tilecont, framecounter, 0),
RHS(_tilecont, framecounter)
{
    levelselectreload=true;

    framecounter=0;
    ConsoleManager::update<std::string>("system",std::string("Engine init"));
}


void Engine::frame(const ActiveInputs& _inputs) {
    ++framecounter;
    incframe();
    random::prng();
    switch(currentmenu) {

    case LEVELSELECT:
        RLS.renderLevelSelect(levelselectreload);
        levelselectreload=false;
        levelselect=RLS.updateLevelSelect(_inputs);
        if (levelselect>=0) {
            currentmenu=PLAYFIELD;
            RPF.resetPlayField(levelselect);
        }
        break;

    case PLAYFIELD:

        RPF.update(_inputs, framecounter);
        RPF.render(framecounter);
        if (RPF.gameOver()) {
            RHS.resetHighScore(RPF.matrixhandler, RPF.piecehandler, RPF.scorehandler, RPF.levellineshandler, RPF.statisticshandler);
            currentmenu=HIGHSCORE;
        }
        break;

    case HIGHSCORE:
        RHS.update(_inputs, framecounter);
        RHS.render(framecounter);
        break;

    default:
        ConsoleManager::update_error("ERROR default frame case in switch");
        break;
    }
}
