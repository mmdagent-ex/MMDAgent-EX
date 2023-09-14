/* ----------------------------------------------------------------- */
/*           The Toolkit for Building Voice Interaction Systems      */
/*           "MMDAgent" developed by MMDAgent Project Team           */
/*           http://www.mmdagent.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2015  Nagoya Institute of Technology          */
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

#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif /* _WIN32 */

/* headers */
#include "MMDAgent.h"

/* Plugin name, commands and events */
#define PLUGIN_NAME "ErrorBlock"
#define MAXBLOCKNUM 10
#define BLOCKNAMEFMT "_ERRORBLOCK%d"
#define BLOCKMODELFILE "errorblock.pmd"
#define BLOCKMODELHEIGHTMARGIN 2.0
#define BLOCKPOSITION_X 0.0f
#define BLOCKPOSITION_Y 20.0f
#define BLOCKPOSITION_Z 0.0f

/* plugin status */
static int mid;
static int blocknum;

/* extAppStart: initialize */
EXPORT void extAppStart(MMDAgent *mmdagent)
{
   mid = mmdagent->getModuleId(PLUGIN_NAME);
   blocknum = 0;
}

/* extLog: process log string */
EXPORT void extLog(MMDAgent *mmdagent, int id, unsigned int flag, const char *text, const char *fulltext)
{
   char buf[MMDAGENT_MAXBUFLEN];
   float y;
   btVector3 pos;

   if (blocknum >= MAXBLOCKNUM)
      return;
   if (id != mid && flag == MLOG_ERROR) {
      y = BLOCKPOSITION_Y + blocknum * 0.1f;
      MMDAgent_snprintf(buf, MMDAGENT_MAXBUFLEN - 1, BLOCKNAMEFMT, blocknum);
      mmdagent->sendMessage(mid, MMDAGENT_COMMAND_MODELADD, "%s|%s%c%s|%.2f,%.2f,%.2f", buf, mmdagent->getAppDirName(), MMDAGENT_DIRSEPARATOR, BLOCKMODELFILE, BLOCKPOSITION_X, y, BLOCKPOSITION_Z);
      mmdagent->sendMessage(mid, "TEXTAREA_ADD", "%s|15,0|1,0.4,0|0,0,0,0|1,0,0,1|0,0,-0.76|180,0,0|%s", buf, buf);
      mmdagent->sendMessage(mid, "TEXTAREA_SET", "%s|%s", buf, text);
      blocknum++;
   }
}

/* extAppEnd: end of application */
EXPORT void extAppEnd(MMDAgent *mmdagent)
{
   char buf[MMDAGENT_MAXBUFLEN];
   short i;
   int j;
   PMDObject *models;

   models = mmdagent->getModelList();
   if (models == NULL)
      return;
   for (j = 0; j < blocknum; j++) {
      MMDAgent_snprintf(buf, MMDAGENT_MAXBUFLEN - 1, BLOCKNAMEFMT, j);
      for (i = 0; i < mmdagent->getNumModel(); i++) {
         if (models[i].isEnable() && MMDAgent_strequal(models[i].getAlias(), buf)) {
            mmdagent->sendMessage(mid, MMDAGENT_COMMAND_MODELDELETE, "%s", buf);
         }
      }
   }
}
