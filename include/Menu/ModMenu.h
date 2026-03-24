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

#ifndef MODMENU_H
#define MODMENU_H

#include "MenuBase.h"
#include <GUI/StaticContainer.h>
#include <GUI/VBox.h>
#include <GUI/HBox.h>
#include <GUI/TextButton.h>
#include <GUI/TextBox.h>
#include <GUI/ListBox.h>
#include <GUI/Label.h>
#include <GUI/Spacer.h>

#include <mod/ModInfo.h>
#include <vector>

/**
 * Menu for browsing, selecting, creating, and deleting mods.
 */
class ModMenu final : public MenuBase {
public:
    ModMenu();
    virtual ~ModMenu();

    ModMenu(const ModMenu&) = delete;
    ModMenu(ModMenu&&) = delete;
    ModMenu& operator=(const ModMenu&) = delete;
    ModMenu& operator=(ModMenu&&) = delete;

private:
    void onActivate();
    void onEdit();
    void onCreateNew();
    void onDelete();
    void onBack();
    void onModListSelectionChange(bool bInteractive);
    void refreshModList();
    void updateModDetails();

    StaticContainer windowWidget;
    
    // Main layout
    VBox mainVBox;
    Label titleLabel;
    
    // Content area
    HBox contentHBox;
    
    // Mod list (left side)
    VBox modListVBox;
    Label modListLabel;
    ListBox modListBox;
    
    // New mod input
    HBox newModHBox;
    Label newModLabel;
    TextBox newModNameTextBox;
    TextButton createButton;
    
    // Mod details (right side)
    VBox detailsVBox;
    Label detailsLabel;
    Label modNameLabel;
    Label modAuthorLabel;
    Label modDescLabel;
    Label modModVersionLabel;
    Label modGameVersionLabel;
    Label modChecksumLabel;
    
    // Buttons
    HBox buttonHBox;
    TextButton activateButton;
    TextButton editButton;
    TextButton deleteButton;
    TextButton backButton;
    
    // Data
    std::vector<ModInfo> mods;
    int selectedModIndex;
};

#endif // MODMENU_H
