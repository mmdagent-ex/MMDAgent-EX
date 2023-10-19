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

// MouthQueue: mouth shape queue for A, I, U, O
class MouthQueue
{
private:
   MMDAgent *m_mmdagent;      // MMDAgent class
   int m_id;                  // Module ID for messaging/logging
   float *m_rateBuffer[4];
   unsigned int m_bufferSize;
   unsigned int m_currentWriteIndex;
   unsigned int m_currentReadIndex;
   double m_framesPerIndex;
   double m_currentRestFrame;

   // initialize: initialize instance
   void initialize();

   // clear: clear instance
   void clear();

public:

   // constructor
   MouthQueue();

   // destructor
   ~MouthQueue();

   // set up
   void setup(MMDAgent *mmdagent, int id, double maxDurationInMSec, double durationPerIndexInMSec);

   // getFramesPerIndex: get frames per index
   double getFramesPerIndex();

   // enqueue: enqueue a set of rate
   void enqueue(const float *rate);

   // dequeue: dequeue till the time ellapsed and return last set of rate
   bool dequeue(float *rate_ret, double ellapsedFrame);

   void clearqueue();

};
