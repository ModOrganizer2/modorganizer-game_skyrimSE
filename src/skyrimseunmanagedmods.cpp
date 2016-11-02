#include "skyrimSEunmanagedmods.h"


SkyrimSEUnmangedMods::SkyrimSEUnmangedMods(const GameGamebryo *game)
  : GamebryoUnmangedMods(game)
{}

SkyrimSEUnmangedMods::~SkyrimSEUnmangedMods()
{}


//not necessary TODO: Remove
QStringList SkyrimSEUnmangedMods::secondaryFiles(const QString &modName) const {
  QStringList archives;
  QDir dataDir = game()->dataDirectory();
  for (const QString &archiveName : dataDir.entryList({modName + "*.bsa"})) {
    archives.append(dataDir.absoluteFilePath(archiveName));
  }
  return archives;
}
// not necessary TOOD: remove
QString SkyrimSEUnmangedMods::displayName(const QString &modName) const
{
  if (modName.compare("hearthfires", Qt::CaseInsensitive) == 0)
  {
    return "Hearthfire";
  } 
  else
  {
    return modName;
  }
  
}

