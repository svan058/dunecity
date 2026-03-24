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

#include <units/Ornithopter.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <Map.h>
#include <House.h>
#include <Game.h>
#include <SoundPlayer.h>
#include <structures/StructureBase.h>
#include <players/QuantBotConfig.h>
#include <sand.h>
#include <Definitions.h>

#include <fstream>
#include <mutex>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <iterator>

#define ORNITHOPTER_FRAMETIME 3

namespace {

bool isHumanControlledHouse(const House* house) {
    if(house == nullptr) {
        return false;
    }

    for(const auto& playerPtr : house->getPlayerList()) {
        if(playerPtr && playerPtr->getPlayerclass() == HUMANPLAYERCLASS) {
            return true;
        }
    }

    return false;
}

void appendOrnithopterHuntLog(const std::string& message) {
    static std::mutex logMutex;
    static std::once_flag initFlag;
    static std::string logPath;

    // Lazily determine log file location and clear it once per run.
    auto initLogPath = []() {
        std::string configPath = getQuantBotConfigFilepath();
        if(configPath.empty()) {
            return std::string();
        }

        // Get the directory containing the config folder, not the config folder itself
        const std::string::size_type slashPos = configPath.find_last_of("/\\");
        if(slashPos == std::string::npos) {
            return std::string();
        }
        std::string directory = configPath.substr(0, slashPos);
        
        // Get parent directory (one level up from config/)
        const std::string::size_type parentSlashPos = directory.find_last_of("/\\");
        if(parentSlashPos != std::string::npos) {
            directory = directory.substr(0, parentSlashPos);
        }
        
        std::string path = directory + "/ornithopter-hunt.log";

        std::ofstream clearStream(path, std::ios::trunc);
        if(clearStream) {
            auto now = std::chrono::system_clock::now();
            std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
            clearStream << "=== Ornithopter hunt diagnostics started "
                        << std::put_time(std::localtime(&nowTime), "%Y-%m-%d %H:%M:%S")
                        << " ===\n";
        }

        return path;
    };

    std::call_once(initFlag, [&]() { logPath = initLogPath(); });

    if(logPath.empty() || message.empty()) {
        return;
    }

    std::lock_guard<std::mutex> guard(logMutex);
    std::ofstream stream(logPath, std::ios::app);
    if(!stream) {
        return;
    }

    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

    stream << std::put_time(std::localtime(&nowTime), "%H:%M:%S")
           << " [cycle " << (currentGame ? currentGame->getGameCycleCount() : 0) << "] "
           << message << '\n';
}

} // namespace

Ornithopter::Ornithopter(House* newOwner) : AirUnit(newOwner) {

    Ornithopter::init();

    setHealth(getMaxHealth());

    timeLastShot = 0;
}

Ornithopter::Ornithopter(InputStream& stream) : AirUnit(stream) {
    Ornithopter::init();

    timeLastShot = stream.readUint32();
}

void Ornithopter::init() {
    itemID = Unit_Ornithopter;
    owner->incrementUnits(itemID);

    graphicID = ObjPic_Ornithopter;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
    shadowGraphic = pGFXManager->getObjPic(ObjPic_OrnithopterShadow,getOwner()->getHouseID());

    numImagesX = NUM_ANGLES;
    numImagesY = 3;

    numWeapons = 1;
    bulletType = Bullet_SmallRocket;

    currentMaxSpeed = currentGame->objectData.data[itemID][originalHouseID].maxspeed;

    static bool loggedInit = false;
    if(!loggedInit) {
        appendOrnithopterHuntLog("Ornithopter hunt logger initialized");
        loggedInit = true;
    }
}

Ornithopter::~Ornithopter() = default;

void Ornithopter::save(OutputStream& stream) const
{
    AirUnit::save(stream);

    stream.writeUint32(timeLastShot);
}

void Ornithopter::checkPos() {
    AirUnit::checkPos();

    if(!target) {
        if(destination.isValid()) {
            if(blockDistance(location, destination) <= 2) {
                destination.invalidate();
            }
        } else {
            if(blockDistance(location, guardPoint) > 17) {
                setDestination(guardPoint);
            }
        }
    }

    drawnFrame = ((currentGame->getGameCycleCount() + getObjectID())/ORNITHOPTER_FRAMETIME) % numImagesY;
}

bool Ornithopter::canAttack(const ObjectBase* object) const {
    if ((object != nullptr) && !object->isAFlyingUnit()
        && ((object->getOwner()->getTeamID() != owner->getTeamID()) || object->getItemID() == Unit_Sandworm)
        && object->isVisible(getOwner()->getTeamID()))
        return true;
    else
        return false;
}

void Ornithopter::destroy() {
    // place wreck
    if(currentGameMap->tileExists(location)) {
        Tile* pTile = currentGameMap->getTile(location);
        pTile->assignDeadUnit(DeadUnit_Ornithopter, owner->getHouseID(), Coord(lround(realX), lround(realY)));
    }

    AirUnit::destroy();
}

void Ornithopter::playAttackSound() {
    soundPlayer->playSoundAt(Sound_Rocket,location);
}

bool Ornithopter::canPass(int xPos, int yPos) const {
    return (currentGameMap->tileExists(xPos, yPos) && (!currentGameMap->getTile(xPos, yPos)->hasAnAirUnit()));
}



FixPoint Ornithopter::getDestinationAngle() const {
    if(timeLastShot > 0 && (currentGame->getGameCycleCount() - timeLastShot) < MILLI2CYCLES(1000)) {
        // we already shot at target and now want to fly in the opposite direction
        return destinationAngleRad(destination.x*TILESIZE + TILESIZE/2, destination.y*TILESIZE + TILESIZE/2, realX, realY)*8 / (FixPt_PI << 1);
    } else {
        return destinationAngleRad(realX, realY, destination.x*TILESIZE + TILESIZE/2, destination.y*TILESIZE + TILESIZE/2)*8 / (FixPt_PI << 1);
    }
}

bool Ornithopter::attack() {
    bool bAttacked = AirUnit::attack();

    if(bAttacked) {
        timeLastShot = currentGame->getGameCycleCount();
    }
    return bAttacked;
}

const ObjectBase* Ornithopter::findTarget() const {
    appendOrnithopterHuntLog("Ornithopter::findTarget() CALLED - this function IS running");
    const ATTACKMODE mode = getAttackMode();
    if(mode != HUNT || !isHumanControlledHouse(owner)) {
        appendOrnithopterHuntLog("Skipping QuantBot path - mode or house check");
        return ObjectBase::findTarget();
    }

    const QuantBotConfig& config = getQuantBotConfig();
    const int myTeam = owner->getTeamID();

    const ObjectBase* bestTarget = nullptr;
    double bestScore = -1.0;
    int evaluatedStructures = 0;
    int evaluatedUnits = 0;
    int viableCandidates = 0;

    auto evaluateCandidate = [&](const ObjectBase* candidate, const QuantBotConfig::TargetPriority& priority, bool isStructure) {
        if(candidate == nullptr || !candidate->isActive()) {
            return;
        }

        const House* candidateOwner = candidate->getOwner();
        if(candidateOwner == nullptr || candidateOwner->getTeamID() == myTeam) {
            return;
        }

        if(!candidate->isVisible(myTeam) || !canAttack(candidate)) {
            return;
        }

        const int weight = priority.build + priority.target;
        if(weight <= 0) {
            return;
        }

        FixPoint dist = blockDistance(getLocation(), candidate->getLocation());
        const double score = static_cast<double>(weight) / (dist.toDouble() + 1.0);

        viableCandidates++;
        if(score > bestScore) {
            bestScore = score;
            bestTarget = candidate;
        }

        if(isStructure) {
            evaluatedStructures++;
        } else {
            evaluatedUnits++;
        }
    };

    for(const StructureBase* pStructure : structureList) {
        evaluateCandidate(pStructure, config.getStructurePriority(pStructure->getItemID()), true);
    }

    for(const UnitBase* pUnit : unitList) {
        if(pUnit->getOwner() == owner) {
            continue;
        }
        evaluateCandidate(pUnit, config.getUnitPriority(pUnit->getItemID()), false);
    }

    if(bestTarget != nullptr) {
        std::ostringstream stream;
        stream << "Acquired target objectId=" << bestTarget->getObjectID()
               << " itemID=" << bestTarget->getItemID()
               << " score=" << std::fixed << std::setprecision(3) << bestScore
               << " scanned(structures=" << evaluatedStructures
               << ", units=" << evaluatedUnits
               << ", viable=" << viableCandidates << ')';
        appendOrnithopterHuntLog(stream.str());
        return bestTarget;
    }

    std::ostringstream stream;
    stream << "No viable target found. Scanned structures=" << evaluatedStructures
           << " units=" << evaluatedUnits
           << " viableCandidates=" << viableCandidates;
    appendOrnithopterHuntLog(stream.str());

    return ObjectBase::findTarget();
}
