#ifndef _SKYRIMSESAVEGAME_H
#define _SKYRIMSESAVEGAME_H

#include "gamebryosavegame.h"

namespace MOBase { class IPluginGame; }

class SkyrimSESaveGame : public GamebryoSaveGame
{
public:
  SkyrimSESaveGame(QString const &fileName, MOBase::IPluginGame const *game, bool const lightEnabled = true);
};

#endif // _SKYRIMSESAVEGAME_H
