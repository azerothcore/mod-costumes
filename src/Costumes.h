/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MOD_COSTUMES_H_
#define _MOD_COSTUMES_H_

#include <map>

#include "ScriptMgr.h"
#include "Player.h"

struct Costume;
struct PlayerState;
struct PlayerMorph;

class Costumes : public PlayerScript, WorldScript, UnitScript
{
public:
    Costumes();

    // PlayerScript
    bool OnPlayerCanUseItem(Player *player, ItemTemplate const *item, InventoryResult &result) override;
    void OnPlayerEnterCombat(Player *player, Unit *enemy) override;
    void OnPlayerMapChanged(Player *player) override;
    void OnPlayerUpdate(Player *player, uint32 p_time) override;

    // WorldScript
    void OnUpdate(uint32 diff) override;
    void OnStartup() override;
    void OnShutdown() override;
    void OnAfterConfigLoad(bool reload) override;

    // UnitScript
    void OnDisplayIdChange(Unit *unit, uint32 displayId) override;

private:
    void ApplyCostume(Player *player, Costume *costume);
    void DemorphPlayer(Player *player);
    bool IsPlayerMorphed(Player *player);
    bool IsCostumeOnCooldown(Player *player, Costume *costume);
    PlayerState *GetPlayerState(Player *player);
    void ResetScale(Player *player);
    void LoadConfig();
    void LoadCostumes();
    void UnloadCostumes();

    bool enabled;
    uint32 costumeSpellId;
    int32 defaultDuration;
    int64 defaultCooldown;
    bool canUseInCombat;
    bool canUseInPvp;
    bool canUseInBg;
    bool canUseInArena;
    const uint32 visualSpell = 24222;                 // Vanish visual effect
    std::map<ObjectGuid, PlayerState *> playerStates; // Map of players' states (current morph and cooldowns) (key = player GUID)
    std::map<uint32, Costume *> costumes;             // Map of all rows from the `costume` table (key = item entry)
};

#endif
