#ifndef _GAMESKYRIMSE_H
#define _GAMESKYRIMSE_H


#include "gamegamebryo.h"

#include <QObject>
#include <QtGlobal>

class GameSkyrimSE : public GameGamebryo
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "com.soundcontactstudio.GameSkyrimSE" FILE "gameskyrimse.json")

public:

    GameSkyrimSE();

    virtual bool init(MOBase::IOrganizer *moInfo) override;

public: // IPluginGame interface

    virtual QString gameName() const override;

    virtual QList<MOBase::ExecutableInfo> executables() const override;
    virtual QList<MOBase::ExecutableForcedLoadSetting> executableForcedLoads() const override;
    virtual void initializeProfile(const QDir &path, ProfileSettings settings) const override;
    virtual QString savegameExtension() const override;
    virtual QString savegameSEExtension() const override;
    virtual QString steamAPPId() const override;
    virtual QStringList primaryPlugins() const override;
    virtual QStringList gameVariants() const override;
    virtual QString gameShortName() const override;
    virtual QString gameNexusName() const override;
    virtual QStringList validShortNames() const override;
    virtual QStringList iniFiles() const override;
    virtual QStringList DLCPlugins() const override;
    virtual QStringList CCPlugins() const override;
    virtual LoadOrderMechanism loadOrderMechanism() const override;
    virtual int nexusModOrganizerID() const override;
    virtual int nexusGameID() const override;

    virtual bool isInstalled() const override;
    virtual void setGamePath(const QString &path) override;
    virtual QDir gameDirectory() const override;

public: // IPlugin interface

    virtual QString name() const override;
    virtual QString author() const override;
    virtual QString description() const override;
    virtual MOBase::VersionInfo version() const override;
    virtual bool isActive() const override;
    virtual QList<MOBase::PluginSetting> settings() const override;
    virtual MappingType mappings() const override;


protected:
    QDir documentsDirectory() const;
    QDir savesDirectory() const;
    QFileInfo findInGameFolder(const QString &relativePath) const;
    QString myGamesPath() const;

    virtual QString identifyGamePath() const override;

};

#endif // _GAMESKYRIMSE_H
