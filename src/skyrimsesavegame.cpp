#include "skyrimSEsavegame.h"

#include <Windows.h>

SkyrimSESaveGame::SkyrimSESaveGame(QString const &fileName, MOBase::IPluginGame const *game) :
  GamebryoSaveGame(fileName, game)
{
    FileWrapper file(this, "TESV_SAVEGAME"); //10bytes
    file.skip<unsigned long>(); // header size "TESV_SAVEGAME"
    file.skip<unsigned long>(); // header version 74. Original Skyrim is 79
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
	time.LowPart=ftime.dwLowDateTime;
	time.HighPart=ftime.dwHighDateTime;
	time.QuadPart-=2.16e11;
	ftime.dwHighDateTime=time.HighPart;
	ftime.dwLowDateTime=time.LowPart;
	
    SYSTEMTIME ctime;
    ::FileTimeToSystemTime(&ftime, &ctime);

    setCreationTime(ctime);
	//file.skip<unsigned char>();
	
	unsigned long width;
	unsigned long height;
	file.read(width);
	file.read(height);
	
	//Skip the 2 empty bytes before the image begins.
	//This is why we aren't using the readImage(scale,alpha)
	//variant.
	//file.skip<unsigned short>();
	file.read(compressionType);
	
	file.readImage(width,height,320,true);

	//Skip reading the plugins altogether, due to problems with the save files.
	//m_Plugins.push_back("Not Working due to save game weirdness");
	/* //Skip a single byte to get it to the right location
	file.skip<unsigned char>(); // form version
	//Need to skip 14 more bytes
	file.skip<unsigned short>(7);
    //file.skip<unsigned long>(); // plugin info size
	//Now in correct location to read plugins.
    file.readPlugins();// */
	file.readPlugins(5);
}
