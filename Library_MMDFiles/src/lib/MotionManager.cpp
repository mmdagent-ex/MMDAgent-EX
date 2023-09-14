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

/* MotionPlayer_initialize: initialize MotionPlayer */
void MotionPlayer_initialize(MotionPlayer *m)
{
   m->name = NULL;

   m->vmd = NULL;

   m->onEnd = 2;
   m->priority = MOTIONMANAGER_DEFAULTPRIORITY;
   m->ignoreStatic = false;
   m->loopAt = MOTIONMANAGER_DEFAULTLOOPATFRAME;
   m->enableSmooth = true;
   m->enableRePos = true;
   m->endingBoneBlendFrames = MOTIONCONTROLLER_BONEENDMARGINFRAME;
   m->endingFaceBlendFrames = MOTIONCONTROLLER_FACEENDMARGINFRAME;
   m->motionBlendRate = 1.0f;

   m->active = true;
   m->endingBoneBlend = 0.0f;
   m->endingFaceBlend = 0.0f;
   m->statusFlag = MOTION_STATUS_RUNNING;

   m->targetSpeedRate = 1.0f;
   m->currentSpeedRate = 1.0f;
   m->remainingFramesForStartOfAcceleration = -1.0f;
   m->remainingFramesForEndOfAcceleration = -1.0f;
   m->accelerationStatusFlag = ACCELERATION_STATUS_CONSTANT;
   m->wantDeleteFlag = false;

   m->next = NULL;
}

/* MotionManager::terminateEndingMotion: terminate ending active motion */
void MotionManager::terminateEndingMotion(const char *name)
{
   MotionPlayer *m;

   for (m = m_playerList; m; m = m->next) {
      if (m->active == false)
         continue;
      if ((m->endingBoneBlend != 0.0f || m->endingFaceBlend != 0.0f) && MMDFiles_strequal(m->name, name)) {
         m->active = false;
      }
   }
}

/* MotionManager::purgeMotion: purge inactive motions */
void MotionManager::purgeMotion()
{
   MotionPlayer *m, *tmp1, *tmp2;

   tmp1 = NULL;
   m = m_playerList;
   while (m) {
      if (!m->active) {
         if (tmp1)
            tmp1->next = m->next;
         else
            m_playerList = m->next;
         tmp2 = m->next;
         if (m->name) free(m->name);
         delete m;
         m = tmp2;
      } else {
         tmp1 = m;
         m = m->next;
      }
   }
}

/* MotionManager::setup: initialize and setup motion manager */
void MotionManager::setup(PMDModel * pmd)
{
   clear();
   m_pmd = pmd;
}

/* MotionManager::startMotionSub: initialize a motion */
void MotionManager::startMotionSub(VMD * vmd, MotionPlayer * m)
{
   btVector3 offset;
   PMDBone *centerBone;
   btTransform tr;
   btVector3 pos;

   btVector3 centerPos;
   btVector3 rootOffset;

   /* initialize and setup motion controller */
   m->mc.setup(m_pmd, vmd);

   /* reset values */
   m->mc.reset();

   /* base motion does treat the bones with single motion frame at 0th frame as the same as normal bones */
   m->mc.setIgnoreSingleMotion(m->ignoreStatic);

   /* reset work area */
   m->vmd = vmd;
   m->active = true;
   m->endingBoneBlend = 0.0f;
   m->endingFaceBlend = 0.0f;
   /* when motion is changed, speed acceleration is turned off */
   m->accelerationStatusFlag = ACCELERATION_STATUS_CONSTANT;
   m->wantDeleteFlag = false;

   /* set model offset */
   if (m->enableSmooth) {
      offset.setZero();
      if (m->mc.hasCenter() && m->enableRePos) {
         /* when the started motion has center motion, the center position of the model will be moved to the current position */
         /* The current global position of the center bone will become the new offset of the root bone, and the local center position will be reset */
         centerBone = m_pmd->getCenterBone();
         /* calculate relative origin of center bone from model root bone */
         tr = m_pmd->getRootBone()->getTransform()->inverse();
         pos = tr * centerBone->getTransform()->getOrigin();
         /* get the translation vector */
         centerBone->getOriginPosition(&centerPos);
         offset = pos - centerPos;
         offset.setY(btScalar(0.0f)); /* Y axis should be set to zero to place model on ground */
         /* save the current pos/rot for smooth motion changing, resetting center location */
         m->mc.setOverrideFirst(&offset);
         /* add the offset to the root bone */
         m_pmd->getRootBone()->getOffset(&rootOffset);
         rootOffset += offset;
         m_pmd->getRootBone()->setOffset(&rootOffset);
         m_pmd->getRootBone()->update();
      } else {
         /* save the current pos/rot for smooth motion changing */
         m->mc.setOverrideFirst(NULL) ;
      }
   }
}

/* MotionManager::initialize: initialize motion manager */
void MotionManager::initialize()
{
   m_pmd = NULL;
   m_playerList = NULL;
   m_beginningNonControlledBlend = 0.0f;
}

/* MotionManager::clear: free motion manager */
void MotionManager::clear()
{
   MotionPlayer *player = m_playerList;
   MotionPlayer *tmp;

   while (player) {
      tmp = player->next;
      if (player->name) free(player->name);
      delete player;
      player = tmp;
   }
   initialize();
}

/* MotionManager::MotionManager: constructor */
MotionManager::MotionManager(PMDModel * pmd)
{
   initialize();
   setup(pmd);
}

/* MotionManager::~MotionManager: destructor */
MotionManager::~MotionManager()
{
   clear();
}

/* MotionManager::startMotion start a motion */
bool MotionManager::startMotion(VMD * vmd, const char *name, bool full, bool once, bool enableSmooth, bool enableRePos, float priority)
{
   MotionPlayer *m, *tmp1, *tmp2;

   if (vmd == NULL || name == NULL) return false;

   /* terminate motion of the same name to allow override if it is in ending state after deletion */
   terminateEndingMotion(name);

   /* purge inactive motion managers */
   purgeMotion();

   /* allocate new motion */
   m = new MotionPlayer;
   MotionPlayer_initialize(m);

   m->name = MMDFiles_strdup(name);
   m->priority = priority;
   m->onEnd = once ? 2 : 1; /* if loop is not specified, this motion will be deleted */
   m->ignoreStatic = full ? false : true;
   m->enableSmooth = enableSmooth;
   m->enableRePos = enableRePos;

   startMotionSub(vmd, m);

   /* set reset timer for bones/faces that are not controlled by the given base motion */
   if (!m->ignoreStatic)
      m_beginningNonControlledBlend = 10.0f;

   /* add this new motion to the last of the motion player list, consulting priority */
   if (m_playerList == NULL || m_playerList->priority > m->priority) {
      m->next = m_playerList;
      m_playerList = m;
   } else {
      tmp2 = m_playerList->next; /* skip the base motion */
      tmp1 = m_playerList;
      while (tmp2) {
         if (tmp2->priority > m->priority) {
            /* insert here */
            m->next = tmp2;
            tmp1->next = m;
            break;
         }
         tmp1 = tmp2;
         tmp2 = tmp2->next;
      }
      if (!tmp2) { /* append */
         m->next = NULL;
         tmp1->next = m;
      }
   }

   return true;
}

/* MotionManager::swapMotion: swap a motion, keeping parameters */
bool MotionManager::swapMotion(VMD * vmd, const char * name)
{
   MotionPlayer *m;

   if (vmd == NULL || name == NULL) return false;

   /* purge inactive motion managers */
   purgeMotion();

   /* find the motion player to change */
   for (m = m_playerList; m; m = m->next)
      if (MMDFiles_strequal(m->name, name) == true)
         break;
   if (!m)
      return false; /* not found */

   startMotionSub(vmd, m);

   /* set reset timer for bones/faces that are not controlled by the given base motion */
   if (!m->ignoreStatic)
      m_beginningNonControlledBlend = 10.0f;

   return true;
}

/* MotionManager::setMotionSpeedRate: set motion speed rate */
bool MotionManager::setMotionSpeedRate(const char *name, float speedRate, float changeLength, float targetFrameIndex)
{
   MotionPlayer *m;

   if (name == NULL || speedRate < 0.0f || changeLength < 0.0f) return false;

   for (m = m_playerList; m; m = m->next) {
      if (m->active && MMDFiles_strequal(m->name, name) == true) {
         m->targetSpeedRate = speedRate;
         if (targetFrameIndex < 0.0f) {
            m->remainingFramesForStartOfAcceleration = 0.0f;
            m->remainingFramesForEndOfAcceleration = changeLength;
         } else {
            m->remainingFramesForStartOfAcceleration = targetFrameIndex - (float) m->mc.getCurrentFrame();
            if (m->remainingFramesForStartOfAcceleration < 0.0f)
               m->remainingFramesForStartOfAcceleration += m->vmd->getMaxFrame();
            m->remainingFramesForEndOfAcceleration = m->remainingFramesForStartOfAcceleration + changeLength;
         }
         m->accelerationStatusFlag = ACCELERATION_STATUS_WAITING;
         return true;
      }
   }
   return false;
}

/* MotionManager::deleteMotion: delete a motion */
bool MotionManager::deleteMotion(const char *name)
{
   MotionPlayer *m;

   if (name == NULL) return false;

   for (m = m_playerList; m; m = m->next) {
      if (m->active && MMDFiles_strequal(m->name, name) == true) {
         m->wantDeleteFlag = true;
         return true;
      }
   }
   return false;
}

/* MotionManager::configureMotion: configure a motion */
bool MotionManager::configureMotion(const char *name, const char *key, const char *value)
{
   MotionPlayer *m;

   if (name == NULL) return false;

   for (m = m_playerList; m; m = m->next) {
      if (m->active && MMDFiles_strequal(m->name, name) == true) {
         return m->mc.configure(key, value);
      }
   }
   return false;
}

/* MotionManager::update: apply all motion players */
bool MotionManager::update(double frame)
{
   MotionPlayer *m;

   if (m_beginningNonControlledBlend > 0.0f) {
      /* if this is the beginning of a base motion, the uncontrolled bone/face will be reset */
      m_beginningNonControlledBlend -= (float) frame;
      if (m_beginningNonControlledBlend < 0.0f)
         m_beginningNonControlledBlend = 0.0f;
      m_pmd->smearAllBonesToDefault(m_beginningNonControlledBlend / 10.0f); /* from 1.0 to 0.0 in 10 frames */
   }

   /* reset status flags */
   for (m = m_playerList; m; m = m->next)
      m->statusFlag = MOTION_STATUS_RUNNING;

   /* update the whole motion (the later one will override the other one) */
   for (m = m_playerList; m; m = m->next) {
      /* skip deactivated motions */
      if (!m->active)
         continue;
      if (m->endingBoneBlend != 0.0f || m->endingFaceBlend != 0.0f) {
         /* this motion player is in ending status */
         if (m->wantDeleteFlag == true) {
            /* just return that delete was completed */
            m->statusFlag = MOTION_STATUS_DELETED;
            m->wantDeleteFlag = false;
         }
         m->mc.setBoneBlendRate(m->motionBlendRate * m->endingBoneBlend / m->endingBoneBlendFrames);
         m->mc.setFaceBlendRate(m->endingFaceBlend / m->endingFaceBlendFrames);
         /* proceed the motion */
         m->mc.advance(frame * m->currentSpeedRate);
         /* decrement the rest frames */
         m->endingBoneBlend -= (float) frame;
         m->endingFaceBlend -= (float) frame;
         if (m->endingBoneBlend < 0.0f) m->endingBoneBlend = 0.0f;
         if (m->endingFaceBlend < 0.0f) m->endingFaceBlend = 0.0f;
         if (m->endingBoneBlend == 0.0f && m->endingFaceBlend == 0.0f) {
            /* this motion player has reached the final end point */
            /* deactivate this motion player */
            /* this will be purged at next call of purgeMotion() */
            m->active = false;
         }
      } else {
         m->mc.setBoneBlendRate(m->motionBlendRate);
         m->mc.setFaceBlendRate(1.0f); /* does not apply blend rate for face morphs */
         /* proceed the motion */
         if (m->mc.advance(frame * m->currentSpeedRate)) {
            /* this motion player has reached end */
            switch (m->onEnd) {
            case 0:
               /* just keep the last pose */
               break;
            case 1:
               /* loop to a frame */
               if (m->mc.getMaxFrame() != 0.0f) { /* avoid infinite event loop when motion is void */
                  m->mc.rewind(m->loopAt, (float)(frame * m->currentSpeedRate));
                  m->statusFlag = MOTION_STATUS_LOOPED;
               }
               break;
            case 2:
               if (m->enableSmooth) {
                  /* enter the ending status, gradually decreasing the blend rate */
                  m->endingBoneBlend = m->endingBoneBlendFrames;
                  m->endingFaceBlend = m->endingFaceBlendFrames;
               } else {
                  /* deactivate this motion player immediately */
                  m->active = false;
               }
               m->statusFlag = MOTION_STATUS_DELETED; /* set return flag */
               break;
            }
         }
         /* this motion is in normal state */
         if (m->wantDeleteFlag == true) {
            /* delete requested */
            if (m->enableSmooth) {
               /* enter the ending status, gradually decreasing the blend rate */
               m->endingBoneBlend = m->endingBoneBlendFrames;
               m->endingFaceBlend = m->endingFaceBlendFrames;
            }
            else {
               /* deactivate this motion player immediately */
               m->active = false;
            }
            m->statusFlag = MOTION_STATUS_DELETED; /* set return flag */
            m->wantDeleteFlag = false;
         }
      }
   }

   /* return true when any status change has occurred within this call */
   for (m = m_playerList; m; m = m->next)
      if (m->statusFlag != MOTION_STATUS_RUNNING)
         return true;
   return false;
}

/* MotionManager::updateMotionSpeedRate: update motion speed rate */
bool MotionManager::updateMotionSpeedRate(double frame)
{
   MotionPlayer *m;
   float f;

   for (m = m_playerList; m; m = m->next) {
      if (!m->active || m->accelerationStatusFlag == ACCELERATION_STATUS_CONSTANT)
         continue;
      if (m->accelerationStatusFlag == ACCELERATION_STATUS_ENDED) {
         m->accelerationStatusFlag = ACCELERATION_STATUS_CONSTANT;
         continue;
      }
      f = (float) frame * m->currentSpeedRate;
      if (m->accelerationStatusFlag == ACCELERATION_STATUS_WAITING) {
         m->remainingFramesForStartOfAcceleration -= f;
         if (m->remainingFramesForStartOfAcceleration <= 0.0f)
            m->accelerationStatusFlag = ACCELERATION_STATUS_CHANGING;
      }
      m->remainingFramesForEndOfAcceleration -= f;
      if (m->accelerationStatusFlag == ACCELERATION_STATUS_CHANGING) {
         if (m->remainingFramesForEndOfAcceleration <= 0.0f) {
            m->currentSpeedRate = m->targetSpeedRate;
            m->accelerationStatusFlag = ACCELERATION_STATUS_ENDED;
         } else {
            m->currentSpeedRate += (m->targetSpeedRate - m->currentSpeedRate) * ((float) frame / (m->remainingFramesForEndOfAcceleration + (float) frame));
         }
      }
   }

   /* return true when any non-constant status exists within this call */
   for (m = m_playerList; m; m = m->next)
      if (m->active && m->accelerationStatusFlag == ACCELERATION_STATUS_ENDED)
         return true;
   return false;
}

/* MotionManager::getMotionPlayerList: get list of motion players */
MotionPlayer * MotionManager::getMotionPlayerList()
{
   return m_playerList;
}

/* MotionManager::getRunning: get the running motion player */
MotionPlayer *MotionManager::getRunning(const char *name)
{
   MotionPlayer *m;

   for (m = m_playerList; m; m = m->next) {
      if (m->active == false)
         continue;
      if (m->endingBoneBlend != 0.0f || m->endingFaceBlend != 0.0f)
         continue;
      if (MMDFiles_strequal(m->name, name)) {
         return m;
      }
   }
   return NULL;
}

/* MotionManager::updateModel: update model */
void MotionManager::updateModel(PMDModel *pmd)
{
   MotionPlayer *motionPlayer;
   double currentFrame;
   double previousFrame;

   for (motionPlayer = m_playerList; motionPlayer; motionPlayer = motionPlayer->next) {
      currentFrame = motionPlayer->mc.getCurrentFrame();
      previousFrame = motionPlayer->mc.getPreviousFrame();
      motionPlayer->mc.setup(pmd, motionPlayer->vmd);
      motionPlayer->mc.setCurrentFrame(currentFrame);
      motionPlayer->mc.setPreviousFrame(previousFrame);
   }
   m_pmd = pmd;

}
