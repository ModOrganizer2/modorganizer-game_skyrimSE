#include "skyrimSEscriptextender.h"

#include <QString>
#include <QStringList>

SkyrimSEScriptExtender::SkyrimSEScriptExtender(GameGamebryo const *game) :
  GamebryoScriptExtender(game)
{
}

QString SkyrimSEScriptExtender::name() const
{
  return "sksese";
}

QStringList SkyrimSEScriptExtender::saveGameAttachmentExtensions() const
{
  return { };
}
