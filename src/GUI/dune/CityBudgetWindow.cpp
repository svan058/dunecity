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

#include <GUI/dune/CityBudgetWindow.h>

#include <globals.h>
#include <Game.h>
#include <dunecity/CitySimulation.h>
#include <Command.h>

#include <FileClasses/TextManager.h>
#include <FileClasses/GFXManager.h>
#include <misc/format.h>

CityBudgetWindow::CityBudgetWindow()
 : Window(0, 0, 400, 350) {

    setWindowWidget(&mainVBox);
    
    int centerX = (getRendererWidth() - getSize().x) / 2;
    int centerY = (getRendererHeight() - getSize().y) / 2;
    setCurrentPosition(centerX, centerY);

    mainVBox.addWidget(VSpacer::create(10));

    titleLabel.setText("City Budget");
    titleLabel.setAlignment(Alignment_HCenter);
    titleLabel.setTextColor(COLOR_WHITE);
    mainVBox.addWidget(&titleLabel);
    mainVBox.addWidget(VSpacer::create(15));

    treasuryLabel.setText("Treasury: 0");
    treasuryLabel.setTextColor(COLOR_WHITE);
    mainVBox.addWidget(&treasuryLabel);
    mainVBox.addWidget(VSpacer::create(8));

    incomeLabel.setText("Income: 0");
    incomeLabel.setTextColor(COLOR_WHITE);
    mainVBox.addWidget(&incomeLabel);
    mainVBox.addWidget(VSpacer::create(8));

    expensesLabel.setText("Expenses: 0");
    expensesLabel.setTextColor(COLOR_WHITE);
    mainVBox.addWidget(&expensesLabel);
    mainVBox.addWidget(VSpacer::create(15));

    taxRateLabel.setText("Tax Rate:");
    taxRateLabel.setTextColor(COLOR_WHITE);
    taxRateHBox.addWidget(&taxRateLabel);
    taxRateHBox.addWidget(HSpacer::create(10));

    taxRateMinus.setTextures(pGFXManager->getUIGraphic(UI_Minus), pGFXManager->getUIGraphic(UI_Minus_Pressed));
    taxRateMinus.setOnClick(std::bind(&CityBudgetWindow::onTaxRateDecrease, this));
    taxRateHBox.addWidget(&taxRateMinus);
    taxRateHBox.addWidget(HSpacer::create(5));

    taxRateValueLabel.setText("0%");
    taxRateValueLabel.setTextColor(COLOR_WHITE);
    taxRateHBox.addWidget(&taxRateValueLabel);
    taxRateHBox.addWidget(HSpacer::create(5));

    taxRatePlus.setTextures(pGFXManager->getUIGraphic(UI_Plus), pGFXManager->getUIGraphic(UI_Plus_Pressed));
    taxRatePlus.setOnClick(std::bind(&CityBudgetWindow::onTaxRateIncrease, this));
    taxRateHBox.addWidget(&taxRatePlus);

    mainVBox.addWidget(&taxRateHBox, 30);
    mainVBox.addWidget(VSpacer::create(15));

    resPopLabel.setText("Residential: 0");
    resPopLabel.setTextColor(COLOR_WHITE);
    mainVBox.addWidget(&resPopLabel);
    mainVBox.addWidget(VSpacer::create(8));

    comPopLabel.setText("Commercial: 0");
    comPopLabel.setTextColor(COLOR_WHITE);
    mainVBox.addWidget(&comPopLabel);
    mainVBox.addWidget(VSpacer::create(8));

    indPopLabel.setText("Industrial: 0");
    indPopLabel.setTextColor(COLOR_WHITE);
    mainVBox.addWidget(&indPopLabel);
    mainVBox.addWidget(VSpacer::create(8));

    totalPopLabel.setText("Total Population: 0");
    totalPopLabel.setTextColor(COLOR_WHITE);
    mainVBox.addWidget(&totalPopLabel);
    mainVBox.addWidget(VSpacer::create(15));

    closeButton.setText("Close");
    closeButton.setOnClick(std::bind(&CityBudgetWindow::onClose, this));
    mainVBox.addWidget(&closeButton);
    mainVBox.addWidget(VSpacer::create(10));

    updateDisplay();
}

CityBudgetWindow::~CityBudgetWindow() {
}

void CityBudgetWindow::onClose() {
    Window* pParentWindow = dynamic_cast<Window*>(getParent());
    if(pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}

void CityBudgetWindow::onTaxRateIncrease() {
    auto* citySim = currentGame->getCitySimulation();
    if (!citySim || !citySim->isInitialized()) return;

    int16_t currentRate = citySim->getCityTax();
    if (currentRate < DuneCity::MAX_TAX_RATE) {
        currentGame->getCommandManager().addCommand(
            Command(pLocalPlayer->getPlayerID(), CMD_CITY_SET_TAX_RATE, 0, currentRate + 1, 0));
    }
    updateDisplay();
}

void CityBudgetWindow::onTaxRateDecrease() {
    auto* citySim = currentGame->getCitySimulation();
    if (!citySim || !citySim->isInitialized()) return;

    int16_t currentRate = citySim->getCityTax();
    if (currentRate > 0) {
        currentGame->getCommandManager().addCommand(
            Command(pLocalPlayer->getPlayerID(), CMD_CITY_SET_TAX_RATE, 0, currentRate - 1, 0));
    }
    updateDisplay();
}

void CityBudgetWindow::updateDisplay() {
    auto* citySim = currentGame->getCitySimulation();
    if (!citySim || !citySim->isInitialized()) {
        return;
    }

    treasuryLabel.setText(fmt::sprintf("Treasury: %d", citySim->getTotalFunds()));
    
    auto& budget = citySim->getCityBudget();
    incomeLabel.setText(fmt::sprintf("Income: %d", budget.getTaxIncome()));
    
    int totalExpenses = budget.getRoadFundingRequest() + 
                        budget.getPoliceFundingRequest() + 
                        budget.getFireFundingRequest();
    expensesLabel.setText(fmt::sprintf("Expenses: %d", totalExpenses));
    
    taxRateValueLabel.setText(fmt::sprintf("%d%%", citySim->getCityTax()));
    
    resPopLabel.setText(fmt::sprintf("Residential: %d", citySim->getResPop()));
    comPopLabel.setText(fmt::sprintf("Commercial: %d", citySim->getComPop()));
    indPopLabel.setText(fmt::sprintf("Industrial: %d", citySim->getIndPop()));
    totalPopLabel.setText(fmt::sprintf("Total Population: %d", citySim->getTotalPop()));
}
