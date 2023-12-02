#include "gameskyrimse.h"

#include "skyrimsedataarchives.h"
#include "skyrimsemoddatachecker.h"
#include "skyrimsemoddatacontent.h"
#include "skyrimsesavegame.h"
#include "skyrimsescriptextender.h"
#include "skyrimseunmanagedmods.h"

#include "versioninfo.h"
#include <creationgameplugins.h>
#include <executableinfo.h>
#include <gamebryolocalsavegames.h>
#include <gamebryosavegameinfo.h>
#include <pluginsetting.h>
#include <utility.h>

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QJsonDocument>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

#include "scopeguard.h"
#include <memory>

using namespace MOBase;

GameSkyrimSE::GameSkyrimSE()
{
}

void GameSkyrimSE::setVariant(QString variant)
{
  m_GameVariant = variant;
}

void GameSkyrimSE::checkVariants()
{
  QFileInfo gog_dll(m_GamePath + "\\Galaxy64.dll");
  QFileInfo epic_dll(m_GamePath + "\\EOSSDK-Win64-Shipping.dll");
  if (gog_dll.exists())
    setVariant("GOG");
  else if (epic_dll.exists())
    setVariant("Epic Games");
  else
    setVariant("Steam");
}

QDir GameSkyrimSE::documentsDirectory() const
{
  return m_MyGamesPath;
}

void GameSkyrimSE::detectGame()
{
  m_GamePath = identifyGamePath();
  checkVariants();
  m_MyGamesPath = determineMyGamesPath(gameDirectoryName());
}

QString GameSkyrimSE::identifyGamePath() const
{
  QMap<QString, QString> paths = {
      {"Software\\Bethesda Softworks\\" + gameName(), "Installed Path"},
      {"Software\\GOG.com\\Games\\1162721350", "path"},
      {"Software\\GOG.com\\Games\\1711230643", "path"},
  };

  QString result;
  for (auto& path : paths.toStdMap()) {
    result = findInRegistry(HKEY_LOCAL_MACHINE, path.first.toStdWString().c_str(), path.second.toStdWString().c_str());
    if (!result.isEmpty())
      break;
  }

  // Check Epic Games Manifests
  // AppName: ac82db5035584c7f8a2c548d98c86b2c
  // AE Update: 5d600e4f59974aeba0259c7734134e27
  if (result.isEmpty()) {
    result = parseEpicGamesLocation(
        {"ac82db5035584c7f8a2c548d98c86b2c", "5d600e4f59974aeba0259c7734134e27"});
  }

  return result;
}

void GameSkyrimSE::setGamePath(const QString& path)
{
  m_GamePath = path;
  checkVariants();
  m_MyGamesPath = determineMyGamesPath(gameDirectoryName());
  registerFeature<DataArchives>(new SkyrimSEDataArchives(myGamesPath()));
  registerFeature<LocalSavegames>(new GamebryoLocalSavegames(myGamesPath(), "SkyrimCustom.ini"));
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

bool GameSkyrimSE::init(IOrganizer* moInfo)
{
  if (!GameGamebryo::init(moInfo)) {
    return false;
  }

  registerFeature<ScriptExtender>(new SkyrimSEScriptExtender(this));
  registerFeature<DataArchives>(new SkyrimSEDataArchives(myGamesPath()));
  registerFeature<LocalSavegames>(new GamebryoLocalSavegames(myGamesPath(), "SkyrimCustom.ini"));
  registerFeature<ModDataChecker>(new SkyrimSEModDataChecker(this));
  registerFeature<ModDataContent>(new SkyrimSEModDataContent(this));
  registerFeature<SaveGameInfo>(new GamebryoSaveGameInfo(this));
  registerFeature<GamePlugins>(new CreationGamePlugins(moInfo));
  registerFeature<UnmanagedMods>(new SkyrimSEUnmangedMods(this));

  return true;
}

QString GameSkyrimSE::gameName() const
{
  return "Skyrim Special Edition";
}

QString GameSkyrimSE::gameDirectoryName() const
{
  if (selectedVariant() == "GOG")
    return "Skyrim Special Edition GOG";
  else if (selectedVariant() == "Epic Games")
    return "Skyrim Special Edition EPIC";
  else
    return "Skyrim Special Edition";
}

QList<ExecutableInfo> GameSkyrimSE::executables() const
{
  return QList<ExecutableInfo>()
    << ExecutableInfo("SKSE", findInGameFolder(feature<ScriptExtender>()->loaderName()))
    << ExecutableInfo("Skyrim Special Edition", findInGameFolder(binaryName()))
    << ExecutableInfo("Skyrim Special Edition Launcher", findInGameFolder(getLauncherName()))
    << ExecutableInfo("Creation Kit", findInGameFolder("CreationKit.exe")).withSteamAppId("1946180")
    << ExecutableInfo("LOOT", QFileInfo(getLootPath())).withArgument("--game=\"Skyrim Special Edition\"");
}

QList<ExecutableForcedLoadSetting> GameSkyrimSE::executableForcedLoads() const
{
  return QList<ExecutableForcedLoadSetting>();
}

QFileInfo GameSkyrimSE::findInGameFolder(const QString& relativePath) const
{
  return QFileInfo(m_GamePath + "/" + relativePath);
}

QString GameSkyrimSE::name() const
{
  return "Skyrim Special Edition Support Plugin";
}

QString GameSkyrimSE::localizedName() const
{
  return tr("Skyrim Special Edition Support Plugin");
}

QString GameSkyrimSE::author() const
{
  return "MO2 Team, Orig: Archost & ZachHaber";
}

QString GameSkyrimSE::description() const
{
  return tr("Adds support for the game Skyrim Special Edition.");
}

MOBase::VersionInfo GameSkyrimSE::version() const
{
  return VersionInfo(1, 7, 2, VersionInfo::RELEASE_FINAL);
}

QList<PluginSetting> GameSkyrimSE::settings() const
{
  return {PluginSetting("enderal_downloads", "Allow Enderal and Enderal SE downloads", QVariant(false))};
}

void GameSkyrimSE::initializeProfile(const QDir& path, ProfileSettings settings) const
{
  if (settings.testFlag(IPluginGame::MODS)) {
    copyToProfile(localAppFolder() + "/" + gameDirectoryName(), path, "plugins.txt");
  }

  if (settings.testFlag(IPluginGame::CONFIGURATION)) {
    if (settings.testFlag(IPluginGame::PREFER_DEFAULTS) || !QFileInfo(myGamesPath() + "/Skyrim.ini").exists()) {
      copyToProfile(gameDirectory().absolutePath(), path, "Skyrim_Default.ini", "Skyrim.ini");
    }
    else {
      copyToProfile(myGamesPath(), path, "Skyrim.ini");
    }

    copyToProfile(myGamesPath(), path, "SkyrimPrefs.ini");
    copyToProfile(myGamesPath(), path, "SkyrimCustom.ini");
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

std::shared_ptr<const GamebryoSaveGame>
GameSkyrimSE::makeSaveGame(QString filePath) const
{
  return std::make_shared<const SkyrimSESaveGame>(filePath, this);
}

QString GameSkyrimSE::steamAPPId() const
{
  if (selectedVariant() == "Steam")
    return "489830";
  return QString();
}

QStringList GameSkyrimSE::primaryPlugins() const
{
  QStringList plugins = {
    "skyrim.esm",
    "update.esm",
    "dawnguard.esm",
    "hearthfires.esm",
    "dragonborn.esm"
  };

  plugins.append(CCPlugins());

  return plugins;
}

QStringList GameSkyrimSE::gameVariants() const
{
  return {"Steam", "GOG", "Epic Games"};
}

QString GameSkyrimSE::gameShortName() const
{
  return "SkyrimSE";
}

QStringList GameSkyrimSE::validShortNames() const
{
  QStringList shortNames{"Skyrim"};
  if (m_Organizer->pluginSetting(name(), "enderal_downloads").toBool()) {
    shortNames.append({"Enderal", "EnderalSE"});
  }
  return shortNames;
}

QString GameSkyrimSE::gameNexusName() const
{
  return "skyrimspecialedition";
}

QStringList GameSkyrimSE::iniFiles() const
{
  return {"Skyrim.ini", "SkyrimPrefs.ini", "SkyrimCustom.ini"};
}

QStringList GameSkyrimSE::DLCPlugins() const
{
  return {
    "dawnguard.esm",
    "hearthfires.esm",
    "dragonborn.esm"
  };
}

QStringList GameSkyrimSE::CCPlugins() const
{
  QStringList plugins;
  std::set<QString> pluginsLookup;

  const QString path = gameDirectory().filePath("Skyrim.ccc");

  MOBase::forEachLineInFile(path, [&](QString s) {
    const auto lc = s.toLower();
    if (!pluginsLookup.contains(lc)) {
      pluginsLookup.insert(lc);
      plugins.append(std::move(s));
    }
  });

  return plugins;
}

IPluginGame::LoadOrderMechanism GameSkyrimSE::loadOrderMechanism() const
{
  return IPluginGame::LoadOrderMechanism::PluginsTxt;
}

int GameSkyrimSE::nexusModOrganizerID() const
{
  return 6194;
}

int GameSkyrimSE::nexusGameID() const
{
  return 1704;
}

QDir GameSkyrimSE::gameDirectory() const
{
  return QDir(m_GamePath);
}

// Not to delete all the spaces...
MappingType GameSkyrimSE::mappings() const
{
  MappingType result;

  for (const QString& profileFile : {"plugins.txt", "loadorder.txt"}) {
    result.push_back({m_Organizer->profilePath() + "/" + profileFile, localAppFolder() + "/" + gameDirectoryName() + "/" + profileFile, false});
  }

  return result;
}
