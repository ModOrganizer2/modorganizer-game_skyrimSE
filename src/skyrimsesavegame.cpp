#include "skyrimSEsavegame.h"

#include <Windows.h>

SkyrimSESaveGame::SkyrimSESaveGame(QString const& fileName, GameSkyrimSE const* game)
    : GamebryoSaveGame(fileName, game, true)
{
  FileWrapper file(fileName, "TESV_SAVEGAME");  // 10bytes

  unsigned long version;
  FILETIME ftime;
  fetchInformationFields(file, version, m_PCName, m_PCLevel, m_PCLocation, m_SaveNumber,
                         ftime);

  // A file time is a 64-bit value that represents the number of 100-nanosecond
  // intervals that have elapsed since 12:00 A.M. January 1, 1601 Coordinated Universal
  // Time (UTC). So we need to convert that to something useful

  // For some reason, the file time is off by about 6 hours.
  // So we need to subtract those 6 hours from the filetime.
  _ULARGE_INTEGER time;
  time.LowPart  = ftime.dwLowDateTime;
  time.HighPart = ftime.dwHighDateTime;
  time.QuadPart -= 2.16e11;
  ftime.dwHighDateTime = time.HighPart;
  ftime.dwLowDateTime  = time.LowPart;

  SYSTEMTIME ctime;
  ::FileTimeToSystemTime(&ftime, &ctime);

  setCreationTime(ctime);
}

void SkyrimSESaveGame::fetchInformationFields(FileWrapper& file, unsigned long& version,
                                              QString& playerName,
                                              unsigned short& playerLevel,
                                              QString& playerLocation,
                                              unsigned long& saveNumber,
                                              FILETIME& creationTime) const
{
  unsigned long headerSize;
  file.read(headerSize);  // header size "TESV_SAVEGAME"
  file.read(version);
  file.read(saveNumber);
  file.read(playerName);

  unsigned long temp;
  file.read(temp);
  playerLevel = static_cast<unsigned short>(temp);
  file.read(playerLocation);

  QString timeOfDay;
  file.read(timeOfDay);

  QString race;
  file.read(race);  // race name (i.e. BretonRace)

  file.skip<unsigned short>();  // Player gender (0 = male)
  file.skip<float>(2);          // experience gathered, experience required

  file.read(creationTime);  // filetime
}

std::unique_ptr<GamebryoSaveGame::DataFields> SkyrimSESaveGame::fetchDataFields() const
{
  FileWrapper file(getFilepath(), "TESV_SAVEGAME");  // 10bytes

  unsigned long version = 0;
  {
    QString dummyName, dummyLocation;
    unsigned short dummyLevel;
    unsigned long dummySaveNumber;
    FILETIME dummyTime;

    fetchInformationFields(file, version, dummyName, dummyLevel, dummyLocation,
                           dummySaveNumber, dummyTime);
  }

  std::unique_ptr<DataFields> fields = std::make_unique<DataFields>();

  unsigned long width;
  unsigned long height;
  file.read(width);
  file.read(height);

  bool alpha = false;

  // compatibility between LE and SE:
  //  SE has an additional uin16_t for compression
  //  SE uses an alpha channel, whereas LE does not
  uint16_t compressionType = 0;
  if (version == 12) {
    file.read(compressionType);
    alpha = true;
  }

  fields->Screenshot = file.readImage(width, height, 320, alpha);

  file.setCompressionType(compressionType);
  file.openCompressedData();

  uint8_t saveGameVersion = file.readChar();
  uint8_t pluginInfoSize  = file.readChar();
  uint16_t other          = file.readShort();  // Unknown

  fields->Plugins = file.readPlugins(1);  // Just empty data

  if (saveGameVersion >= 78) {
    fields->LightPlugins = file.readLightPlugins();
  }

  file.closeCompressedData();

  return fields;
}
