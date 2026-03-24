/*
 *  This file is part of Dune Legacy.
 *
 *  Dune Legacy is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Dune Legacy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Dune Legacy.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <GameInterface.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/FontManager.h>
#include <House.h>
#include <Game.h>

#include <ObjectBase.h>
#include <GUI/ObjectInterfaces/ObjectInterface.h>
#include <GUI/ObjectInterfaces/MultiUnitInterface.h>

#include <misc/draw_util.h>
#include <misc/SDL2pp.h>
#include <misc/format.h>

#include <dunecity/CitySimulation.h>
#include <dunecity/CityBudget.h>
#include <dunecity/CityEvaluation.h>

#include <algorithm>

GameInterface::GameInterface() : Window(0,0,0,0) {
    pObjectContainer = nullptr;
    objectID = NONE_ID;
    showCityStatsOverlay = false;

    setTransparentBackground(true);

    setCurrentPosition(0,0,getRendererWidth(),getRendererHeight());

    setWindowWidget(&windowWidget);

    // top bar
    SDL_Texture* pTopBarTex = pGFXManager->getUIGraphic(UI_TopBar, pLocalHouse->getHouseID());
    topBar.setTexture(pTopBarTex);
    windowWidget.addWidget(&topBar,Point(0,0),Point(getWidth(pTopBarTex),getHeight(pTopBarTex) - 12));

    // side bar
    SDL_Texture* pSideBarTex = pGFXManager->getUIGraphic(UI_SideBar, pLocalHouse->getHouseID());
    sideBar.setTexture(pSideBarTex);
    SDL_Rect dest = calcAlignedDrawingRect(pSideBarTex, HAlign::Right, VAlign::Top);
    windowWidget.addWidget(&sideBar, dest);

    // add buttons
    windowWidget.addWidget(&topBarHBox,Point(5,5),
                            Point(getRendererWidth() - sideBar.getSize().x, topBar.getSize().y - 10));

    topBarHBox.addWidget(&newsticker);

    topBarHBox.addWidget(Spacer::create());

    optionsButton.setTextures(  pGFXManager->getUIGraphic(UI_Options, pLocalHouse->getHouseID()),
                                pGFXManager->getUIGraphic(UI_Options_Pressed, pLocalHouse->getHouseID()));
    optionsButton.setOnClick(std::bind(&Game::onOptions, currentGame));
    topBarHBox.addWidget(&optionsButton);

    topBarHBox.addWidget(Spacer::create());

    mentatButton.setTextures(   pGFXManager->getUIGraphic(UI_Mentat, pLocalHouse->getHouseID()),
                                pGFXManager->getUIGraphic(UI_Mentat_Pressed, pLocalHouse->getHouseID()));
    mentatButton.setOnClick(std::bind(&Game::onMentat, currentGame));
    topBarHBox.addWidget(&mentatButton);

    topBarHBox.addWidget(Spacer::create());

    // add radar
    const Point radarOrigin(getRendererWidth() - sideBar.getSize().x + SIDEBAR_COLUMN_WIDTH, 0);
    const Point radarSize = radarView.getMinimumSize();
    windowWidget.addWidget(&radarView, radarOrigin, radarSize);
    radarView.setOnRadarClick(std::bind(&Game::onRadarClick, currentGame, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // add ornithopter selection shortcut button just below the radar
    ornithopterSelectButton.setText(_("Ornithopter"));
    ornithopterSelectButton.setTooltipText(_("Select all ornithopters (Hotkey: O)"));
    ornithopterSelectButton.setOnClick(std::bind(&Game::selectAllOrnithopters, currentGame));

    const int ornithopterButtonWidth = sideBar.getSize().x - 25;
    const int ornithopterButtonHeight = std::max(ornithopterSelectButton.getMinimumSize().y, 36);
    const Point ornithopterButtonSize(ornithopterButtonWidth, ornithopterButtonHeight);
    const Point ornithopterButtonPos(
        getRendererWidth() - sideBar.getSize().x + 24,
        146
    );
    ornithopterSelectButton.resize(ornithopterButtonSize.x, ornithopterButtonSize.y);
    windowWidget.addWidget(&ornithopterSelectButton, ornithopterButtonPos, ornithopterButtonSize);

    // add chat manager
    windowWidget.addWidget(&chatManager, Point(20, 60), Point(getRendererWidth() - sideBar.getSize().x, 360));
}

GameInterface::~GameInterface() {
    removeOldContainer();
}

void GameInterface::draw(Point position) {
    Window::draw(position);

    // draw Power Indicator and Spice indicator

    SDL_Rect powerIndicatorPos = {  getRendererWidth() - sideBar.getSize().x + 14, 146, 4, getRendererHeight() - 146 - 2 };
    renderFillRect(renderer, &powerIndicatorPos, COLOR_BLACK);

    SDL_Rect spiceIndicatorPos = {  getRendererWidth() - sideBar.getSize().x + 20, 146, 4, getRendererHeight() - 146 - 2 };
    renderFillRect(renderer, &spiceIndicatorPos, COLOR_BLACK);

    int xCount = 0, yCount = 0;
    int yCount2 = 0;

    //draw power level indicator
    if (pLocalHouse->getPowerRequirement() == 0)    {
        if (pLocalHouse->getProducedPower() > 0) {
            yCount2 = powerIndicatorPos.h + 1;
        } else {
            yCount2 = powerIndicatorPos.h/2;
        }
    } else {
        yCount2 = lround((double)pLocalHouse->getProducedPower()/(double)pLocalHouse->getPowerRequirement()*(double)(powerIndicatorPos.h/2));
    }

    if (yCount2 > powerIndicatorPos.h + 1) {
        yCount2 = powerIndicatorPos.h + 1;
    }

    setRenderDrawColor(renderer, COLOR_GREEN);
    for (yCount = 0; yCount < yCount2; yCount++) {
        for (xCount = 1; xCount < powerIndicatorPos.w - 1; xCount++) {
            if(((yCount/2) % 3) != 0) {
                SDL_RenderDrawPoint(renderer, xCount + powerIndicatorPos.x, powerIndicatorPos.y + powerIndicatorPos.h - yCount);
            }
        }
    }

    //draw spice level indicator
    if (pLocalHouse->getCapacity() == 0) {
        yCount2 = 0;
    } else {
        yCount2 = lround((pLocalHouse->getStoredCredits()/pLocalHouse->getCapacity())*spiceIndicatorPos.h);
    }

    if (yCount2 > spiceIndicatorPos.h + 1) {
        yCount2 = spiceIndicatorPos.h + 1;
    }

    setRenderDrawColor(renderer, COLOR_ORANGE);
    for (yCount = 0; yCount < yCount2; yCount++) {
        for (xCount = 1; xCount < spiceIndicatorPos.w - 1; xCount++) {
            if(((yCount/2) % 3) != 0) {
                SDL_RenderDrawPoint(renderer, xCount + spiceIndicatorPos.x, spiceIndicatorPos.y + spiceIndicatorPos.h - yCount);
            }
        }
    }

    //draw credits
    const auto credits = pLocalHouse->getCredits();
    const auto CreditsBuffer = std::to_string((credits < 0) ? 0 : credits);
    const auto NumDigits = CreditsBuffer.length();
    SDL_Texture* digitsTex = pGFXManager->getUIGraphic(UI_CreditsDigits);

    for(int i=NumDigits-1; i>=0; i--) {
        SDL_Rect source = calcSpriteSourceRect(digitsTex, CreditsBuffer[i] - '0', 10);
        SDL_Rect dest = calcSpriteDrawingRect(digitsTex, getRendererWidth() - sideBar.getSize().x + 49 + (6 - NumDigits + i)*10, 135, 10);
        SDL_RenderCopy(renderer, digitsTex, &source, &dest);
    }

    if(showCityStatsOverlay) {
        drawCityStatsOverlay();
    }
}

void GameInterface::updateObjectInterface() {
    const auto& selection = currentGame->getSelectedList();

    if(selection.empty()) {
        ornithopterSelectButton.setVisible(true);
        removeOldContainer();
        return;
    }

    ornithopterSelectButton.setVisible(false);

    if(selection.size() == 1) {
        ObjectBase* pObject = currentGame->getObjectManager().getObject(*selection.begin());
        Uint32 newObjectID = pObject->getObjectID();

        if(newObjectID != objectID) {
            removeOldContainer();

            pObjectContainer = pObject->getInterfaceContainer();

            if(pObjectContainer != nullptr) {
                objectID = newObjectID;

                windowWidget.addWidget(pObjectContainer,
                                        Point(getRendererWidth() - sideBar.getSize().x + 24, 146),
                                        Point(sideBar.getSize().x - 25,getRendererHeight() - 148));

            }

        } else {
            if(pObjectContainer->update() == false) {
                removeOldContainer();
            }
        }
    } else {

        if((pObjectContainer == nullptr) || (objectID != NONE_ID)) {
            // either there was nothing selected before or exactly one unit

            if(pObjectContainer != nullptr) {
                removeOldContainer();
            }

            pObjectContainer = MultiUnitInterface::create();

            windowWidget.addWidget(pObjectContainer,
                                    Point(getRendererWidth() - sideBar.getSize().x + 24, 146),
                                    Point(sideBar.getSize().x - 25,getRendererHeight() - 148));
        } else {
            if(pObjectContainer->update() == false) {
                removeOldContainer();
            }
        }
    }
}

void GameInterface::removeOldContainer() {
    if(pObjectContainer != nullptr) {
        delete pObjectContainer;
        pObjectContainer = nullptr;
        objectID = NONE_ID;
    }
}

void GameInterface::drawCityStatsOverlay() {
    auto* citySim = currentGame->getCitySimulation();
    if(!citySim || !citySim->isInitialized()) {
        return;
    }

    const int overlayWidth = 250;
    const int overlayHeight = 220;
    const int overlayX = getRendererWidth() - overlayWidth - 10;
    const int overlayY = 10;
    const int padding = 8;
    const int lineHeight = 16;
    const int barHeight = 12;

    SDL_Rect bgRect = { overlayX, overlayY, overlayWidth, overlayHeight };
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_RenderFillRect(renderer, &bgRect);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &bgRect);

    int textX = overlayX + padding;
    int textY = overlayY + padding;

    auto drawText = [&](const std::string& text, unsigned int fontSize, Uint32 color = COLOR_WHITE) {
        sdl2::texture_ptr textTexture = pFontManager->createTextureWithText(text, color, fontSize);
        if(textTexture) {
            SDL_Rect destRect = { textX, textY, 0, 0 };
            SDL_QueryTexture(textTexture.get(), nullptr, nullptr, &destRect.w, &destRect.h);
            SDL_RenderCopy(renderer, textTexture.get(), nullptr, &destRect);
        }
        textY += lineHeight;
    };

    auto drawBar = [&](int value, int maxValue, uint8_t r, uint8_t g, uint8_t b) {
        int barWidth = overlayWidth - 2 * padding;
        int fillWidth = maxValue > 0 ? (value * barWidth) / maxValue : 0;
        
        SDL_Rect barBg = { textX, textY, barWidth, barHeight };
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(renderer, &barBg);
        
        if (fillWidth > 0) {
            SDL_Rect barFill = { textX, textY, fillWidth, barHeight };
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderFillRect(renderer, &barFill);
        }
        
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        SDL_RenderDrawRect(renderer, &barBg);
        
        textY += barHeight + 4;
    };

    drawText("=== CITY STATS ===", 12);
    textY += 4;

    int resPop = citySim->getResPop();
    int comPop = citySim->getComPop();
    int indPop = citySim->getIndPop();
    int totalPop = citySim->getTotalPop();
    
    int16_t resValve = citySim->getResValve();
    int16_t comValve = citySim->getComValve();
    int16_t indValve = citySim->getIndValve();
    
    const char* resArrow = resValve > 100 ? " ↑" : (resValve < -100 ? " ↓" : "");
    const char* comArrow = comValve > 100 ? " ↑" : (comValve < -100 ? " ↓" : "");
    const char* indArrow = indValve > 100 ? " ↑" : (indValve < -100 ? " ↓" : "");
    
    drawText(fmt::sprintf("Population: %d", totalPop), 12);
    drawText(fmt::sprintf("  R: %d%s  C: %d%s  I: %d%s", resPop, resArrow, comPop, comArrow, indPop, indArrow), 10);
    
    if (totalPop > 0) {
        drawBar(resPop, totalPop, 0, 200, 0);
        drawBar(comPop, totalPop, 0, 100, 255);
        drawBar(indPop, totalPop, 200, 200, 0);
    }

    auto& budget = citySim->getCityBudget();
    int income = budget.getTaxIncome();
    int expenses = budget.getRoadFundingRequest() + budget.getPoliceFundingRequest() + budget.getFireFundingRequest();
    int profit = income - expenses;
    Uint32 profitColor = profit >= 0 ? COLOR_GREEN : COLOR_RED;
    
    drawText(fmt::sprintf("Treasury: %d", citySim->getTotalFunds()), 12);
    drawText(fmt::sprintf("Income: %d  Expenses: %d", income, expenses), 10, profitColor);

    int totalTiles = citySim->getMapWidth() * citySim->getMapHeight();
    int poweredTiles = 0;
    const auto& powerMap = citySim->getPowerGridMap();
    for(int y = 0; y < citySim->getMapHeight(); y++) {
        for(int x = 0; x < citySim->getMapWidth(); x++) {
            if(powerMap.get(x, y) > 0) {
                poweredTiles++;
            }
        }
    }
    int powerPercent = totalTiles > 0 ? (poweredTiles * 100) / totalTiles : 0;
    const char* powerIcon = powerPercent >= 80 ? "✓" : (powerPercent >= 50 ? "!" : "✗");
    Uint32 powerColor = powerPercent >= 80 ? COLOR_GREEN : (powerPercent >= 50 ? COLOR_YELLOW : COLOR_RED);
    drawText(fmt::sprintf("Power: %d%% %s", powerPercent, powerIcon), 12, powerColor);

    drawText(fmt::sprintf("Year: %d", citySim->getCityYear()), 12);

    auto overlayMode = currentGame->getCityOverlayMode();
    if (overlayMode != DuneCity::CityOverlayMode::None) {
        const char* overlayName = "Unknown";
        switch (overlayMode) {
            case DuneCity::CityOverlayMode::PowerGrid:      overlayName = "Power Grid"; break;
            case DuneCity::CityOverlayMode::TrafficDensity: overlayName = "Traffic"; break;
            case DuneCity::CityOverlayMode::Pollution:      overlayName = "Pollution"; break;
            case DuneCity::CityOverlayMode::LandValue:      overlayName = "Land Value"; break;
            case DuneCity::CityOverlayMode::CrimeRate:      overlayName = "Crime"; break;
            case DuneCity::CityOverlayMode::Population:     overlayName = "Population"; break;
            default: break;
        }
        drawText(fmt::sprintf("Overlay: %s", overlayName), 10);
    }

    textY += 4;
    drawText("Press C to toggle", 10);
}
