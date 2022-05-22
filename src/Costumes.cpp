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
#include "Time/GameTime.h"

struct Costume
{
    uint32 itemEntry;
    uint32 displayId;
    uint32 soundId;
    float scale;
    int32 duration;
    int64 cooldown;

    Costume(uint32 itemEntry, uint32 displayId, uint32 soundId, float scale, int32 duration, int64 cooldown)
        : itemEntry(itemEntry),
          displayId(displayId),
          soundId(soundId),
          scale(scale),
          duration(duration),
          cooldown(cooldown)
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

struct PlayerState
{
    PlayerMorph* morph;
    std::map<uint32, int64> cooldowns;

    PlayerState()
        : morph(nullptr),
          cooldowns()
    {
    }

    bool DeleteMorph()
    {
        if (morph)
        {
            delete morph;
            morph = nullptr;
            return true;
        }
        return false;
    }

    bool IsEmpty()
    {
        return morph == nullptr && cooldowns.empty();
    }

    bool UpdateCooldowns(int64 now)
    {
        bool cdRemoved = false;

        for (auto it = cooldowns.begin(); it != cooldowns.end(); ++it)
        {
            if (it->second <= now)
            {
                cooldowns.erase(it);
                cdRemoved = true;
            }
        }

        return cdRemoved;
    }
};

Costumes::Costumes()
    : PlayerScript("CostumesPlayerScript"),
      WorldScript("CostumesWorldScript"),
      enabled(false),
      costumeSpellId(0),
      defaultDuration(0),
      defaultCooldown(0),
      canUseInCombat(false),
      canUseInBg(false),
      canUseInArena(false)
{
}

bool Costumes::CanUseItem(Player *player, ItemTemplate const *item, InventoryResult &result)
{
    if (!enabled || !player || !item || (uint32)item->Spells[0].SpellId != (uint32)costumeSpellId)
    {
        return true;
    }

    if (!canUseInBg && player->InBattleground())
    {
        result = InventoryResult::EQUIP_ERR_CANT_DO_RIGHT_NOW;
        return false;
    }

    if (!canUseInArena && player->InArena())
    {
        result = InventoryResult::EQUIP_ERR_NOT_DURING_ARENA_MATCH;
        return false;
    }

    if (!canUseInCombat && player->IsInCombat())
    {
        result = InventoryResult::EQUIP_ERR_NOT_IN_COMBAT;
        return false;
    }

    Costume* costume = costumes[item->ItemId];
    if (!costume)
    {
        return true;
    }

    if (player->GetDisplayId() == costume->displayId)
    {
        // This specific costume is already active, demorph the player
        DemorphPlayer(player);
        return true;
    }

    ObjectGuid guid = player->GetGUID();
    if (IsCostumeOnCooldown(player, costume))
    {
        int64 cd = playerStates[guid]->cooldowns[costume->itemEntry] - GameTime::GetGameTimeMS().count();
        cd /= 1000;
        int64 hours = cd / 3600;
        int64 minutes = (cd % 3600) / 60;
        int64 seconds = cd % 60;
        char formattedTime[9]{};
        if (hours)
        {
            sprintf(formattedTime, "%02lld:%02lld:%02lld", hours, minutes, seconds);
        }
        else if (minutes)
        {
            sprintf(formattedTime, "%02lld:%02lld", minutes, seconds);
        }
        else
        {
            sprintf(formattedTime, "%02llds", seconds);
        }
        player->GetSession()->SendNotification("Cooldown: %s", formattedTime);

        result = InventoryResult::EQUIP_ERR_CANT_DO_RIGHT_NOW;
        return false;
    }

    float currentScale = player->GetObjectScale();
    if (IsPlayerMorphed(player))
    {
        // Already has a costume active, remove it before applying the new one
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

    int32 startDelay = delay ? 2 : 0;
    int32 duration = costume->duration < 0 ? defaultDuration : costume->duration;
    int64 cooldown = (costume->cooldown < 0 ? defaultCooldown : costume->cooldown) + startDelay;
    PlayerState *state = GetPlayerState(player);
    state->DeleteMorph();
    state->morph = new PlayerMorph(costume, startDelay * IN_MILLISECONDS, duration * IN_MILLISECONDS, !delay);
    state->cooldowns[costume->itemEntry] = GameTime::GetGameTimeMS().count() + static_cast<long long>(cooldown * IN_MILLISECONDS);

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
    if (!enabled || !player || canUseInCombat || !IsPlayerMorphed(player))
    {
        return;
    }

    playerStates[player->GetGUID()]->morph->durationLeft = 0;
}

void Costumes::OnMapChanged(Player* player)
{
    if (!player || !IsPlayerMorphed(player))
    {
        return;
    }

    PlayerState* state = GetPlayerState(player);

    if ((!canUseInBg && player->InBattleground()) || (!canUseInArena && player->InArena()))
    {
        state->morph->durationLeft = 0;
        return;
    }

    // Reapply the costume on map change since you get demorphed
    if (state && state->morph && state->morph->costume && state->morph->durationLeft > 2.0f * IN_MILLISECONDS)
    {
        state->morph->morphed = false;
        state->morph->startDelay = 1.0f * IN_MILLISECONDS; // Remorph in 1sec
    }
}

void Costumes::OnUpdate(Player* /* player */, uint32 /* p_time */) {}

void Costumes::OnUpdate(uint32 diff)
{
    if (!enabled)
    {
        return;
    }

    int32 dt = static_cast<int32>(diff);
    int64 now = GameTime::GetGameTimeMS().count();

    for (auto it = playerStates.begin(); it != playerStates.end(); ++it)
    {
        ObjectGuid guid = it->first;
        PlayerState *state = it->second;

        if (PlayerMorph* morph = state->morph)
        {
            if (!morph->morphed)
            {
                morph->startDelay -= dt;
                if (morph->startDelay <= 0)
                {
                    Player* player = ObjectAccessor::FindPlayer(guid);
                    ApplyCostume(player, morph->costume);
                    morph->morphed = true;
                }
            }
            else
            {
                morph->durationLeft -= dt;
                if (morph->durationLeft <= 0)
                {
                    Player* player = ObjectAccessor::FindPlayer(guid);
                    DemorphPlayer(player);
                }
            }
        }

        if (state->UpdateCooldowns(now) && state->IsEmpty())
        {
            delete playerStates[guid];
            playerStates.erase(guid);
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
    if (playerStates.find(guid) != playerStates.end())
    {
        playerStates[guid]->DeleteMorph();

        if (playerStates[guid]->IsEmpty())
        {
            delete playerStates[guid];
            playerStates.erase(guid);
        }
    }
}

bool Costumes::IsPlayerMorphed(Player *player)
{
    ObjectGuid guid = player->GetGUID();
    return playerStates.find(guid) != playerStates.end() && playerStates[guid]->morph != nullptr;
}

bool Costumes::IsCostumeOnCooldown(Player *player, Costume *costume)
{
    ObjectGuid guid = player->GetGUID();
    if (playerStates.find(guid) == playerStates.end())
    {
        return false;
    }

    PlayerState *state = playerStates[guid];
    if (state->cooldowns.find(costume->itemEntry) == state->cooldowns.end())
    {
        return false;
    }

    int64 now = GameTime::GetGameTimeMS().count();
    return state->cooldowns[costume->itemEntry] > now;
}

PlayerState *Costumes::GetPlayerState(Player *player)
{
    ObjectGuid guid = player->GetGUID();
    if (playerStates.find(guid) == playerStates.end())
    {
        playerStates[guid] = new PlayerState();
    }

    return playerStates[guid];
}

void Costumes::LoadConfig()
{
    enabled = sConfigMgr->GetOption("Costumes.Enabled", true);
    costumeSpellId = sConfigMgr->GetOption<uint32>("Costumes.SpellId", 18282);
    defaultDuration = sConfigMgr->GetOption<int32>("Costumes.Duration", 60);
    defaultCooldown = sConfigMgr->GetOption<int64>("Costumes.Cooldown", 240);
    canUseInCombat = sConfigMgr->GetOption("Costumes.CanUseInCombat", false);
    canUseInBg = sConfigMgr->GetOption("Costumes.CanUseInBg", false);
    canUseInArena = sConfigMgr->GetOption("Costumes.CanUseInArena", false);

    LoadCostumes();
}

void Costumes::LoadCostumes()
{
    if (!enabled)
    {
        return;
    }

    UnloadCostumes();

    QueryResult result = WorldDatabase.Query("SELECT item_entry, display_id, sound_id, scale, duration, cooldown FROM costume");
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
        int64 cooldown = fields[col++].Get<int64>();

        Costume *costume = new Costume(itemEntry, displayId, soundId, scale, duration, cooldown);
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
