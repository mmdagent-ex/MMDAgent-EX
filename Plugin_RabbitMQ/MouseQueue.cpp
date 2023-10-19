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
