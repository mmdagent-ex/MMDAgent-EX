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

/* CameraController::control: set camera parameters according to the motion at the specified frame */
void CameraController::control(float frameNow)
{
   float frame = frameNow;
   unsigned long k1, k2 = 0;
   unsigned long i;
   float time1;
   float time2;
   float distance1, distance2;
   btVector3 pos1, pos2;
   btVector3 angle1, angle2;
   float fovy1, fovy2;

   CameraKeyFrame *keyFrameForInterpolation;
   float x, y, z, ww;
   float w;
   short idx;
   bool do_lerp;

   /* clamp frame to the defined last frame */
   if (frame > m_motion->keyFrameList[m_motion->numKeyFrame - 1].keyFrame)
      frame = m_motion->keyFrameList[m_motion->numKeyFrame - 1].keyFrame;

   /* find key frames between which the given frame exists */
   if (frame >= m_motion->keyFrameList[m_lastKey].keyFrame) {
      /* start searching from last used key frame */
      for (i = m_lastKey; i < m_motion->numKeyFrame; i++) {
         if (frame <= m_motion->keyFrameList[i].keyFrame) {
            k2 = i;
            break;
         }
      }
   } else {
      for (i = 0; i <= m_lastKey && i < m_motion->numKeyFrame; i++) {
         if (frame <= m_motion->keyFrameList[i].keyFrame) {
            k2 = i;
            break;
         }
      }
   }

   /* bounding */
   if (k2 >= m_motion->numKeyFrame)
      k2 = m_motion->numKeyFrame - 1;
   if (k2 <= 1)
      k1 = 0;
   else
      k1 = k2 - 1;

   /* store the last key frame for next call */
   m_lastKey = k1;

   /* calculate the camera pameters at the specified frame */
   time1 = m_motion->keyFrameList[k1].keyFrame;
   time2 = m_motion->keyFrameList[k2].keyFrame;
   keyFrameForInterpolation = &(m_motion->keyFrameList[k2]);

   distance1 = m_motion->keyFrameList[k1].distance;
   pos1      = m_motion->keyFrameList[k1].pos;
   angle1    = m_motion->keyFrameList[k1].angle;
   fovy1     = m_motion->keyFrameList[k1].fovy;
   distance2 = m_motion->keyFrameList[k2].distance;
   pos2      = m_motion->keyFrameList[k2].pos;
   angle2    = m_motion->keyFrameList[k2].angle;
   fovy2     = m_motion->keyFrameList[k2].fovy;

   /* calculate the position and rotation */
   if (time1 != time2) {
      do_lerp = false;
      if (frame <= time1) {
         m_distance = distance1;
         m_pos      = pos1;
         m_angle    = angle1;
         m_fovy     = fovy1;
      } else if (frame >= time2) {
         m_distance = distance2;
         m_pos      = pos2;
         m_angle    = angle2;
         m_fovy     = fovy2;
      } else if (time2 - time1 <= 1.0f) {
         /* successive keyframe in camera motion */
         /* if position or angle is far, do not perform interpolate between the frame */
         btQuaternion rot1 = btQuaternion(btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(1.0f)), btScalar(MMDFILES_RAD(angle1.z())))
            * btQuaternion(btVector3(btScalar(1.0f), btScalar(0.0f), btScalar(0.0f)), btScalar(MMDFILES_RAD(angle1.x())))
            * btQuaternion(btVector3(btScalar(0.0f), btScalar(1.0f), btScalar(0.0f)), btScalar(MMDFILES_RAD(angle1.y())));
         btQuaternion rot2 = btQuaternion(btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(1.0f)), btScalar(MMDFILES_RAD(angle1.z())))
            * btQuaternion(btVector3(btScalar(1.0f), btScalar(0.0f), btScalar(0.0f)), btScalar(MMDFILES_RAD(angle2.x())))
            * btQuaternion(btVector3(btScalar(0.0f), btScalar(1.0f), btScalar(0.0f)), btScalar(MMDFILES_RAD(angle2.y())));
         btQuaternion q_diff = rot2 * rot1.inverse();
         btScalar angle_diff = q_diff.getAngle();
         if (pos1.distance2(pos2) > 3.0 || angle_diff > 0.08 || fabs(distance1 - distance2) > 3.0 || fabs(fovy1 - fovy2) > 2.0) {
            m_distance = distance1;
            m_pos = pos1;
            m_angle = angle1;
            m_fovy = fovy1;
         } else {
            do_lerp = true;
         }
      } else {
         do_lerp = true;
      }
      if (do_lerp) {
         /* lerp */
         w = (frame - time1) / (time2 - time1);
         idx = (short)(w * VMD_INTERPOLATIONTABLESIZE);
         if (keyFrameForInterpolation->linear[0]) {
            x = pos1.x() * (1.0f - w) + pos2.x() * w;
         } else {
            ww = keyFrameForInterpolation->interpolationTable[0][idx] + (keyFrameForInterpolation->interpolationTable[0][idx + 1] - keyFrameForInterpolation->interpolationTable[0][idx]) * (w * VMD_INTERPOLATIONTABLESIZE - idx);
            x = pos1.x() * (1.0f - ww) + pos2.x() * ww;
         }
         if (keyFrameForInterpolation->linear[1]) {
            y = pos1.y() * (1.0f - w) + pos2.y() * w;
         } else {
            ww = keyFrameForInterpolation->interpolationTable[1][idx] + (keyFrameForInterpolation->interpolationTable[1][idx + 1] - keyFrameForInterpolation->interpolationTable[1][idx]) * (w * VMD_INTERPOLATIONTABLESIZE - idx);
            y = pos1.y() * (1.0f - ww) + pos2.y() * ww;
         }
         if (keyFrameForInterpolation->linear[2]) {
            z = pos1.z() * (1.0f - w) + pos2.z() * w;
         } else {
            ww = keyFrameForInterpolation->interpolationTable[2][idx] + (keyFrameForInterpolation->interpolationTable[2][idx + 1] - keyFrameForInterpolation->interpolationTable[2][idx]) * (w * VMD_INTERPOLATIONTABLESIZE - idx);
            z = pos1.z() * (1.0f - ww) + pos2.z() * ww;
         }
         m_pos.setValue(btScalar(x), btScalar(y), btScalar(z));
         if (keyFrameForInterpolation->linear[3]) {
            m_angle = angle1.lerp(angle2, btScalar(w));
         } else {
            ww = keyFrameForInterpolation->interpolationTable[3][idx] + (keyFrameForInterpolation->interpolationTable[3][idx + 1] - keyFrameForInterpolation->interpolationTable[3][idx]) * (w * VMD_INTERPOLATIONTABLESIZE - idx);
            m_angle = angle1.lerp(angle2, btScalar(ww));
         }
         if (keyFrameForInterpolation->linear[4]) {
            m_distance = distance1 * (1.0f - w) + distance2 * w;
         } else {
            ww = keyFrameForInterpolation->interpolationTable[4][idx] + (keyFrameForInterpolation->interpolationTable[4][idx + 1] - keyFrameForInterpolation->interpolationTable[4][idx]) * (w * VMD_INTERPOLATIONTABLESIZE - idx);
            m_distance = distance1 * (1.0f - ww) + distance2 * ww;
         }
         if (keyFrameForInterpolation->linear[5]) {
            m_fovy = fovy1 * (1.0f - w) + fovy2 * w;
         } else {
            ww = keyFrameForInterpolation->interpolationTable[5][idx] + (keyFrameForInterpolation->interpolationTable[5][idx + 1] - keyFrameForInterpolation->interpolationTable[5][idx]) * (w * VMD_INTERPOLATIONTABLESIZE - idx);
            m_fovy = fovy1 * (1.0f - ww) + fovy2 * ww;
         }
      }
   } else {
      /* both keys have the same time, just apply one of them */
      m_distance = distance1;
      m_pos      = pos1;
      m_angle    = angle1;
      m_fovy     = fovy1;
   }
}

/* CameraController::initialize: initialize controller */
void CameraController::initialize()
{
   m_motion = NULL;
   m_currentFrame = 0.0;
   m_previousFrame = 0.0;
}

/* CameraController::clear: free controller */
void CameraController::clear()
{
   initialize();
}

/* CameraController::CameraController: constructor */
CameraController::CameraController()
{
   initialize();
}

/* CameraController::~CameraController: destructor */
CameraController::~CameraController()
{
   clear();
}

/* CameraController::setup: initialize and set up controller */
void CameraController::setup(VMD *vmd)
{
   clear();
   m_motion = vmd->getCameraMotion();
}

/* CameraController::reset: reset values */
void CameraController::reset()
{
   m_lastKey = 0;
   m_currentFrame = 0.0;
   m_previousFrame = 0.0;
}

/* CameraController::advance: advance motion controller by the given frame, return true when reached end */
bool CameraController::advance(double deltaFrame)
{
   if (m_motion == NULL)
      return false;

   /* apply motion at current frame to bones and faces */
   control((float) m_currentFrame);

   /* advance the current frame count */
   /* store the last frame to m_previousFrame */
   m_previousFrame = m_currentFrame;
   m_currentFrame += deltaFrame;

   if (m_currentFrame >= m_motion->keyFrameList[m_motion->numKeyFrame - 1].keyFrame) {
      /* we have reached the last key frame of this motion */
      /* clamp the frame to the maximum */
      m_currentFrame = m_motion->keyFrameList[m_motion->numKeyFrame - 1].keyFrame;
      /* return finished status */
      return true;
   }
   return false;
}

/* CameraController::getCurrentViewParam: get current view parameters */
void CameraController::getCurrentViewParam(float *distance, btVector3 *pos, btVector3 *angle, float *fovy)
{
   *distance = m_distance;
   *pos = m_pos;
   *angle = m_angle;
   *fovy = m_fovy;
}

/* CameraController::getCurrentFrame: get current frame */
double CameraController::getCurrentFrame()
{
   return m_currentFrame;
}

/* CameraController::setCurrentFrame: set current frame */
void CameraController::setCurrentFrame(double frame)
{
   m_currentFrame = frame;
}

/* CameraController::getPreviousFrame: get previous frame */
double CameraController::getPreviousFrame()
{
   return m_previousFrame;
}

/* CameraController::setPreviousFrame: set previous frame */
void CameraController::setPreviousFrame(double frame)
{
   m_previousFrame = frame;
}
