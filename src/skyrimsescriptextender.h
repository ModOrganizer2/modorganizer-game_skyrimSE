#ifndef _SKYRIMSESCRIPTEXTENDER_H
#define _SKYRIMSESCRIPTEXTENDER_H

#include "gamebryoscriptextender.h"

class GameGamebryo;

class SkyrimSEScriptExtender : public GamebryoScriptExtender
{
public:
  SkyrimSEScriptExtender(GameGamebryo const *game);

  virtual QString name() const override;

  virtual QStringList saveGameAttachmentExtensions() const override;

};

#endif // _SKYRIMSESCRIPTEXTENDER_H
