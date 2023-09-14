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

#include "MMDAgent.h"

/* Timer::initialize: initialize timer */
void Timer::initialize()
{
   int i;

   m_systemStartTime = 0.0;
   m_systemLastUpdateFrame = 0.0;
   m_pauseTime = 0.0;

   m_fps = 0.0f;
   m_fpsStartTime = 0.0;
   m_fpsCount = 0;

   m_targetAdjustmentFrame = 0.0;
   m_currentAdjustmentFrame = 0.0;
   m_enableAdjustment = false;

   for (i = 0; i < MMDAGENT_TIMERNUM; i++)
      m_userStartTime[i] = 0.0;
}

/* Timer::clear: free timer */
void Timer::clear()
{
   initialize();
}

/* Timer::Timer: constructor */
Timer::Timer()
{
   initialize();
}

/* Timer::~Timer: destructor */
Timer::~Timer()
{
   clear();
}

/* Timer::setup: initialize and start timer */
void Timer::setup()
{
   /* get system start time */
   m_systemStartTime = MMDAgent_getTime();
   /* reset number of frames from last getTimeInterval function */
   m_systemLastUpdateFrame = 0.0;
   /* reset start time of fps count */
   m_fpsStartTime = m_systemStartTime;
   /* reset count of calling countFrame function */
   m_fpsCount = 0;
}

/* Timer::getFrameInterval: return time interval from last call */
double Timer::getFrameInterval()
{
   double currentTime;
   double currentSystemFrame;
   double intervalFrame;

   /* get current sec */
   currentTime = MMDAgent_getTime();
   /* calculate time from system start time */
   currentSystemFrame = MMDAgent_diffTime(currentTime, m_systemStartTime) * 30.0;
   /* get number of frames from last calling */
   if (m_systemLastUpdateFrame == 0.0)
      intervalFrame = 0.0;
   else
      intervalFrame = currentSystemFrame - m_systemLastUpdateFrame;
   /* save number of frames for next calling */
   m_systemLastUpdateFrame = currentSystemFrame;

   return intervalFrame;
}

/* Timer::pause: pause timer */
void Timer::pause()
{
   m_pauseTime = MMDAgent_getTime();
}

/* Timer::resume: resume timer */
void Timer::resume()
{
   m_systemLastUpdateFrame += MMDAgent_diffTime(MMDAgent_getTime(), m_pauseTime) * 30.0;
}

/* Timer::start: start user timer */
void Timer::start(int id)
{
   m_userStartTime[id] = MMDAgent_getTime();
}

/* Timer::ellapsed: return ellapsed time in sec since last call of start() */
double Timer::ellapsed(int id)
{
   return MMDAgent_diffTime(MMDAgent_getTime(), m_userStartTime[id]);
}

/* Timer::countFrame: increment frame count for FPS calculation */
void Timer::countFrame()
{
   double t;

   /* increment count */
   m_fpsCount++;

   /* update fps per second */
   t = MMDAgent_getTime();
   if (t - m_fpsStartTime >= 1.0) {
      /* calculate fps */
      m_fps = (float) m_fpsCount / (float)(t - m_fpsStartTime);
      /* reset counter */
      m_fpsStartTime = t;
      m_fpsCount = 0;
   }
}

/* Timer::getFps: get fps */
float Timer::getFps()
{
   return m_fps;
}

/* Timer::setTargetAdjustmentFrame: set target frame to sync music */
void Timer::setTargetAdjustmentFrame(double frame)
{
   m_targetAdjustmentFrame = frame;
}

/* Timer::startAdjustment: start to sync music */
void Timer::startAdjustment()
{
   m_currentAdjustmentFrame = 0.0;
   m_enableAdjustment = true;
}

/* Timer::stopAdjustment: stop to sync music */
void Timer::stopAdjustment()
{
   m_enableAdjustment = false;
}

/* Timer::getCurrentAdjustmentFrame: get current frame to sync music */
double Timer::getCurrentAdjustmentFrame()
{
   return m_currentAdjustmentFrame;
}

/* Timer::getAdditionalFrame: get number of additional frames to sync music */
double Timer::getAdditionalFrame(double frame)
{
   double step = 0.0;

   if (m_enableAdjustment == false)
      return 0.0;

   if (m_targetAdjustmentFrame > m_currentAdjustmentFrame) {
      /* x2 (max = 0.01 sec) */
      if (frame > 0.01 * 30.0) {
         step = 0.01 * 30.0;
      } else {
         step = frame;
      }
      if (m_currentAdjustmentFrame + step > m_targetAdjustmentFrame) {
         step = m_targetAdjustmentFrame - m_currentAdjustmentFrame;
         m_currentAdjustmentFrame = m_targetAdjustmentFrame;
      } else {
         m_currentAdjustmentFrame += step;
      }
   }
   if (m_targetAdjustmentFrame < m_currentAdjustmentFrame) {
      /* /2 (max = 0.005 sec) */
      if (frame > 0.01 * 30.0) {
         step = -0.005 * 30.0;
      } else {
         step = frame * -0.5;
      }
      if (m_currentAdjustmentFrame + step < m_targetAdjustmentFrame) {
         step = m_targetAdjustmentFrame - m_currentAdjustmentFrame;
         m_currentAdjustmentFrame = m_targetAdjustmentFrame;
      } else {
         m_currentAdjustmentFrame += step;
      }
   }

   return step;
}

/* Timer::getSystemUpFrame: get system up time in frames */
double Timer::getSystemUpFrame()
{
   return m_systemLastUpdateFrame;
}
