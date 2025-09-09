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

/* PMDTextureLoader_strcat: strcat using buffer size */
static void PMDTextureLoader_strcat(char *buf, int size, const char *str)
{
   size_t i, j, len1, len2;
   char c;

   len1 = MMDFiles_strlen(buf);
   len2 = MMDFiles_strlen(str);

   if(len1 <= 0 || size <= 0 || len2 <= 0)
      return;

   for(i = 0, j = len1; i < len2; i++, j++) {
      c = str[i];
      if(j >= size - 1) break;
      buf[j] = c;
   }
   buf[j] = '\0';
}

/* PMDTextureLoader:lookup: lookup texture in cache */
PMDTexture *PMDTextureLoader::lookup(const char *filePath, bool *alreadyFailRet)
{
   TextureLink *tmp = m_root;

   while (tmp) {
      if (MMDFiles_strequal(tmp->filePath, filePath) == true) {
         /* if exist but texture is NULL, it has been failed */
         *alreadyFailRet = (tmp->texture == NULL) ? true : false;
         return tmp->texture;
      }
      tmp = tmp->next;
   }
   *alreadyFailRet = false;

   return NULL;
}

/* PMDTextureLoader::store: add a texture to cache */
void PMDTextureLoader::store(PMDTexture *tex, const char *filePath, const char *fileBaseName)
{
   TextureLink *newLink = new TextureLink;

   newLink->filePath = MMDFiles_strdup(filePath);
   newLink->baseName = MMDFiles_strdup(fileBaseName);
   newLink->texture = tex;
   newLink->next = m_root;
   m_root = newLink;
}

/* PMDTextureLoader::initialize: initialize texture loader  */
void PMDTextureLoader::initialize()
{
   m_root = NULL;
   m_hasError = false;
   m_textureLoadMemory = NULL;
}

/* PMDTextureLoader::clear: free texture loader  */
void PMDTextureLoader::clear()
{
   TextureLink *tmp = m_root;
   TextureLink *next;

   freeTextureWorkArea();

   while (tmp) {
      next = tmp->next;
      if (tmp->filePath)
         free(tmp->filePath);
      if (tmp->baseName)
         free(tmp->baseName);
      if(tmp->texture != NULL)
         delete tmp->texture;
      delete tmp;
      tmp = next;
   }
   initialize();
}

/* PMDTextureLoader::PMDTextureLoader: constructor */
PMDTextureLoader::PMDTextureLoader()
{
   initialize();
}

/* PMDTextureLoader::~PMDTextureLoader: destructor */
PMDTextureLoader::~PMDTextureLoader()
{
   clear();
}

/* PMDTextureLoader::load: load texture from file name (multi-byte char) */
PMDTexture *PMDTextureLoader::load(const char *filePath, const char *fileBaseName, bool sphereFlag, bool sphereAddFlag)
{
   PMDTexture *tex;
   bool already_fail;

   /* consult cache */
   tex = lookup(filePath, &already_fail);
   /* when exist but has failed, return error without trying to load */
   if (already_fail) return NULL;
   if (tex == NULL) {
      /* not exist, try to load */
      tex = new PMDTexture;
      if (tex->load(filePath, sphereFlag, sphereAddFlag, m_textureLoadMemory) == false) {
         /* failed, store with failed status */
         store(NULL, filePath, fileBaseName);
         m_hasError = true;
         delete tex;
         return NULL;
      }
      /* succeeded, store it */
      store(tex, filePath, fileBaseName);
   }
   return tex;
}

/* PMDTextureLoader::getErrorTextureString: get newline-separated list of error textures */
void PMDTextureLoader::getErrorTextureString(char *buf, int size)
{
   TextureLink *tmp = m_root;

   strcpy(buf, "");
   if (!m_hasError) return;
   for (tmp = m_root; tmp; tmp = tmp->next) {
      if (tmp->texture == NULL) {
         PMDTextureLoader_strcat(buf, size, tmp->baseName);
         PMDTextureLoader_strcat(buf, size, "\n");
      }
   }
}

/* PMDTextureLoader::release: free texture loader */
void PMDTextureLoader::release()
{
   clear();
}

/* PMDTextureLoader::update: update */
void PMDTextureLoader::update(double ellapsedFrame)
{
   TextureLink *tmp = m_root;

   while (tmp) {
      if (tmp->texture)
         tmp->texture->setNextFrame(ellapsedFrame);
      tmp = tmp->next;
   }
}

/* PMDTextureLoader::setAnimationSpeedRate: set animation speed rate of a texture */
bool PMDTextureLoader::setAnimationSpeedRate(const char *fileName, double rate)
{
   TextureLink *tmp = m_root;
   bool ret = false;

   while (tmp) {
      if (tmp->texture && MMDFiles_strequal(tmp->baseName, fileName)) {
         tmp->texture->setAnimationSpeedRate(rate);
         ret = true;
      }
      tmp = tmp->next;
   }

   return(ret);
}

/* PMDTextureLoader::allocateTextureWorkArea: allocate texture work area */
void PMDTextureLoader::allocateTextureWorkArea(unsigned int size)
{
   freeTextureWorkArea();
   m_textureLoadMemory = new TextureLoadMemory();
   m_textureLoadMemory->allocMemory(size);
}

/* PMDTextureLoader::freeTextureWorkArea: free texture work area */
void PMDTextureLoader::freeTextureWorkArea()
{
   if (m_textureLoadMemory) {
      delete m_textureLoadMemory;
      m_textureLoadMemory = NULL;
   }
}
