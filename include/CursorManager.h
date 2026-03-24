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

#ifndef CURSORMANAGER_H
#define CURSORMANAGER_H

#include <SDL.h>
#include <vector>

class CursorManager {
private:
    SDL_Cursor* normalCursor;
    SDL_Cursor* moveCursor;
    SDL_Cursor* attackCursor;
    SDL_Cursor* captureCursor;
    SDL_Cursor* carryallDropCursor;
    bool initialized;
    
public:
    CursorManager();
    ~CursorManager();
    
    void initialize();
    void cleanup();
    void setCursorMode(int mode);
    bool canSetCursorMode(int mode, const std::vector<Uint32>& selectedObjects);
    bool isInitialized() const { return initialized; }
};

#endif // CURSORMANAGER_H
