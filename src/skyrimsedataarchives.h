#ifndef _SKYRIMSEDATAARCHIVES_H
#define _SKYRIMSEDATAARCHIVES_H

#include "gamebryodataarchives.h"

namespace MOBase { class IProfile; }

#include <QStringList>

class SkyrimSEDataArchives : public GamebryoDataArchives
{

public:

  virtual QStringList vanillaArchives() const override;
  virtual QStringList archives(const MOBase::IProfile *profile) const override;

private:

  virtual void writeArchiveList(MOBase::IProfile *profile, const QStringList &before) override;

};

#endif // _SKYRIMSEDATAARCHIVES_H
