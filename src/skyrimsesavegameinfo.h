#ifndef _SKYRIMSAVEGAMEINFO_H
#define _SKYRIMSAVEGAMEINFO_H

#include "gamebryosavegameinfo.h"

class GameGamebryo;

class SkyrimSESaveGameInfo : public GamebryoSaveGameInfo
{
public:
  SkyrimSESaveGameInfo(GameGamebryo const *game);
  ~SkyrimSESaveGameInfo();

  virtual MOBase::ISaveGame const *getSaveGameInfo(QString const &file) const override;
};

#endif // _SKYRIMSAVEGAMEINFO_H
