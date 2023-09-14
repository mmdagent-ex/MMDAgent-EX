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

/* TextureLink: list of textures */
typedef struct _TextureLink {
   char *filePath;            /* source file path */
   char *baseName;            /* source file base name (= name in model) */
   PMDTexture *texture;       /* texture data and information */
   struct _TextureLink *next;
} TextureLink;

/* PMDTextureLoader: texture loader of PMD */
class PMDTextureLoader
{
private:

   TextureLink *m_root; /* linked list of textures currently loaded */
   bool m_hasError;     /* true then some error occured at texture loading */

   /* lookup: lookup texture in cache */
   PMDTexture *lookup(const char *filePath, bool *alreadyFailRet);

   /* store: add a texture to cache */
   void store(PMDTexture *tex, const char *filePath, const char *fileBaseName);

   /* initialize: initialize texture loader */
   void initialize();

   /* clear: free texture loader */
   void clear();

public:

   /* PMDTextureLoader: constructor */
   PMDTextureLoader();

   /* ~PMDTextureLoader: destructor */
   ~PMDTextureLoader();

   /* load: load texture from file name (multi-byte char) */
   PMDTexture *load(const char *filePath, const char *fileBaseName, bool sphereFlag = false, bool sphereAddFlag = false);

   /* getErrorTextureString: get newline-separated list of error textures */
   void getErrorTextureString(char *buf, int size);

   /* release: free texture loader */
   void release();

   /* update: update */
   void update(double ellapsedFrame);

   /* setAnimationSpeedRate: set animation speed rate of a texture */
   bool setAnimationSpeedRate(const char *fileName, double rate);
};
