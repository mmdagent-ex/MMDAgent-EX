/*
  Copyright 2022-2023  Nagoya Institute of Technology

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/
/* ----------------------------------------------------------------- */
/*           The Toolkit for Building Voice Interaction Systems      */
/*           "MMDAgent" developed by MMDAgent Project Team           */
/*           http://www.mmdagent.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2016  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAgent project team nor the names of  */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

/* headers */

#include "MMDFiles.h"

/* SystemTexture::initialize: initialize SystemTexture */
void SystemTexture::initialize()
{
   int i;

   for (i = 0; i < SYSTEMTEXTURE_NUMFILES; i++)
      m_toonTextureID[i] = 0;
}

/* SystemTexture::clear: free SystemTexutre */
void SystemTexture::clear()
{
   int i;

   for (i = 0; i < SYSTEMTEXTURE_NUMFILES; i++)
      m_toonTexture[i].release();
   initialize();
}

/* SystemTexture::SystemTexutre: constructor */
SystemTexture::SystemTexture()
{
   initialize();
}

/* SystemTexture::SystemTexutre:: destructor */
SystemTexture::~SystemTexture()
{
   clear();
}

/* SystemTexture::load: load system texture from current directory */
bool SystemTexture::load(const char *dir)
{
   int i;
   bool ret = true;
   const char *files[] = {SYSTEMTEXTURE_FILENAMES};
   char buff[MMDFILES_MAXBUFLEN];

   for (i = 0; i < SYSTEMTEXTURE_NUMFILES; i++) {
      if (MMDFiles_strlen(dir) > 0)
         sprintf(buff, "%s%c%s", dir, MMDFILES_DIRSEPARATOR, files[i]);
      else
         strcpy(buff, files[i]);
      if (m_toonTexture[i].load(buff) == false)
         ret = false;
      m_toonTextureID[i] = m_toonTexture[i].getID();
      /* set GL_CLAMP_TO_EDGE for toon texture to avoid texture interpolation at edge */
      glBindTexture(GL_TEXTURE_2D, m_toonTextureID[i]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glBindTexture(GL_TEXTURE_2D, 0);
   }

   return ret;
}

/* SystemTexture::getTextureID: get toon texture ID */
unsigned int SystemTexture::getTextureID(int i)
{
   return m_toonTextureID[i];
}

/* SystemTexture::release: free SystemTexture */
void SystemTexture::release()
{
   clear();
}
