#include "skyrimSEsavegame.h"

#include <Windows.h>

SkyrimSESaveGame::SkyrimSESaveGame(QString const &fileName, MOBase::IPluginGame const *game, bool const lightEnabled) :
  GamebryoSaveGame(fileName, game, lightEnabled)
{
  FileWrapper file(this, "TESV_SAVEGAME"); //10bytes
  unsigned long headerSize;
  file.read(headerSize); // header size "TESV_SAVEGAME"
  unsigned long version = 0;
  file.read(version);
  file.read(m_SaveNumber);

  file.read(m_PCName);

  unsigned long temp;
  file.read(temp);
  m_PCLevel = static_cast<unsigned short>(temp);

  file.read(m_PCLocation);

  QString timeOfDay;
  file.read(timeOfDay);

  QString race;
  file.read(race); // race name (i.e. BretonRace)

  file.skip<unsigned short>(); // Player gender (0 = male)
  file.skip<float>(2); // experience gathered, experience required

  FILETIME ftime;
  file.read(ftime); //filetime
                    //A file time is a 64-bit value that represents the number of 100-nanosecond
                    //intervals that have elapsed since 12:00 A.M. January 1, 1601 Coordinated Universal Time (UTC).
                    //So we need to convert that to something useful

                    //For some reason, the file time is off by about 6 hours.
                    //So we need to subtract those 6 hours from the filetime.
  _ULARGE_INTEGER time;
  time.LowPart = ftime.dwLowDateTime;
  time.HighPart = ftime.dwHighDateTime;
  time.QuadPart -= 2.16e11;
  ftime.dwHighDateTime = time.HighPart;
  ftime.dwLowDateTime = time.LowPart;

  SYSTEMTIME ctime;
  ::FileTimeToSystemTime(&ftime, &ctime);

  setCreationTime(ctime);

  unsigned long width;
  unsigned long height;
  file.read(width);
  file.read(height);

  bool alpha = false;

  // compatibility between LE and SE:
  //  SE has an additional uin16_t for compression
  //  SE uses an alpha channel, whereas LE does not
  if (static_cast<GameVersions>(version) == GameVersions::SpecialEdition) {
    file.read(m_CompressionType);
    alpha = true;
  }

  file.readImage(width, height, 320, alpha);

  file.openCompressedData();

  uint8_t saveGameVersion = file.readChar();
  uint8_t pluginInfoSize = file.readChar();
  uint16_t other = file.readShort(); //Unknown

  file.readPlugins(1); // Just empty data

  if (saveGameVersion >= 78) {
    file.readLightPlugins();
  }

  file.closeCompressedData();
}
