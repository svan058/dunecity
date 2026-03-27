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

#ifndef DISASTERNOTIFICATION_H
#define DISASTERNOTIFICATION_H

#include <GUI/Widget.h>
#include <misc/SDL2pp.h>

#include <string>
#include <cstdint>

enum class DisasterType {
    Fire,
    Sandstorm,
    Sandworm
};

class DisasterNotification : public Widget {
public:
    DisasterNotification();
    virtual ~DisasterNotification();

    void setDisaster(DisasterType type, const std::string& message, int durationSeconds, int affectedCount);

    void setOnDismiss(std::function<void()> onDismiss) { onDismiss_ = onDismiss; }

    bool handleMouseLeft(Sint32 x, Sint32 y, bool pressed) override;

    void draw(Point position) override;

    Point getMinimumSize() const override {
        return Point(300, 40);
    }

    void update();

private:
    DisasterType disasterType_;
    std::string message_;
    int durationSeconds_;
    int affectedCount_;
    int elapsed_ = 0;
    bool dismissed_ = false;
    std::function<void()> onDismiss_;

    Uint32 bgColor_;
    sdl2::texture_ptr pMessage_;
    sdl2::texture_ptr pCountdown_;
    std::string countdownText_;

public:
    bool isDismissed() const { return dismissed_; }
    void dismiss() { dismissed_ = true; if(onDismiss_) onDismiss_(); }
};

#endif //DISASTERNOTIFICATION_H
