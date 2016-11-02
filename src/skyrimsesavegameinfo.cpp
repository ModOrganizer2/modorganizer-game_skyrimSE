#include "skyrimSEsavegameinfo.h"

#include "skyrimSEsavegame.h"
#include "gamegamebryo.h"

SkyrimSESaveGameInfo::SkyrimSESaveGameInfo(GameGamebryo const *game) :
  GamebryoSaveGameInfo(game)
{
}

SkyrimSESaveGameInfo::~SkyrimSESaveGameInfo()
{
}

const MOBase::ISaveGame *SkyrimSESaveGameInfo::getSaveGameInfo(const QString &file) const
{
  return new SkyrimSESaveGame(file, m_Game);
}

