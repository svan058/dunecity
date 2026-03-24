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

#include <Menu/ModMenu.h>
#include <Menu/ModEditorMenu.h>

#include <globals.h>
#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <GUI/MsgBox.h>
#include <mod/ModManager.h>

#include <functional>

ModMenu::ModMenu() : MenuBase(), selectedModIndex(-1) {
    // Set up window
    SDL_Texture* pBackground = pGFXManager->getUIGraphic(UI_MenuBackground);
    setBackground(pBackground);
    resize(getTextureSize(pBackground));

    setWindowWidget(&windowWidget);

    windowWidget.addWidget(&mainVBox, 
        Point((getRendererWidth() - 560) / 2, (getRendererHeight() - 420) / 2),
        Point(560, 420));

    // Title
    titleLabel.setText(_("MODS"));
    titleLabel.setAlignment(Alignment_HCenter);
    mainVBox.addWidget(&titleLabel);
    mainVBox.addWidget(VSpacer::create(15));

    // Content area (mod list on left, details on right)
    mainVBox.addWidget(&contentHBox, 280);

    // Left side: Mod list
    contentHBox.addWidget(&modListVBox, 250);
    
    modListLabel.setText(_("Available Mods:"));
    modListVBox.addWidget(&modListLabel);
    modListVBox.addWidget(VSpacer::create(5));
    
    modListBox.setOnSelectionChange(std::bind(&ModMenu::onModListSelectionChange, this, std::placeholders::_1));
    modListVBox.addWidget(&modListBox, 230);
    
    contentHBox.addWidget(HSpacer::create(20));

    // Right side: Mod details
    contentHBox.addWidget(&detailsVBox, 270);
    
    detailsLabel.setText(_("Mod Details:"));
    detailsVBox.addWidget(&detailsLabel);
    detailsVBox.addWidget(VSpacer::create(10));
    
    modNameLabel.setText(_("Name: "));
    detailsVBox.addWidget(&modNameLabel);
    detailsVBox.addWidget(VSpacer::create(5));
    
    modAuthorLabel.setText(_("Author: "));
    detailsVBox.addWidget(&modAuthorLabel);
    detailsVBox.addWidget(VSpacer::create(5));
    
    modDescLabel.setText(_("Description: "));
    detailsVBox.addWidget(&modDescLabel);
    detailsVBox.addWidget(VSpacer::create(5));
    
    modModVersionLabel.setText(_("Mod Version: "));
    detailsVBox.addWidget(&modModVersionLabel);
    detailsVBox.addWidget(VSpacer::create(5));
    
    modGameVersionLabel.setText(_("Game Version: "));
    detailsVBox.addWidget(&modGameVersionLabel);
    detailsVBox.addWidget(VSpacer::create(5));
    
    modChecksumLabel.setText(_("Checksum: "));
    detailsVBox.addWidget(&modChecksumLabel);
    
    detailsVBox.addWidget(Spacer::create());

    mainVBox.addWidget(VSpacer::create(15));
    
    // New mod creation row
    mainVBox.addWidget(&newModHBox, 25);
    newModLabel.setText(_("New mod name:"));
    newModHBox.addWidget(&newModLabel, 110);
    newModHBox.addWidget(HSpacer::create(5));
    newModNameTextBox.setText("");
    newModHBox.addWidget(&newModNameTextBox, 200);
    newModHBox.addWidget(HSpacer::create(10));
    createButton.setText(_("CREATE"));
    createButton.setOnClick(std::bind(&ModMenu::onCreateNew, this));
    newModHBox.addWidget(&createButton, 80);
    newModHBox.addWidget(Spacer::create());

    mainVBox.addWidget(VSpacer::create(15));

    // Buttons at bottom
    mainVBox.addWidget(&buttonHBox, 25);
    
    activateButton.setText(_("ACTIVATE"));
    activateButton.setOnClick(std::bind(&ModMenu::onActivate, this));
    buttonHBox.addWidget(&activateButton, 100);
    
    buttonHBox.addWidget(HSpacer::create(10));
    
    editButton.setText(_("EDIT"));
    editButton.setOnClick(std::bind(&ModMenu::onEdit, this));
    buttonHBox.addWidget(&editButton, 70);
    
    buttonHBox.addWidget(HSpacer::create(10));
    
    deleteButton.setText(_("DELETE"));
    deleteButton.setOnClick(std::bind(&ModMenu::onDelete, this));
    buttonHBox.addWidget(&deleteButton, 80);
    
    buttonHBox.addWidget(HSpacer::create(10));
    
    backButton.setText(_("BACK"));
    backButton.setOnClick(std::bind(&ModMenu::onBack, this));
    buttonHBox.addWidget(&backButton, 70);
    
    buttonHBox.addWidget(Spacer::create());

    // Load mod list
    refreshModList();
}

ModMenu::~ModMenu() = default;

void ModMenu::refreshModList() {
    SDL_Log("ModMenu::refreshModList() - starting");
    modListBox.clearAllEntries();
    mods = ModManager::instance().listMods();
    
    SDL_Log("ModMenu::refreshModList() - got %zu mods", mods.size());
    
    std::string activeModName = ModManager::instance().getActiveModName();
    int activeIndex = -1;
    
    for (size_t i = 0; i < mods.size(); i++) {
        std::string displayName = mods[i].displayName;
        SDL_Log("ModMenu::refreshModList() - mod[%zu]: name='%s', displayName='%s'", 
                i, mods[i].name.c_str(), displayName.c_str());
        if (mods[i].name == activeModName) {
            displayName += " *";  // Mark active mod
            activeIndex = static_cast<int>(i);
        }
        modListBox.addEntry(displayName);
    }
    
    // If no mods found, show a message
    if (mods.empty()) {
        SDL_Log("ModMenu::refreshModList() - no mods found, showing placeholder");
        modListBox.addEntry(_("(No mods - enter name above)"));
        selectedModIndex = -1;
    } else {
        // Select active mod
        if (activeIndex >= 0) {
            modListBox.setSelectedItem(activeIndex);
            selectedModIndex = activeIndex;
        } else {
            modListBox.setSelectedItem(0);
            selectedModIndex = 0;
        }
    }
    
    SDL_Log("ModMenu::refreshModList() - done, selectedModIndex=%d", selectedModIndex);
    updateModDetails();
}

void ModMenu::onModListSelectionChange(bool bInteractive) {
    selectedModIndex = modListBox.getSelectedIndex();
    updateModDetails();
}

void ModMenu::updateModDetails() {
    if (selectedModIndex < 0 || selectedModIndex >= static_cast<int>(mods.size())) {
        modNameLabel.setText(_("Name: ") + std::string("-"));
        modAuthorLabel.setText(_("Author: ") + std::string("-"));
        modDescLabel.setText(_("Description: ") + std::string("-"));
        modModVersionLabel.setText(_("Mod Version: ") + std::string("-"));
        modGameVersionLabel.setText(_("Game Version: ") + std::string("-"));
        modChecksumLabel.setText(_("Checksum: ") + std::string("-"));
        return;
    }
    
    const ModInfo& mod = mods[selectedModIndex];
    modNameLabel.setText(_("Name: ") + mod.displayName);
    modAuthorLabel.setText(_("Author: ") + mod.author);
    modDescLabel.setText(_("Description: ") + mod.description);
    modModVersionLabel.setText(_("Mod Version: ") + (mod.version.empty() ? std::string("-") : mod.version));
    modGameVersionLabel.setText(_("Game Version: ") + mod.gameVersion);
    
    // Show abbreviated checksum
    std::string checksum = mod.checksums.combined;
    if (checksum.length() > 12) {
        checksum = checksum.substr(0, 12) + "...";
    }
    modChecksumLabel.setText(_("Checksum: ") + checksum);
}

void ModMenu::onActivate() {
    if (selectedModIndex < 0 || selectedModIndex >= static_cast<int>(mods.size())) {
        return;
    }
    
    const ModInfo& mod = mods[selectedModIndex];
    if (ModManager::instance().setActiveMod(mod.name)) {
        // Reload effective game options with new mod
        effectiveGameOptions = ModManager::instance().loadEffectiveGameOptions(settings.gameOptions);
        
        openWindow(MsgBox::create(_("Mod activated: ") + mod.displayName));
        refreshModList();
    } else {
        openWindow(MsgBox::create(_("Failed to activate mod.")));
    }
}

void ModMenu::onEdit() {
    if (selectedModIndex < 0 || selectedModIndex >= static_cast<int>(mods.size())) {
        return;
    }
    
    const ModInfo& mod = mods[selectedModIndex];
    
    if (mod.name == "vanilla") {
        openWindow(MsgBox::create(_("Cannot edit vanilla mod.\nCreate a new mod first.")));
        return;
    }
    
    // Open the mod editor
    ModEditorMenu modEditor(mod.name);
    modEditor.showMenu();
    
    // Refresh list in case mod was modified
    refreshModList();
}

void ModMenu::onCreateNew() {
    std::string newName = newModNameTextBox.getText();
    
    // Validate name
    if (newName.empty()) {
        openWindow(MsgBox::create(_("Please enter a mod name.")));
        return;
    }
    
    // Check for invalid characters (keep it simple - alphanumeric, spaces, dashes, underscores)
    for (char c : newName) {
        if (!isalnum(c) && c != ' ' && c != '-' && c != '_') {
            openWindow(MsgBox::create(_("Invalid character in mod name.\nUse only letters, numbers, spaces, - and _")));
            return;
        }
    }
    
    if (newName == "vanilla") {
        openWindow(MsgBox::create(_("Cannot use 'vanilla' as mod name.")));
        return;
    }
    
    if (ModManager::instance().modExists(newName)) {
        openWindow(MsgBox::create(_("A mod with that name already exists.")));
        return;
    }
    
    if (ModManager::instance().createMod(newName, "vanilla")) {
        openWindow(MsgBox::create(_("Created new mod: ") + newName + 
            "\n\n" + _("Edit the files in the mod folder to customize.")));
        newModNameTextBox.setText("");
        refreshModList();
    } else {
        openWindow(MsgBox::create(_("Failed to create mod.")));
    }
}

void ModMenu::onDelete() {
    if (selectedModIndex < 0 || selectedModIndex >= static_cast<int>(mods.size())) {
        return;
    }
    
    const ModInfo& mod = mods[selectedModIndex];
    
    if (mod.name == "vanilla") {
        openWindow(MsgBox::create(_("Cannot delete vanilla mod.")));
        return;
    }
    
    if (ModManager::instance().deleteMod(mod.name)) {
        openWindow(MsgBox::create(_("Deleted mod: ") + mod.displayName));
        refreshModList();
    } else {
        openWindow(MsgBox::create(_("Failed to delete mod.")));
    }
}

void ModMenu::onBack() {
    quit();
}
