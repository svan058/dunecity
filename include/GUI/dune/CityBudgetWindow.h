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

#ifndef CITYBUDGETWINDOW_H
#define CITYBUDGETWINDOW_H

#include <GUI/Window.h>
#include <GUI/HBox.h>
#include <GUI/VBox.h>
#include <GUI/TextButton.h>
#include <GUI/Label.h>
#include <GUI/Spacer.h>
#include <GUI/PictureButton.h>

class CityBudgetWindow : public Window
{
public:
    CityBudgetWindow();
    virtual ~CityBudgetWindow();

    void onClose();
    void onTaxRateIncrease();
    void onTaxRateDecrease();

    /**
        This static method creates a dynamic city budget window.
        \return The new dialog box (will be automatically destroyed when it's closed)
    */
    static CityBudgetWindow* create() {
        CityBudgetWindow* dlg = new CityBudgetWindow();
        dlg->pAllocated = true;
        return dlg;
    }

private:
    void updateDisplay();

    VBox mainVBox;
    Label titleLabel;
    
    Label treasuryLabel;
    Label incomeLabel;
    Label expensesLabel;
    
    HBox taxRateHBox;
    Label taxRateLabel;
    PictureButton taxRateMinus;
    Label taxRateValueLabel;
    PictureButton taxRatePlus;
    
    Label resPopLabel;
    Label comPopLabel;
    Label indPopLabel;
    Label totalPopLabel;
    
    TextButton closeButton;
};

#endif // CITYBUDGETWINDOW_H
