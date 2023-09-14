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

#include "julius/juliuslib.h"

/* definitions */
#define JULIUSRECORD_TMPFILENAME "tmprecord.000"
#define JULIUSRECORD_FILENAMEFORMAT "%4d_%02d_%02d__%02d_%02d_%02d__%03d"

/* Julius_Record: record input for Julius */
class Julius_Record
{
private:
   MMDAgent *m_mmdagent;
   int m_id;

   bool m_enabled;
   char *m_dirName;                    // recording directory
   int m_rate;                         // sampling rate
   char m_finalFileName[MAXLINELEN];   // final file name to be renamed at end()
   char m_tmpFileName[MAXLINELEN];     // temporary file name to be used while recording
   unsigned int m_num;                 // length of stored samples
   int m_totalSize;                    // total size of recorded samples
   int m_maxSize;                      // maximum size of recorded samples
   int m_maxDurationMin;               // maximum duration of recorded samples in minutes

   FILE *m_fp;                         // file pointer for currently opened file

   /* initialize: initialize */
   void initialize();

   /* clear: free */
   void clear();

public :

   /* Julius_Record: constructor */
   Julius_Record(MMDAgent *mmdagent, int id, Recog *recog);

   /* ~Julius_Record: destructor  */
   ~Julius_Record();

   /* setDir: set recording directory */
   void setDir(const char *recordDir);

   /* setLimit: set recording size limitation in minutes */
   void setLimit(int maxDurationMin);

   /* enable: enable recording */
   void enable();

   /* disable: disable recording */
   void disable();

   /* start: callback function to open file for recording */
   void start();

   /* store: callback function to store samples to currently opened file */
   void store(SP16 *buf, unsigned int num);

   /* end: callback function to end of storing, finalize recorded file */
   void end();

};
