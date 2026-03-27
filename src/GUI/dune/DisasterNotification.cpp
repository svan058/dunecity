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
    dismissed_ = false;

    // Set background color
    switch (type) {
        case DisasterType::Fire:
            bgColor_ = COLOR_RGB(180, 60, 30); // Red
            break;
        case DisasterType::Sandstorm:
            bgColor_ = COLOR_RGB(180, 140, 60); // Orange-brown
            break;
        case DisasterType::Sandworm:
            bgColor_ = COLOR_RGB(140, 40, 100); // Purple
            break;
    }

    // Create message texture
    std::string fullMsg = message;
    if (affectedCount > 0) {
        fullMsg += " (" + std::to_string(affectedCount) + " zones)";
    }
    pMessage_ = pFontManager->createTextureWithText(fullMsg, COLOR_WHITE, 12);

    // Countdown text
    int remaining = std::max(0, durationSeconds - elapsed_ / 1000);
    countdownText_ = std::to_string(remaining) + "s";
    pCountdown_ = pFontManager->createTextureWithText(countdownText_, COLOR_WHITE, 10);
}

bool DisasterNotification::handleMouseLeft(Sint32 x, Sint32 y, bool pressed) {
    if (pressed) {
        dismiss();
    }
    return true;
}

void DisasterNotification::draw(Point position) {
    if (dismissed_) return;

    SDL_Rect dest = { position.x, position.y, getSize().x, getSize().y };

    // Draw background
    setRenderDrawColor(renderer, bgColor_);
    SDL_RenderFillRect(renderer, &dest);

    // Draw border
    renderDrawRect(renderer, &dest, COLOR_WHITE);

    // Draw icon (simple colored box as placeholder)
    Uint32 iconColor;
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
    setRenderDrawColor(renderer, iconColor);
    SDL_RenderFillRect(renderer, &iconRect);

    // Draw message
    if (pMessage_) {
        SDL_Rect msgRect = { position.x + 30, position.y + 8, getSize().x - 80, 14 };
        SDL_RenderCopy(renderer, pMessage_.get(), nullptr, &msgRect);
    }

    // Draw countdown timer on the right
    if (pCountdown_) {
        SDL_Rect countdownRect = { position.x + getSize().x - 35, position.y + 13, 30, 14 };
        SDL_RenderCopy(renderer, pCountdown_.get(), nullptr, &countdownRect);
    }

    // Draw timer bar at bottom
    float remainingRatio = 1.0f;
    if (durationSeconds_ > 0) {
        remainingRatio = std::max(0.0f, 1.0f - (float)elapsed_ / (durationSeconds_ * 1000));
    }
    if (remainingRatio > 0) {
        SDL_Rect timerRect = { position.x + 1, position.y + getSize().y - 4,
                               (int)((getSize().x - 2) * remainingRatio), 3 };
        setRenderDrawColor(renderer, COLOR_WHITE);
        SDL_RenderFillRect(renderer, &timerRect);
    }
}

void DisasterNotification::update() {
    elapsed_ += 16; // Approximate frame time
    if (durationSeconds_ > 0 && elapsed_ >= durationSeconds_ * 1000) {
        dismiss();
    }

    // Update countdown texture every second
    if (elapsed_ % 1000 < 20) {
        int secs = std::max(0, durationSeconds_ - elapsed_ / 1000);
        countdownText_ = std::to_string(secs) + "s";
        pCountdown_ = pFontManager->createTextureWithText(countdownText_, COLOR_WHITE, 10);
    }
}
