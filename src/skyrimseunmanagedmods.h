#ifndef _SKYRIMSEUNMANAGEDMODS_H
#define _SKYRIMSEUNMANAGEDMODS_H


#include "gamebryounmanagedmods.h"
#include <gamegamebryo.h>


class SkyrimSEUnmangedMods : public GamebryoUnmangedMods {
public:
  SkyrimSEUnmangedMods(const GameGamebryo *game);
  ~SkyrimSEUnmangedMods();

  virtual QStringList secondaryFiles(const QString &modName) const override;
  virtual QString displayName(const QString &modName) const override;
};



#endif // _SKYRIMSEUNMANAGEDMODS_H
