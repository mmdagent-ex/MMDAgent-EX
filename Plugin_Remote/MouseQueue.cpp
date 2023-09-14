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

/* headers */

#include "MMDAgent.h"
#include "MouseQueue.h"

// MouthQueue::initialize: initialize instance
void MouthQueue::initialize()
{
   m_rateBuffer[0] = m_rateBuffer[1] = m_rateBuffer[2] = m_rateBuffer[3] = NULL;
   m_bufferSize = 0;
   m_currentReadIndex = 0;
   m_currentWriteIndex = 0;
   m_framesPerIndex = 0.0;
   m_currentRestFrame = 0.0;
}

// MouthQueue::clear: clear instance
void MouthQueue::clear()
{
   for (int i = 0; i < 4; i++) {
      if (m_rateBuffer[i])
         free(m_rateBuffer[i]);
   }
   initialize();
}

// constructor
MouthQueue::MouthQueue()
{
   initialize();
}

// MouthQueue::setup: setup
void MouthQueue::setup(MMDAgent *mmdagent, int id, double maxDurationInMSec, double durationPerIndexInMSec)
{
   initialize();

   m_mmdagent = mmdagent;
   m_id = id;

   m_bufferSize = (unsigned int)(maxDurationInMSec / durationPerIndexInMSec);

   for (int i = 0; i < 4; i++) {
      m_rateBuffer[i] = (float *)malloc(sizeof(float) * m_bufferSize);
   }

   m_framesPerIndex = durationPerIndexInMSec * 30.0 / 1000.0;
}

// MouthQueue::getFramesPerIndex: get frames per index
double MouthQueue::getFramesPerIndex()
{
   return m_framesPerIndex;
}


// destructor
MouthQueue::~MouthQueue()
{
   clear();
}

// MouthQueue::enqueue: enqueue a set of rate
void MouthQueue::enqueue(const float *rate)
{
   for (int i = 0; i < 4; i++)
      m_rateBuffer[i][m_currentWriteIndex] = rate[i];
   m_currentWriteIndex++;
   if (m_currentWriteIndex >= m_bufferSize)
      m_currentWriteIndex = 0;
}

// MouthQueue::dequeue: dequeue till the time ellapsed and return last set of rate
bool MouthQueue::dequeue(float *rate_ret, double ellapsedFrame)
{
   double frame;
   int step;
   unsigned int index;

   frame = m_currentRestFrame + ellapsedFrame;
   step = (int)(frame / m_framesPerIndex);
   m_currentRestFrame = frame - m_framesPerIndex * (double)step;
   if (step == 0) {
      // not reached a next step
      return false;
   }
   index = m_currentReadIndex;
   for (int i = 0; i < step; i++) {
      if (m_currentReadIndex == m_currentWriteIndex) {
         if (i == 0) {
            // no data
            return false;
         }
         break;
      }
      index = m_currentReadIndex;
      m_currentReadIndex++;
      if (m_currentReadIndex >= m_bufferSize)
         m_currentReadIndex = 0;
   }

   for (int i = 0; i < 4; i++)
      rate_ret[i] = m_rateBuffer[i][index];

   return true;
}

void MouthQueue::clearqueue()
{
   m_currentReadIndex = 0;
   m_currentWriteIndex = 0;
}
