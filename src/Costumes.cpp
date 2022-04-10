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

#include "Costumes.h"
#include "Config.h"

struct Costume
{
    uint32 itemEntry;
    uint32 displayId;
    uint32 soundId;
    float scale;
    int32 duration;

    Costume(uint32 itemEntry, uint32 displayId, uint32 soundId, float scale = 1.0f, int32 duration = -1)
        : itemEntry(itemEntry),
          displayId(displayId),
          soundId(soundId),
          scale(scale),
          duration(duration)
    {
    }
};

struct PlayerMorph
{
    Costume* costume;
    int32 startDelay;
    int32 durationLeft;
    bool morphed;

    PlayerMorph(Costume* costume, int32 startDelay, int32 durationLeft, bool morphed)
        : costume(costume),
          startDelay(startDelay),
          durationLeft(durationLeft),
          morphed(morphed)
    {
    }
};

Costumes::Costumes()
    : PlayerScript("CostumesPlayerScript"),
      WorldScript("CostumesWorldScript"),
      enabled(false),
      costumeSpellId(0),
      defaultDuration(0),
      canUseInCombat(false)
{
}

bool Costumes::CanUseItem(Player *player, ItemTemplate const *item, InventoryResult &result)
{
    if (!enabled || !player || !item || (uint32)item->Spells[0].SpellId != (uint32)costumeSpellId)
    {
        return true;
    }

    if (!canUseInCombat && player->IsInCombat())
    {
        result = InventoryResult::EQUIP_ERR_CANT_DO_RIGHT_NOW;
        return false;
    }

    uint32 cd = player->GetSpellCooldownDelay(costumeSpellId);
    if (cd)
    {
        cd /= 1000;
        uint32 hours = cd / 3600;
        uint32 minutes = (cd % 3600) / 60;
        uint32 seconds = cd % 60;
        char formattedTime[9]{};
        if (hours)
        {
            sprintf(formattedTime, "%02u:%02u:%02u", hours, minutes, seconds);
        }
        else if (minutes)
        {
            sprintf(formattedTime, "%02u:%02u", minutes, seconds);
        }
        else
        {
            sprintf(formattedTime, "%02us", seconds);
        }
        player->GetSession()->SendNotification("Cooldown: %s", formattedTime);

        result = InventoryResult::EQUIP_ERR_CANT_DO_RIGHT_NOW;
        return false;
    }

    Costume *costume = costumes[item->ItemId];
    if (!costume)
    {
        return true;
    }

    if (player->GetDisplayId() == costume->displayId) // if the morph is active -> demoprh
    {
        DemorphPlayer(player);
        return true;
    }

    float currentScale = player->GetObjectScale();
    ObjectGuid guid = player->GetGUID();
    if (playerMorphs.find(guid) != playerMorphs.end())
    {
        // Already has a morph, remove it before applying the new one
        DemorphPlayer(player);
    }

    bool delay = std::abs(currentScale - costume->scale) > 0.00001;
    if (!delay)
    {
        ApplyCostume(player, costume);
    }
    else
    {
        player->SetObjectScale(costume->scale);
    }

    // Add morph timer
    int32 startDelay = delay ? 2 : -1;
    int32 duration = (costume->duration < 0 ? static_cast<int32>(defaultDuration) : costume->duration) + startDelay;
    playerMorphs[guid] = new PlayerMorph(costume, startDelay * IN_MILLISECONDS, duration * IN_MILLISECONDS, !delay);

    return true;
}

void Costumes::ApplyCostume(Player* player, Costume* costume)
{
    player->SetDisplayId(costume->displayId);
    player->SetObjectScale(costume->scale);
    player->CastSpell(player, visualSpell, TRIGGERED_FULL_MASK);
    if (costume->soundId != 0)
    {
        player->PlayDistanceSound(costume->soundId);
    }
}

void Costumes::OnPlayerEnterCombat(Player *player, Unit * /* enemy */)
{
    if (!enabled || !player || canUseInCombat || playerMorphs.count(player->GetGUID()) == 0)
    {
        return;
    }

    playerMorphs[player->GetGUID()]->durationLeft = 0;
}

void Costumes::OnUpdate(uint32 diff)
{
    if (!enabled)
    {
        return;
    }

    int32 dt = static_cast<int32>(diff);
    for (auto it = playerMorphs.begin(); it != playerMorphs.end(); ++it)
    {
        ObjectGuid playerGuid = it->first;
        PlayerMorph *morph = it->second;

        if (!morph->morphed)
        {
            morph->startDelay -= dt;
            if (morph->startDelay <= 0)
            {
                Player* player = ObjectAccessor::FindPlayer(playerGuid);
                ApplyCostume(player, morph->costume);
                morph->morphed = true;
            }
        }

        morph->durationLeft -= dt;
        if (morph->durationLeft <= 0)
        {
            Player *player = ObjectAccessor::FindPlayer(playerGuid);
            DemorphPlayer(player);
        }
    }
}

void Costumes::OnStartup()
{
    LoadConfig();
}

void Costumes::OnShutdown()
{
    UnloadCostumes();
}

void Costumes::OnAfterConfigLoad(bool /* reload */)
{
    LoadConfig();
}

void Costumes::DemorphPlayer(Player *player)
{
    if (!player)
    {
        return;
    }

    player->DeMorph();
    player->SetObjectScale(1.0f);
    player->CastSpell(player, visualSpell, TRIGGERED_FULL_MASK);

    ObjectGuid guid = player->GetGUID();
    PlayerMorph* morph = playerMorphs[guid];
    playerMorphs.erase(guid);
    delete morph;
}

void Costumes::LoadConfig()
{
    enabled = sConfigMgr->GetOption("Costumes.Enabled", true);
    costumeSpellId = sConfigMgr->GetOption<uint32>("Costumes.SpellId", 18282);
    defaultDuration = sConfigMgr->GetOption<uint32>("Costumes.Duration", 60);
    canUseInCombat = sConfigMgr->GetOption("Costumes.CanUseInCombat", false);

    LoadCostumes();
}

void Costumes::LoadCostumes()
{
    if (!enabled)
    {
        return;
    }

    UnloadCostumes();

    QueryResult result = WorldDatabase.Query("SELECT item_entry, display_id, sound_id, scale, duration FROM costume");
    if (!result)
    {
        return;
    }

    do
    {
        int col = 0;
        Field *fields = result->Fetch();
        uint32 itemEntry = fields[col++].Get<uint32>();
        uint32 displayId = fields[col++].Get<uint32>();
        uint32 soundId = fields[col++].Get<uint32>();
        float scale = fields[col++].Get<float>();
        int32 duration = fields[col++].Get<int32>();

        Costume *costume = new Costume(itemEntry, displayId, soundId, scale, duration);
        costumes.insert(std::make_pair(itemEntry, costume));
    } while (result->NextRow());

    return;
}

void Costumes::UnloadCostumes()
{
    for (const auto &pair : costumes)
    {
        delete pair.second;
    }
    costumes.clear();
}

void AddCostumesScripts()
{
    new Costumes();
}
