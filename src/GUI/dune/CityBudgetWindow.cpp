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
 : Window(100, 100, 420, 480) {

    setWindowWidget(&mainVBox);

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

    // Budget allocation section
    Label allocationTitle;
    allocationTitle.setText("--- Allocation ---");
    allocationTitle.setTextColor(COLOR_WHITE);
    mainVBox.addWidget(&allocationTitle);
    mainVBox.addWidget(VSpacer::create(10));

    // Road/Transport slider
    roadLabel.setText("Transport:");
    roadLabel.setTextColor(COLOR_WHITE);
    roadHBox.addWidget(&roadLabel);
    roadHBox.addWidget(HSpacer::create(10));

    roadMinus.setTextures(pGFXManager->getUIGraphic(UI_Minus), pGFXManager->getUIGraphic(UI_Minus_Pressed));
    roadMinus.setOnClick(std::bind(&CityBudgetWindow::onRoadDecrease, this));
    roadHBox.addWidget(&roadMinus);
    roadHBox.addWidget(HSpacer::create(5));

    roadValueLabel.setText("100%");
    roadValueLabel.setTextColor(COLOR_WHITE);
    roadHBox.addWidget(&roadValueLabel);
    roadHBox.addWidget(HSpacer::create(5));

    roadPlus.setTextures(pGFXManager->getUIGraphic(UI_Plus), pGFXManager->getUIGraphic(UI_Plus_Pressed));
    roadPlus.setOnClick(std::bind(&CityBudgetWindow::onRoadIncrease, this));
    roadHBox.addWidget(&roadPlus);

    mainVBox.addWidget(&roadHBox, 30);
    mainVBox.addWidget(VSpacer::create(5));

    // Police slider
    policeLabel.setText("Police:");
    policeLabel.setTextColor(COLOR_WHITE);
    policeHBox.addWidget(&policeLabel);
    policeHBox.addWidget(HSpacer::create(10));

    policeMinus.setTextures(pGFXManager->getUIGraphic(UI_Minus), pGFXManager->getUIGraphic(UI_Minus_Pressed));
    policeMinus.setOnClick(std::bind(&CityBudgetWindow::onPoliceDecrease, this));
    policeHBox.addWidget(&policeMinus);
    policeHBox.addWidget(HSpacer::create(5));

    policeValueLabel.setText("100%");
    policeValueLabel.setTextColor(COLOR_WHITE);
    policeHBox.addWidget(&policeValueLabel);
    policeHBox.addWidget(HSpacer::create(5));

    policePlus.setTextures(pGFXManager->getUIGraphic(UI_Plus), pGFXManager->getUIGraphic(UI_Plus_Pressed));
    policePlus.setOnClick(std::bind(&CityBudgetWindow::onPoliceIncrease, this));
    policeHBox.addWidget(&policePlus);

    mainVBox.addWidget(&policeHBox, 30);
    mainVBox.addWidget(VSpacer::create(5));

    // Fire slider
    fireLabel.setText("Fire:");
    fireLabel.setTextColor(COLOR_WHITE);
    fireHBox.addWidget(&fireLabel);
    fireHBox.addWidget(HSpacer::create(10));

    fireMinus.setTextures(pGFXManager->getUIGraphic(UI_Minus), pGFXManager->getUIGraphic(UI_Minus_Pressed));
    fireMinus.setOnClick(std::bind(&CityBudgetWindow::onFireDecrease, this));
    fireHBox.addWidget(&fireMinus);
    fireHBox.addWidget(HSpacer::create(5));

    fireValueLabel.setText("100%");
    fireValueLabel.setTextColor(COLOR_WHITE);
    fireHBox.addWidget(&fireValueLabel);
    fireHBox.addWidget(HSpacer::create(5));

    firePlus.setTextures(pGFXManager->getUIGraphic(UI_Plus), pGFXManager->getUIGraphic(UI_Plus_Pressed));
    firePlus.setOnClick(std::bind(&CityBudgetWindow::onFireIncrease, this));
    fireHBox.addWidget(&firePlus);

    mainVBox.addWidget(&fireHBox, 30);
    mainVBox.addWidget(VSpacer::create(10));

    // Total and buttons
    totalAllocationLabel.setText("Total: 300%");
    totalAllocationLabel.setTextColor(COLOR_WHITE);
    allocationButtonsHBox.addWidget(&totalAllocationLabel);
    allocationButtonsHBox.addWidget(HSpacer::create(20));

    applyButton.setText("Apply");
    applyButton.setOnClick(std::bind(&CityBudgetWindow::onApply, this));
    allocationButtonsHBox.addWidget(&applyButton);
    allocationButtonsHBox.addWidget(HSpacer::create(10));

    cancelButton.setText("Cancel");
    cancelButton.setOnClick(std::bind(&CityBudgetWindow::onClose, this));
    allocationButtonsHBox.addWidget(&cancelButton);

    mainVBox.addWidget(&allocationButtonsHBox, 30);
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

void CityBudgetWindow::onRoadIncrease() {
    if (pendingRoadPercent < 100) {
        pendingRoadPercent += 5;
        if (pendingRoadPercent > 100) pendingRoadPercent = 100;
        updateAllocationLabels();
    }
}

void CityBudgetWindow::onRoadDecrease() {
    if (pendingRoadPercent > 0) {
        pendingRoadPercent -= 5;
        if (pendingRoadPercent < 0) pendingRoadPercent = 0;
        updateAllocationLabels();
    }
}

void CityBudgetWindow::onPoliceIncrease() {
    if (pendingPolicePercent < 100) {
        pendingPolicePercent += 5;
        if (pendingPolicePercent > 100) pendingPolicePercent = 100;
        updateAllocationLabels();
    }
}

void CityBudgetWindow::onPoliceDecrease() {
    if (pendingPolicePercent > 0) {
        pendingPolicePercent -= 5;
        if (pendingPolicePercent < 0) pendingPolicePercent = 0;
        updateAllocationLabels();
    }
}

void CityBudgetWindow::onFireIncrease() {
    if (pendingFirePercent < 100) {
        pendingFirePercent += 5;
        if (pendingFirePercent > 100) pendingFirePercent = 100;
        updateAllocationLabels();
    }
}

void CityBudgetWindow::onFireDecrease() {
    if (pendingFirePercent > 0) {
        pendingFirePercent -= 5;
        if (pendingFirePercent < 0) pendingFirePercent = 0;
        updateAllocationLabels();
    }
}

void CityBudgetWindow::onApply() {
    if (!validateAllocation()) return;

    currentGame->getCommandManager().addCommand(
        Command(pLocalPlayer->getPlayerID(), CMD_CITY_SET_BUDGET,
                pendingRoadPercent, pendingPolicePercent, pendingFirePercent));

    // Reset pending to current values after apply
    auto* citySim = currentGame->getCitySimulation();
    if (citySim && citySim->isInitialized()) {
        auto& budget = citySim->getCityBudget();
        pendingRoadPercent = budget.getRoadPercent();
        pendingPolicePercent = budget.getPolicePercent();
        pendingFirePercent = budget.getFirePercent();
    }

    updateDisplay();
}

bool CityBudgetWindow::validateAllocation() {
    int total = pendingRoadPercent + pendingPolicePercent + pendingFirePercent;
    if (total > 100) {
        totalAllocationLabel.setTextColor(COLOR_RED);
        applyButton.setEnabled(false);
        return false;
    }
    totalAllocationLabel.setTextColor(COLOR_WHITE);
    applyButton.setEnabled(true);
    return true;
}

void CityBudgetWindow::updateAllocationLabels() {
    roadValueLabel.setText(fmt::sprintf("%d%%", pendingRoadPercent));
    policeValueLabel.setText(fmt::sprintf("%d%%", pendingPolicePercent));
    fireValueLabel.setText(fmt::sprintf("%d%%", pendingFirePercent));

    int total = pendingRoadPercent + pendingPolicePercent + pendingFirePercent;
    totalAllocationLabel.setText(fmt::sprintf("Total: %d%%", total));

    validateAllocation();
}

void CityBudgetWindow::updateDisplay() {
    auto* citySim = currentGame->getCitySimulation();
    if (!citySim || !citySim->isInitialized()) {
        return;
    }

    treasuryLabel.setText(fmt::sprintf("Treasury: %d", citySim->getTotalFunds()));

    auto& budget = citySim->getCityBudget();
    int32_t lastRevenue = budget.getLastTaxRevenue();
    incomeLabel.setText(fmt::sprintf("Tax Revenue: %d", lastRevenue));

    expensesLabel.setText("Expenses: (calculated)");

    taxRateValueLabel.setText(fmt::sprintf("%d%%", citySim->getCityTax()));

    // Initialize pending values from current
    pendingRoadPercent = budget.getRoadPercent();
    pendingPolicePercent = budget.getPolicePercent();
    pendingFirePercent = budget.getFirePercent();
    updateAllocationLabels();

    resPopLabel.setText(fmt::sprintf("Residential: %d", citySim->getResPop()));
    comPopLabel.setText(fmt::sprintf("Commercial: %d", citySim->getComPop()));
    indPopLabel.setText(fmt::sprintf("Industrial: %d", citySim->getIndPop()));
    totalPopLabel.setText(fmt::sprintf("Total Population: %d", citySim->getTotalPop()));
}
