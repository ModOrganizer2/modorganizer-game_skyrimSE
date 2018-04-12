#ifndef _SKYRIMSEGAMEPLUGINS_H
#define _SKYRIMSEGAMEPLUGINS_H


#include <gamebryogameplugins.h>
#include <iplugingame.h>
#include <imoinfo.h>
#include <map>


class SkyrimSEGamePlugins : public GamebryoGamePlugins
{
public:
  SkyrimSEGamePlugins(MOBase::IOrganizer *organizer);

protected:
  virtual void writePluginList(const MOBase::IPluginList *pluginList,
                               const QString &filePath) override;
  virtual QStringList readPluginList(MOBase::IPluginList *pluginList) override;
  virtual void getLoadOrder(QStringList &loadOrder) override;

private:
  std::map<QString, QByteArray> m_LastSaveHash;
};

#endif // _SKYRIMSEGAMEPLUGINS_H
