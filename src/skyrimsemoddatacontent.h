#ifndef SKYRIMSE_MODDATACONTENT_H
#define SKYRIMSE_MODDATACONTENT_H

#include <gamebryomoddatacontent.h>
#include <ifiletree.h>

class SkyrimSEModDataContent : public GamebryoModDataContent
{
public:
  /**
   *
   */
  SkyrimSEModDataContent(GameGamebryo const* gamePlugin)
      : GamebryoModDataContent(gamePlugin)
  {
    // Just need to disable some contents:
    m_Enabled[CONTENT_SKYPROC] = false;
  }
};

#endif  // SKYRIMSE_MODDATACONTENT_H
