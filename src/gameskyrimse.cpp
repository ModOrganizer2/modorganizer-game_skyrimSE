#include "gameskyrimse.h"

#include "skyrimsedataarchives.h"
#include "skyrimsescriptextender.h"
#include "skyrimsesavegameinfo.h"
#include "skyrimseunmanagedmods.h"

#include <pluginsetting.h>
#include <executableinfo.h>
#include <gamebryolocalsavegames.h>
#include <creationgameplugins.h>
#include "versioninfo.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

#include <memory>
#include "scopeguard.h"

using namespace MOBase;

GameSkyrimSE::GameSkyrimSE()
{
}

void GameSkyrimSE::setGamePath(const QString &path)
{
    m_GamePath = path;
}

QDir GameSkyrimSE::documentsDirectory() const
{
    return m_MyGamesPath;
}

QString GameSkyrimSE::identifyGamePath() const
{
    QString path = "Software\\Bethesda Softworks\\" + gameName();
    return findInRegistry(HKEY_LOCAL_MACHINE, path.toStdWString().c_str(), L"Installed Path");
}

QDir GameSkyrimSE::savesDirectory() const
{
    return QDir(m_MyGamesPath + "/Saves");
}

QString GameSkyrimSE::myGamesPath() const
{
    return m_MyGamesPath;
}

bool GameSkyrimSE::isInstalled() const
{
    return !m_GamePath.isEmpty();
}

bool GameSkyrimSE::init(IOrganizer *moInfo)
{
    if (!GameGamebryo::init(moInfo)) {
        return false;
    }

    m_GamePath = GameSkyrimSE::identifyGamePath();
    m_MyGamesPath = determineMyGamesPath(gameName());

    registerFeature<ScriptExtender>(new SkyrimSEScriptExtender(this));
    registerFeature<DataArchives>(new SkyrimSEDataArchives(myGamesPath()));
    registerFeature<LocalSavegames>(new GamebryoLocalSavegames(myGamesPath(), "Skyrim.ini"));
    registerFeature<SaveGameInfo>(new SkyrimSESaveGameInfo(this));
    registerFeature<GamePlugins>(new CreationGamePlugins(moInfo));
    registerFeature<UnmanagedMods>(new SkyrimSEUnmangedMods(this));

    return true;
}



QString GameSkyrimSE::gameName() const
{
    return "Skyrim Special Edition";
}

QList<ExecutableInfo> GameSkyrimSE::executables() const
{
  return QList<ExecutableInfo>()
    << ExecutableInfo("SKSE", findInGameFolder(feature<ScriptExtender>()->loaderName()))
    << ExecutableInfo("Skyrim Special Edition", findInGameFolder(binaryName()))
    << ExecutableInfo("Skyrim Special Edition Launcher", findInGameFolder(getLauncherName()))
    << ExecutableInfo("Creation Kit", findInGameFolder("CreationKit.exe"))
    << ExecutableInfo("LOOT", getLootPath()).withArgument("--game=\"Skyrim Special Edition\"")
    ;
}

QFileInfo GameSkyrimSE::findInGameFolder(const QString &relativePath) const
{
    return QFileInfo(m_GamePath + "/" + relativePath);
}

QString GameSkyrimSE::name() const
{
    return "Skyrim Special Edition Support Plugin";
}

QString GameSkyrimSE::author() const
{
    return "Archost & ZachHaber";
}

QString GameSkyrimSE::description() const
{
    return tr("Adds support for the game Skyrim Special Edition.");
}

MOBase::VersionInfo GameSkyrimSE::version() const
{
    return VersionInfo(0, 1, 5, VersionInfo::RELEASE_ALPHA);
}

bool GameSkyrimSE::isActive() const
{
    return qApp->property("managed_game").value<IPluginGame*>() == this;
}

QList<PluginSetting> GameSkyrimSE::settings() const
{
    return QList<PluginSetting>();
}

void GameSkyrimSE::initializeProfile(const QDir &path, ProfileSettings settings) const
{
    if (settings.testFlag(IPluginGame::MODS)) {
        copyToProfile(localAppFolder() + "/Skyrim Special Edition", path, "plugins.txt");
        copyToProfile(localAppFolder() + "/Skyrim Special Edition", path, "loadorder.txt");
    }

    if (settings.testFlag(IPluginGame::CONFIGURATION)) {
        if (settings.testFlag(IPluginGame::PREFER_DEFAULTS)
            || !QFileInfo(myGamesPath() + "/skyrim.ini").exists()) {
            copyToProfile(gameDirectory().absolutePath(), path, "skyrim_default.ini", "skyrim.ini");
        }
        else {
            copyToProfile(myGamesPath(), path, "skyrim.ini");
        }

        copyToProfile(myGamesPath(), path, "skyrimprefs.ini");
    }
}

QString GameSkyrimSE::savegameExtension() const
{
    return "ess";
}

QString GameSkyrimSE::savegameSEExtension() const
{
    return "skse";
}

QString GameSkyrimSE::steamAPPId() const
{
    return "489830";
}

QStringList GameSkyrimSE::primaryPlugins() const {
  QStringList plugins = { "skyrim.esm", "update.esm", "dawnguard.esm", "hearthfires.esm", "dragonborn.esm" };

  plugins.append(CCPlugins());

  return plugins;
}

QStringList GameSkyrimSE::gameVariants() const
{
    return{ "Regular" };
}

QString GameSkyrimSE::gameShortName() const
{
    return "skyrimse";
}

QStringList GameSkyrimSE::validShortNames() const
{
  return { "skyrim" };
}

QString GameSkyrimSE::gameNexusName() const
{
    return "skyrimspecialedition";
}

QStringList GameSkyrimSE::iniFiles() const
{
    return{ "skyrim.ini", "skyrimprefs.ini" };
}

QStringList GameSkyrimSE::DLCPlugins() const
{
    return{ "dawnguard.esm", "hearthfires.esm", "dragonborn.esm" };
}

QStringList GameSkyrimSE::CCPlugins() const
{
  QStringList plugins = {};
  QFile file(gameDirectory().filePath("Skyrim.ccc"));
  if (file.open(QIODevice::ReadOnly)) {
    ON_BLOCK_EXIT([&file]() { file.close(); });

    if (file.size() == 0) {
      return plugins;
    }
    while (!file.atEnd()) {
      QByteArray line = file.readLine().trimmed();
      QString modName;
      if ((line.size() > 0) && (line.at(0) != '#')) {
        modName = QString::fromUtf8(line.constData()).toLower();
      }

      if (modName.size() > 0) {
        if (!plugins.contains(modName, Qt::CaseInsensitive)) {
          plugins.append(modName);
        }
      }
    }
  }
  return plugins;
}

IPluginGame::LoadOrderMechanism GameSkyrimSE::loadOrderMechanism() const
{
    return IPluginGame::LoadOrderMechanism::PluginsTxt;
}

int GameSkyrimSE::nexusModOrganizerID() const
{
    return 6194; //... Should be 0?
}

int GameSkyrimSE::nexusGameID() const
{
    return 1704; //1704
}

QDir GameSkyrimSE::gameDirectory() const
{
    return QDir(m_GamePath);
}

// Not to delete all the spaces...
MappingType GameSkyrimSE::mappings() const
{
    MappingType result;

    for (const QString &profileFile : { "plugins.txt", "loadorder.txt" }) {
        result.push_back({ m_Organizer->profilePath() + "/" + profileFile,
            localAppFolder() + "/" + gameName() + "/" + profileFile,
            false });
    }

    return result;
}

