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

#ifndef MODEDITORMENU_H
#define MODEDITORMENU_H

#include "MenuBase.h"
#include <GUI/StaticContainer.h>
#include <GUI/VBox.h>
#include <GUI/HBox.h>
#include <GUI/TextButton.h>
#include <GUI/TextBox.h>
#include <GUI/Label.h>
#include <GUI/Spacer.h>

#include <string>

/**
 * Simple menu for editing mod name/version and showing file locations
 */
class ModEditorMenu final : public MenuBase {
public:
    explicit ModEditorMenu(const std::string& modName);
    virtual ~ModEditorMenu();

    ModEditorMenu(const ModEditorMenu&) = delete;
    ModEditorMenu(ModEditorMenu&&) = delete;
    ModEditorMenu& operator=(const ModEditorMenu&) = delete;
    ModEditorMenu& operator=(ModEditorMenu&&) = delete;

private:
    void onSave();
    void onCancel();
    void loadModInfo();
    void saveModInfo();
    
    std::string modName;
    std::string modPath;

    // Main layout
    StaticContainer windowWidget;
    VBox mainVBox;
    Label titleLabel;
    
    // Mod Info
    HBox nameRow;
    Label displayNameLabel;
    TextBox displayNameTextBox;
    
    HBox versionRow;
    Label versionLabel;
    TextBox versionTextBox;
    
    // Instructions
    Label instructionsLabel1;
    Label instructionsLabel2;
    Label instructionsLabel3;
    Label pathLabel;
    
    // Files info
    Label filesLabel;
    Label objectDataLabel;
    Label quantBotLabel;
    Label gameOptionsLabel;
    
    // Bottom buttons
    HBox buttonHBox;
    TextButton saveButton;
    TextButton cancelButton;
    
    bool modified;
};

#endif // MODEDITORMENU_H
