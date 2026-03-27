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

#include <GUI/dune/DisasterNotification.h>

#include <globals.h>
#include <Game.h>

#include <FileClasses/FontManager.h>
#include <FileClasses/TextManager.h>
#include <misc/draw_util.h>

DisasterNotification::DisasterNotification()
 : Widget() {
    enableResizing(false, false);
    resize(300, 40);
}

DisasterNotification::~DisasterNotification() = default;

void DisasterNotification::setDisaster(DisasterType type, const std::string& message, int durationSeconds, int affectedCount) {
    disasterType_ = type;
    message_ = message;
    durationSeconds_ = durationSeconds;
    affectedCount_ = affectedCount;
    elapsed_ = 0;

    // Create background
    SDL_Color bgColor;
    switch (type) {
        case DisasterType::Fire:
            bgColor = COLOR_RGB(180, 60, 30); // Red
            break;
        case DisasterType::Sandstorm:
            bgColor = COLOR_RGB(180, 140, 60); // Orange-brown
            break;
        case DisasterType::Sandworm:
            bgColor = COLOR_RGB(140, 40, 100); // Purple
            break;
    }
    pBackground_ = renderFillRect(renderer, nullptr, bgColor);

    // Create message texture
    std::string fullMsg = message;
    if (affectedCount > 0) {
        fullMsg += " (" + std::to_string(affectedCount) + " zones)";
    }
    pMessage_ = pFontManager->createTextureWithText(fullMsg, COLOR_WHITE, 12);
}

bool DisasterNotification::handleMouseLeft(Sint32 x, Sint32 y, bool pressed) {
    if (pressed && onDismiss_) {
        onDismiss_();
    }
    return true;
}

void DisasterNotification::draw(Point position) {
    if (!pBackground_) return;

    SDL_Rect dest = { position.x, position.y, getSize().x, getSize().y };
    SDL_RenderCopy(renderer, pBackground_, nullptr, &dest);

    // Draw border
    renderDrawRect(renderer, &dest, COLOR_WHITE);

    // Draw icon (simple colored box as placeholder)
    SDL_Color iconColor;
    switch (disasterType_) {
        case DisasterType::Fire:
            iconColor = COLOR_RGB(255, 100, 30);
            break;
        case DisasterType::Sandstorm:
            iconColor = COLOR_RGB(200, 160, 80);
            break;
        case DisasterType::Sandworm:
            iconColor = COLOR_RGB(180, 60, 120);
            break;
    }
    SDL_Rect iconRect = { position.x + 5, position.y + 10, 20, 20 };
    renderFillRect(renderer, &iconRect, iconColor);

    // Draw message
    if (pMessage_) {
        SDL_Rect msgRect = { position.x + 30, position.y + 12, getSize().x - 35, 16 };
        SDL_RenderCopy(renderer, pMessage_, nullptr, &msgRect);
    }

    // Draw timer bar at bottom
    float remaining = 1.0f - (float)elapsed_ / (durationSeconds_ * 1000);
    if (remaining > 0) {
        SDL_Rect timerRect = { position.x + 1, position.y + getSize().y - 4,
                               (int)((getSize().x - 2) * remaining), 3 };
        renderFillRect(renderer, &timerRect, COLOR_WHITE);
    }
}

void DisasterNotification::update() {
    elapsed_ += 16; // Approximate frame time
    if (durationSeconds_ > 0 && elapsed_ >= durationSeconds_ * 1000) {
        if (onDismiss_) {
            onDismiss_();
        }
    }
}
