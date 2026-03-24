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

#ifndef RADARVIEWBASE_H
#define RADARVIEWBASE_H

#include <GUI/Widget.h>

#include <DataTypes.h>

#include <misc/SDL2pp.h>

#include <functional>
#include <algorithm>
#include <cmath>


#define NUM_STATIC_FRAMES 21
#define NUM_STATIC_FRAME_TIME 5

#define RADARVIEW_BORDERTHICKNESS 2
#define RADARWIDTH 128
#define RADARHEIGHT 128

struct RadarScaleInfo {
    float scale;        ///< pixels per tile on the radar
    int offsetX;        ///< horizontal offset inside the 128x128 radar viewport
    int offsetY;        ///< vertical offset inside the 128x128 radar viewport
    int scaledWidth;    ///< width of the scaled map area inside the radar viewport
    int scaledHeight;   ///< height of the scaled map area inside the radar viewport
};


/// This class manages the mini map at the top right corner of the screen
class RadarViewBase : public Widget
{
public:
    /**
        Constructor
    */
    RadarViewBase() : Widget(), bRadarInteraction(false) {
        resize(RADARWIDTH + (2 * RADARVIEW_BORDERTHICKNESS), RADARHEIGHT + (2 * RADARVIEW_BORDERTHICKNESS));
    }

    /**
        Destructor
    */
    virtual ~RadarViewBase() = default;

    /**
        Get the map size in x direction
        \return map width
    */
    virtual int getMapSizeX() const = 0;

    /**
        Get the map size in y direction
        \return map height
    */
    virtual int getMapSizeY() const = 0;


    /**
        Draws the radar to screen. This method is called before drawOverlay().
        \param  Position    Position to draw the radar to
    */
    inline void draw(Point position) override { ; };


    /**
        This method checks if position is inside the radar view
        \param mouseX the x-coordinate to check (relative to the top left corner of the radar)
        \param mouseY the y-coordinate to check (relative to the top left corner of the radar)
        \return true, if inside the radar view; false otherwise
    */
    bool isOnRadar(int mouseX, int mouseY) const {
        RadarScaleInfo scaleInfo = calculateScaleAndOffsets(getMapSizeX(), getMapSizeY());

        const int left = scaleInfo.offsetX + RADARVIEW_BORDERTHICKNESS;
        const int right = left + scaleInfo.scaledWidth;
        const int top = scaleInfo.offsetY + RADARVIEW_BORDERTHICKNESS;
        const int bottom = top + scaleInfo.scaledHeight;

        return ((mouseX >= left) && (mouseX < right) && (mouseY >= top) && (mouseY < bottom));
    }

    /**
        This method returns the corresponding world coordinates for a point on the radar
        \param mouseX  the position on the radar screen (relative to the top left corner of the radar)
        \param mouseY  the position on the radar screen (relative to the top left corner of the radar)
        \return the world coordinates
    */
    Coord getWorldCoords(int mouseX, int mouseY) const {
        Coord positionOnRadar(mouseX - RADARVIEW_BORDERTHICKNESS, mouseY - RADARVIEW_BORDERTHICKNESS);

        RadarScaleInfo scaleInfo = calculateScaleAndOffsets(getMapSizeX(), getMapSizeY());

        const float tileX = (positionOnRadar.x - scaleInfo.offsetX) / scaleInfo.scale;
        const float tileY = (positionOnRadar.y - scaleInfo.offsetY) / scaleInfo.scale;

        const int clampedTileX = std::clamp(static_cast<int>(std::floor(tileX)), 0, std::max(0, getMapSizeX() - 1));
        const int clampedTileY = std::clamp(static_cast<int>(std::floor(tileY)), 0, std::max(0, getMapSizeY() - 1));

        return Coord(clampedTileX * TILESIZE, clampedTileY * TILESIZE);
    }


    /**
        This method calculates the scale and the offsets that are neccessary to show a minimap centered inside a 128x128 rectangle.
        \param  MapSizeX    The width of the map in tiles
        \param  MapSizeY    The height of the map in tiles
        \param  scale       The scale factor is saved here
        \param  offsetX     The offset in x direction is saved here
        \param  offsetY     The offset in y direction is saved here
    */
    static RadarScaleInfo calculateScaleAndOffsets(int MapSizeX, int MapSizeY) {
        RadarScaleInfo info{};

        if(MapSizeX <= 0 || MapSizeY <= 0) {
            info.scale = 1.0f;
            info.offsetX = info.offsetY = 0;
            info.scaledWidth = info.scaledHeight = 0;
            return info;
        }

        constexpr float maxScale = 5.0f;
        const int maxMapSize = std::max(MapSizeX, MapSizeY);

        info.scale = std::min(maxScale, RADARWIDTH / static_cast<float>(maxMapSize));
        info.scaledWidth = std::max(1, static_cast<int>(std::lround(MapSizeX * info.scale)));
        info.scaledHeight = std::max(1, static_cast<int>(std::lround(MapSizeY * info.scale)));

        info.offsetX = (RADARWIDTH - info.scaledWidth)/2;
        info.offsetY = (RADARHEIGHT - info.scaledHeight)/2;

        return info;
    }


    /**
        Returns the minimum size of this widget. The widget should not
        resized to a size smaller than this. If the widget is not resizeable
        in a direction this method returns the size in that direction.
        \return the minimum size of this widget
    */
    Point getMinimumSize() const override { return Point(RADARWIDTH + (2 * RADARVIEW_BORDERTHICKNESS),RADARHEIGHT + (2 * RADARVIEW_BORDERTHICKNESS)); };


    /**
        Handles a mouse movement.
        \param  x               x-coordinate (relative to the left top corner of the widget)
        \param  y               y-coordinate (relative to the left top corner of the widget)
        \param  insideOverlay   true, if (x,y) is inside an overlay and this widget may be behind it, false otherwise
    */
    void handleMouseMovement(Sint32 x, Sint32 y, bool insideOverlay) override
    {
        if(bRadarInteraction && isOnRadar(x,y)) {
            if(pOnRadarClick) {
                bRadarInteraction = pOnRadarClick(getWorldCoords(x,y), false, true);
            }
        }
    }


    /**
        Handles a left mouse click.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  pressed true = mouse button pressed, false = mouse button released
        \return true = click was processed by the widget, false = click was not processed by the widget
    */
    bool handleMouseLeft(Sint32 x, Sint32 y, bool pressed) override
    {
        if(pressed) {
            if(isOnRadar(x,y)) {
                if(pOnRadarClick) {
                    bRadarInteraction = pOnRadarClick(getWorldCoords(x,y), false, false);
                }
                return true;
            }
            return false;
        } else {
            bRadarInteraction = false;
            return false;
        }
    }


    /**
        Handles a right mouse click.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  pressed true = mouse button pressed, false = mouse button released
        \return true = click was processed by the widget, false = click was not processed by the widget
    */
    bool handleMouseRight(Sint32 x, Sint32 y, bool pressed) override
    {
        if(pressed) {
            if(isOnRadar(x,y)) {
                if(pOnRadarClick) {
                    bRadarInteraction = pOnRadarClick(getWorldCoords(x,y), true, false);
                }
                return true;
            }
            return false;
        } else {
            bRadarInteraction = false;
            return false;
        }
    }


    /**
        Sets the function that should be called when the radar view is clicked.
        \param  pOnRadarClick   A function to be called on click
    */
    inline void setOnRadarClick(std::function<bool (Coord,bool,bool)> pOnRadarClick) {
        this->pOnRadarClick = pOnRadarClick;
    }

protected:
    std::function<bool (Coord,bool,bool)> pOnRadarClick;  ///< this function is called when the user clicks on the radar (1st parameter is world coordinate; 2nd parameter is whether the right mouse button was pressed; 3rd parameter is whether the mouse was moved while being pressed, e.g. dragging; return value shall be true if dragging should start or continue)

    bool bRadarInteraction;                               ///< currently dragging on the radar? (e.g. moving the view rectangle on the radar)
};

#endif // RADARVIEWBASE_H
