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
struct PlayerMorph;

class Costumes : public PlayerScript, WorldScript
{
public:
    Costumes();

    // PlayerScript
    bool CanUseItem(Player* player, ItemTemplate const* item, InventoryResult& result) override;
    void OnPlayerEnterCombat(Player* player, Unit* enemy) override;

    // WorldScript
    void OnUpdate(uint32 diff) override;
    void OnStartup() override;
    void OnShutdown() override;
    void OnAfterConfigLoad(bool reload) override;

private:
    void DemorphPlayer(Player* player);
    void LoadConfig();
    void LoadCostumes();
    void UnloadCostumes();

    bool enabled;
    uint32 costumeSpellId;
    uint32 defaultDuration;
    bool canUseInCombat;
    const uint32 visualSpell = 24222; // Vanish Visual
    std::map<ObjectGuid, PlayerMorph*> playerMorphs; // Map of all players currently morphed (key = player GUID)
    std::map<uint32, Costume*> costumes; // Map of all rows from the `costume` table (key = item entry)
};

#endif
