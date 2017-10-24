#include "gameskyrimse.h"

#include "skyrimsedataarchives.h"
#include "skyrimsescriptextender.h"
#include "skyrimsesavegameinfo.h"
#include "skyrimsegameplugins.h"
#include "skyrimseunmanagedmods.h"

#include <pluginsetting.h>
#include "iplugingame.h"
#include <executableinfo.h>
#include <gamebryolocalsavegames.h>
#include <gamebryogameplugins.h>
#include "versioninfo.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

#include <memory>


#include "utility.h"
#include <windows.h>
#include <winreg.h>
#include "scopeguard.h"
namespace {

    std::unique_ptr<BYTE[]> getRegValue(HKEY key, LPCWSTR path, LPCWSTR value,
        DWORD flags, LPDWORD type = nullptr)
    {
        DWORD size = 0;
        HKEY subKey;
        LONG res = ::RegOpenKeyExW(key, path, 0,
            KEY_QUERY_VALUE | KEY_WOW64_32KEY, &subKey);
        if (res != ERROR_SUCCESS) {
            return std::unique_ptr<BYTE[]>();
        }
        res = ::RegGetValueW(subKey, L"", value, flags, type, nullptr, &size);
        if (res == ERROR_FILE_NOT_FOUND || res == ERROR_UNSUPPORTED_TYPE) {
            return std::unique_ptr<BYTE[]>();
        }
        if (res != ERROR_SUCCESS && res != ERROR_MORE_DATA) {
            throw MOBase::MyException(QObject::tr("failed to query registry path (preflight): %1").arg(res, 0, 16));
        }

        std::unique_ptr<BYTE[]> result(new BYTE[size]);
        res = ::RegGetValueW(subKey, L"", value, flags, type, result.get(), &size);

        if (res != ERROR_SUCCESS) {
            throw MOBase::MyException(QObject::tr("failed to query registry path (read): %1").arg(res, 0, 16));
        }

        return result;
    }

    QString findInRegistry(HKEY baseKey, LPCWSTR path, LPCWSTR value)
    {
        std::unique_ptr<BYTE[]> buffer = getRegValue(baseKey, path, value, RRF_RT_REG_SZ | RRF_NOEXPAND);

        return QString::fromUtf16(reinterpret_cast<const ushort*>(buffer.get()));
    }

    QString getKnownFolderPath(REFKNOWNFOLDERID folderId, bool useDefault)
    {
        PWSTR path = nullptr;
        ON_BLOCK_EXIT([&]() {
            if (path != nullptr) ::CoTaskMemFree(path);
        });

        if (::SHGetKnownFolderPath(folderId, useDefault ? KF_FLAG_DEFAULT_PATH : 0, NULL, &path) == S_OK) {
            return QDir::fromNativeSeparators(QString::fromWCharArray(path));
        }
        else {
            return QString();
        }
    }


    QString getSpecialPath(const QString &name)
    {
        QString base = findInRegistry(HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders",
            name.toStdWString().c_str());

        WCHAR temp[MAX_PATH];
        if (::ExpandEnvironmentStringsW(base.toStdWString().c_str(), temp, MAX_PATH) != 0) {
            return QString::fromWCharArray(temp);
        }
        else {
            return base;
        }
    } 

    QString determineMyGamesPath(const QString &gameName)
    {
        // a) this is the way it should work. get the configured My Documents directory
        QString result = getKnownFolderPath(FOLDERID_Documents, false);

        // b) if there is no <game> directory there, look in the default directory
        if (result.isEmpty()
            || !QFileInfo(result + "/My Games/" + gameName).exists()) {
            result = getKnownFolderPath(FOLDERID_Documents, true);
        }
        // c) finally, look in the registry. This is discouraged
        if (result.isEmpty()
            || !QFileInfo(result + "/My Games/" + gameName).exists()) {
            result = getSpecialPath("Personal");
        }

        return result + "/My Games/" + gameName;
    }


}


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

    m_Organizer = moInfo;
    m_GamePath = GameSkyrimSE::identifyGamePath();
    m_MyGamesPath = determineMyGamesPath(gameName());


    registerFeature<ScriptExtender>(new SkyrimSEScriptExtender(this));
    registerFeature<DataArchives>(new SkyrimSEDataArchives());
    registerFeature<LocalSavegames>(new GamebryoLocalSavegames(myGamesPath(), "Skyrim.ini"));
    registerFeature<SaveGameInfo>(new SkyrimSESaveGameInfo(this));
    registerFeature<GamePlugins>(new SkyrimSEGamePlugins(moInfo));
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
        << ExecutableInfo("LOOT", getLootPath())
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

QString GameSkyrimSE::steamAPPId() const
{
    return "489830";
}

QStringList GameSkyrimSE::primaryPlugins() const {
    return{ "skyrim.esm", "update.esm", "dawnguard.esm", "hearthfires.esm", "dragonborn.esm",
            "ccbgssse002-exoticarrows.esl", "ccbgssse003-zombies.esl", "ccbgssse004-ruinsedge.esl",
            "ccbgssse006-stendarshammer.esl", "ccbgssse007-chrysamere.esl", "ccbgssse010-petdwarvenarmoredmudcrab.esl",
            "ccbgssse014-spellpack01.esl", "ccbgssse019-staffofsheogorath.esl", "ccmtysse001-knightsofthenine.esl",
            "ccqdrsse001-survivalmode.esl" };//  };
}

QStringList GameSkyrimSE::gameVariants() const
{
    return{ "Regular" };
}

QString GameSkyrimSE::gameShortName() const
{
    return "skyrimse";
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
    return{ "dawnguard.esm", "hearthfires.esm", "dragonborn.esm",
            "ccbgssse002-exoticarrows.esl", "ccbgssse003-zombies.esl", "ccbgssse004-ruinsedge.esl",
            "ccbgssse006-stendarshammer.esl", "ccbgssse007-chrysamere.esl", "ccbgssse010-petdwarvenarmoredmudcrab.esl",
            "ccbgssse014-spellpack01.esl", "ccbgssse019-staffofsheogorath.esl", "ccmtysse001-knightsofthenine.esl",
            "ccqdrsse001-survivalmode.esl" };
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

