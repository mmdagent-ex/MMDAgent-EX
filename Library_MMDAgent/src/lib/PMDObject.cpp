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

#include "MMDAgent.h"

/* intial number of frame steps to allocate capture buffer (30 = 1 sec.) */
#define PMDOBJECT_MOTIONCAPTURE_FRAMESTEP 300

/* PMDObject::initialize: initialize PMDObject */
void PMDObject::initialize()
{
   m_alias = NULL;
   m_pmd = NULL;
   m_motionManager = NULL;

   m_globalLipSync = NULL;
   m_localLipSync = NULL;

   m_isEnable = false;

   m_lightDir = btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f));

   m_assignTo = NULL;
   m_baseBone = NULL;
   m_origBasePos = btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f));

   m_offsetPos = btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f));
   m_offsetRot = btQuaternion(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));
   m_absPosFlag[0] = false;
   m_absPosFlag[1] = false;
   m_absPosFlag[2] = false;
   m_moveSpeed = -1.0f;
   m_spinSpeed = -1.0f;
   m_useCartoonRendering = true;
   m_allowMotionFileDrop = true;

   m_isMoving = false;
   m_isRotating = false;
   m_underTurn = false;

   m_alphaAppearFrame = 0.0;
   m_alphaDisappearFrame = 0.0;
   m_displayCommentFrame = 0.0;

   m_needResetKinematic = false;

   memset(&m_commentElem, 0, sizeof(FTGLTextDrawElements));
   memset(&m_errorElem, 0, sizeof(FTGLTextDrawElements));
   memset(&m_nameElem, 0, sizeof(FTGLTextDrawElements));

   for (int i = 0; i < PMDOBJECT_MAXNUMBIND; i++)
      m_boneFaceControl[i] = NULL;

   for (int i = 0; i < PMDOBJECT_MAXNUMMESSAGE; i++) {
      m_loadMessage[i] = NULL;
      m_deleteMessage[i] = NULL;
   }
   m_loadMessageNum = 0;
   m_deleteMessageNum = 0;

   m_shapeMap = NULL;
   m_isShapeMapDefault = false;

   m_motionCaptureSaveFileName = NULL;
   m_motionCaptureDataStorage[0] = m_motionCaptureDataStorage[1] = NULL;

   m_loadingProgressRate = -1.0f;

   m_mutex = glfwCreateMutex();
}

/* PMDOjbect::clear: free PMDObject */
void PMDObject::clear()
{
   if (m_pmd) {
      m_pmd->~PMDModel();
      MMDFiles_alignedfree(m_pmd);
   }
   if (m_motionManager)
      delete m_motionManager;
   if (m_localLipSync)
      delete m_localLipSync;
   if(m_alias)
      free(m_alias);

   if(m_commentElem.vertices) free(m_commentElem.vertices);
   if(m_commentElem.texcoords) free(m_commentElem.texcoords);
   if(m_commentElem.indices) free(m_commentElem.indices);
   if(m_errorElem.vertices) free(m_errorElem.vertices);
   if(m_errorElem.texcoords) free(m_errorElem.texcoords);
   if(m_errorElem.indices) free(m_errorElem.indices);
   if(m_nameElem.vertices) free(m_nameElem.vertices);
   if(m_nameElem.texcoords) free(m_nameElem.texcoords);
   if(m_nameElem.indices) free(m_nameElem.indices);

   for (int i = 0; i < PMDOBJECT_MAXNUMBIND; i++)
      if (m_boneFaceControl[i]) {
         m_boneFaceControl[i]->~BoneFaceControl();
         MMDFiles_alignedfree(m_boneFaceControl[i]);
      }

   clearLoadMessages();

   if (m_shapeMap)
      delete m_shapeMap;

   if (m_motionCaptureSaveFileName)
      stopCapture();

   if (m_motionCaptureDataStorage[0])
      free(m_motionCaptureDataStorage[0]);
   if (m_motionCaptureDataStorage[1])
      free(m_motionCaptureDataStorage[1]);

   if (m_mutex)
      glfwDestroyMutex(m_mutex);

   initialize();
}

/* PMDObject::PMDObject: constructor */
PMDObject::PMDObject()
{
   initialize();
}

/* PMDObject::PMDObject: destructor */
PMDObject::~PMDObject()
{
   clear();
}

/* PMDOjbect::release: free PMDObject */
void PMDObject::release()
{
   clear();
}

/* PMDObject::load: load model */
bool PMDObject::load(const char *fileName, const char *alias, btVector3 *offsetPos, btQuaternion *offsetRot, bool forcedPosition, PMDBone *assignBone, PMDObject *assignObject, BulletPhysics *bullet, SystemTexture *systex, LipSync *sysLipSync, bool useCartoonRendering, float cartoonEdgeWidth, bool useLightEdge, btVector3 *light, float commentFrame, PMDModel *preloadedPMDModel, const char *appDirName)
{
   int i;
   int len;
   char *buf;
   LipSync *lip;
   char buff[MMDAGENT_MAXBUFLEN];

   if (fileName == NULL || alias == NULL) return false;

   if (m_pmd) {
      m_pmd->~PMDModel();
      MMDFiles_alignedfree(m_pmd);
   }
   if (preloadedPMDModel != NULL)
      m_pmd = preloadedPMDModel;
   else {
      void *ptr = MMDFiles_alignedmalloc(sizeof(PMDModel), 16);
      m_pmd = new(ptr) PMDModel();
   }

   /* apply given parameters */
   m_assignTo = assignObject;
   m_baseBone = assignBone;

   if (forcedPosition) {
      /* set offset by given parameters */
      if (offsetPos)
         m_offsetPos = (*offsetPos);
      if (offsetRot)
         m_offsetRot = (*offsetRot);
   } /* else, remain last offset */
   m_pmd->getRootBone()->setOffset(&m_offsetPos);
   m_pmd->getRootBone()->update();

   /* copy absolute position flag */
   for (i = 0; i < 3; i++)
      m_absPosFlag[i] = false;

   /* copy toon rendering flag */
   m_useCartoonRendering = useCartoonRendering;

   /* copy flag for motion file drop or all motion */
   if(assignBone || assignObject)
      m_allowMotionFileDrop = false;
   else
      m_allowMotionFileDrop = true;

   /* save position when position is fixed */
   if (m_baseBone) m_origBasePos = m_baseBone->getTransform()->getOrigin();

   /* set alpha frame */
   m_alphaAppearFrame = PMDOBJECT_ALPHAFRAME;
   m_alphaDisappearFrame = 0.0;

   /* set comment frame */
   m_displayCommentFrame = commentFrame;

   /* reset text rendering elements */
   m_commentElem.textLen = 0;
   m_commentElem.numIndices = 0;
   m_errorElem.textLen = 0;
   m_errorElem.numIndices = 0;
   m_nameElem.textLen = 0;
   m_nameElem.numIndices = 0;

   /* load model */
   /* if already have a preloaded texture made in another thread, use it for loading */
   if (preloadedPMDModel == NULL && m_pmd->load(fileName, bullet, systex) == false) {
      clear();
      return false;
   }

   /* set toon rendering flag */
   m_pmd->setToonFlag(useCartoonRendering);
   m_pmd->setLightEdge(useLightEdge);

   /* set edge width */
   m_pmd->setEdgeThin(cartoonEdgeWidth);

   /* load lip sync */
   m_globalLipSync = sysLipSync;
   if(m_localLipSync != NULL)
      delete m_localLipSync;
   m_localLipSync = NULL;
   lip = new LipSync();
   len = MMDAgent_strlen(fileName);
   if(len < 5) {
      delete lip;
   } else {
      buf = MMDAgent_strdup(fileName);
      buf[len - 4] = '.';
      buf[len - 3] = 'l';
      buf[len - 2] = 'i';
      buf[len - 1] = 'p';
      if(lip->load(buf) == true) {
         m_localLipSync = lip;
      } else
         delete lip;
      if(buf)
         free(buf);
   }

   /* set alias */
   setAlias(alias);

   /* reset */
   setLightForToon(light);
   m_moveSpeed = -1.0f;
   m_spinSpeed = -1.0f;
   for (int i = 0; i < PMDOBJECT_MAXNUMBIND; i++) {
      if (m_boneFaceControl[i]) {
         m_boneFaceControl[i]->setModel(m_pmd);
         m_boneFaceControl[i]->update(0.0);
      }
   }

   /* set temporarily all body to Kinematic */
   /* this is fixed at first simulation */
   skipNextSimulation();

   /* load start / end messages if exist */
   clearLoadMessages();
   loadModelMessagesFromFile(fileName);

   /* load shape2morph mapping data */
   if (m_shapeMap) {
      delete m_shapeMap;
      m_shapeMap = NULL;
   }
   m_isShapeMapDefault = false;
   char *mapFileName = (char *)malloc(MMDAgent_strlen(fileName) + 10);
   strcpy(mapFileName, fileName);
   strcat(mapFileName, ".shapemap");
   if (MMDAgent_exist(mapFileName)) {
      char *dirname = MMDFiles_dirname(fileName);
      m_shapeMap = new ShapeMap();
      if (m_shapeMap->load(mapFileName, m_pmd, dirname) == false) {
         delete m_shapeMap;
         m_shapeMap = NULL;
      }
      free(dirname);
   } else {
      /* if not found, try to use system default */
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", appDirName, MMDAGENT_DIRSEPARATOR, SHAPEMAP_DEFAULT_FILENAME);
      if (MMDAgent_exist(buff)) {
         m_shapeMap = new ShapeMap();
         if (m_shapeMap->load(buff, m_pmd, appDirName) == false) {
            delete m_shapeMap;
            m_shapeMap = NULL;
         } else {
            m_isShapeMapDefault = true;
         }
      }
   }
   free(mapFileName);

   /* stop motion capturing if running */
   if (m_motionCaptureSaveFileName)
      stopCapture();

   /* enable */
   m_isEnable = true;

   return true;
}

/* PMDObject::skipNextSimulation: skip next physics simulation */
void PMDObject::skipNextSimulation()
{
   m_needResetKinematic = true;
#ifdef MY_RESETPHYSICS
   m_pmd->setPhysicsControl(false, true);
#else
   m_pmd.setPhysicsControl(false);
#endif
}

/* PMDObject::setMotion: start a motion */
bool PMDObject::startMotion(VMD * vmd, const char *name, bool full, bool once, bool enableSmooth, bool enableRepos, float priority)
{
   if (m_motionManager == NULL || m_motionManager->startMotion(vmd, name, full, once, enableSmooth, enableRepos, priority) == false)
      return false;
   if (enableRepos)
      m_pmd->getRootBone()->getOffset(&m_offsetPos);

   return true;
}

/* PMDObject::swapMotion: swap a motion */
bool PMDObject::swapMotion(VMD * vmd, const char *name)
{
   MotionPlayer *m;

   if (m_motionManager == NULL || m_motionManager->swapMotion(vmd, name) == false)
      return false;
   for (m = m_motionManager->getMotionPlayerList(); m; m = m->next) {
      if (MMDFiles_strequal(m->name, name) == true) {
         if (m->enableRePos == true)
            m_pmd->getRootBone()->getOffset(&m_offsetPos);
         break;
      }
   }

   return true;
}

/* PMDObject::updateRootBone: update root bone if assigned to a base bone */
void PMDObject::updateRootBone()
{
   btVector3 pos;
   btVector3 posAbs;
   PMDBone *b;
   btTransform tr;

   if (!m_baseBone) return;

   /* relative position */
   pos = m_offsetPos;
   /* if absolute flag is true, fix relative position from root bone */
   posAbs = m_offsetPos + m_origBasePos - m_baseBone->getTransform()->getOrigin();
   if (m_absPosFlag[0]) pos.setX(posAbs.x());
   if (m_absPosFlag[1]) pos.setY(posAbs.y());
   if (m_absPosFlag[2]) pos.setZ(posAbs.z());

   /* set root bone */
   b = m_pmd->getRootBone();
   b->setCurrentPosition(&pos);
   b->setCurrentRotation(&m_offsetRot);
   b->update();
   /* update transform for base position */
   tr = (*m_baseBone->getTransform()) * (*b->getTransform());
   b->setTransform(&tr);
}

/* PMDObject::updateMotion: update motions */
bool PMDObject::updateMotion(double deltaFrame)
{
   bool ret;

   if (m_isEnable == false || m_motionManager == NULL) return false;

   /* set rotation and position to bone and face from motion */
   m_pmd->resetBone();  /* reset bone position */
   ret = m_motionManager->update(deltaFrame); /* set from motion */
   m_pmd->updateBone(false); /* update bone, IK, and rotation before simulation */
   for (int i = 0; i < PMDOBJECT_MAXNUMBIND; i++) {
      if (m_boneFaceControl[i])
         m_boneFaceControl[i]->update(deltaFrame); /* update bone/face controller */
   }
   /* update comment frame */
   if (m_displayCommentFrame > 0.0f) {
      m_displayCommentFrame -= deltaFrame;
      if (m_displayCommentFrame < 0.0f)
         m_displayCommentFrame = 0.0f;
   }
   /* update texture animation */
   m_pmd->getTextureLoader()->update(deltaFrame);

   return ret;
}

/* PMDObject::updateAfterSimulation: update bone transforms from simulated rigid bodies */
void PMDObject::updateAfterSimulation(bool physicsEnabled)
{
   if (m_isEnable == false) return;

   /* if necessary, change state of Bullet Physics */
   if (m_needResetKinematic) {
#ifdef MY_RESETPHYSICS
      if (physicsEnabled) m_pmd->setPhysicsControl(true, true);
#else
      if (physicsEnabled) m_pmd->setPhysicsControl(true);
#endif
      m_needResetKinematic = false;
   }
   /* apply calculation result to bone */
   m_pmd->updateBoneFromSimulation();

   m_pmd->updateBone(true); /* update bone, IK, and rotation after simulation */
}

/* PMDObject::updateSkin: update skin and toon */
void PMDObject::updateSkin()
{
   if (m_isEnable == false) return;

   /* update skin and toon */
   m_pmd->setToonLight(&m_lightDir);
   m_pmd->updateFace();
   m_pmd->updateSkin();
}

/* PMDObject::updateAlpha: update global model alpha */
bool PMDObject::updateAlpha(double deltaFrame)
{
   bool ended = false;

   if (m_alphaAppearFrame > 0.0f) {
      m_alphaAppearFrame -= deltaFrame;
      if (m_alphaAppearFrame < 0.0f)
         m_alphaAppearFrame = 0.0f;
      m_pmd->setGlobalAlpha((float)(1.0 - m_alphaAppearFrame / PMDOBJECT_ALPHAFRAME));
   }
   if (m_alphaDisappearFrame > 0.0f) {
      m_alphaDisappearFrame -= deltaFrame;
      if (m_alphaDisappearFrame <= 0.0f) {
         m_alphaDisappearFrame = 0.0f;
         ended = true; /* model was deleted */
      }
      m_pmd->setGlobalAlpha((float) (m_alphaDisappearFrame / PMDOBJECT_ALPHAFRAME));
   }
   return ended;
}

/* PMDObject::startDisppear: set disappear timer */
void PMDObject::startDisappear()
{
   m_alphaDisappearFrame = PMDOBJECT_ALPHAFRAME;
}

/* PMDModel::setLightForToon: set light direction for ton shading */
void PMDObject::setLightForToon(btVector3 * v)
{
   m_lightDir = (*v);
   m_lightDir.normalize();
}

/* PMDObject::updateModel: update model position of root bone */
bool PMDObject::updateModelRootOffset(float fps)
{
   bool ret = false;
   PMDBone *b;
   btVector3 pos, pos2;
   float diff;
   float maxStep;

   if (m_isEnable == false) return false;

   /* get root bone */
   b = m_pmd->getRootBone();

   /* target position is m_offsetPos */
   /* move offset of root bone closer to m_offsetPos */
   b->getOffset(&pos);
   m_isMoving = false;
   if (m_offsetPos != pos) {
      /* if there is difference then update */
      diff = pos.distance(m_offsetPos);
      if (diff > PMDOBJECT_MINMOVEDIFF) {
         if (m_moveSpeed >= 0.0f && fps != 0.0f) {
            /* max speed */
            maxStep = m_moveSpeed / fps;
            if (diff > maxStep) {
               pos2 = pos.lerp(m_offsetPos, btScalar(maxStep / diff));
               m_isMoving = true;
            } else {
               pos2 = m_offsetPos;
               ret = true;
            }
         } else {
            /* current * 0.9 + target * 0.1 */
            pos2 = pos.lerp(m_offsetPos, btScalar(1.0f - PMDOBJECT_MOVESPEEDRATE));
            m_isMoving = true;
         }
      } else {
         /* set target offset directory if small difference */
         pos2 = m_offsetPos;
         ret = true;
      }
      m_pmd->getRootBone()->setOffset(&pos2);
      m_pmd->getRootBone()->update();
   }

   return ret;
}

/* PMDObject::updateModelRootRotation: update model rotation of root bone */
bool PMDObject::updateModelRootRotation(float fps)
{
   btQuaternion tmpRot;
   PMDBone *b;
   bool ret = false;
   btQuaternion r;
   float diff;
   float maxStep;

   if (m_isEnable == false) return false;

   m_isRotating = false;

   /* get root bone */
   b = m_pmd->getRootBone();
   /* target rotation is m_offsetRot */
   /* turn rotation of root bone closer to m_offsetRot */
   b->getCurrentRotation(&r);
   if (m_offsetRot != r) {
      /* difference calculation */
      r = r - m_offsetRot;
      diff = r.length();
      if (diff > PMDOBJECT_MINSPINDIFF) {
         if (m_spinSpeed >= 0.0f && fps != 0.0f) {
            /* max turn speed */
            maxStep = MMDFILES_RAD(m_spinSpeed) / fps;
            if (diff > maxStep) {
               b->getCurrentRotation(&tmpRot);
               tmpRot = tmpRot.slerp(m_offsetRot, btScalar(maxStep / diff));
               b->setCurrentRotation(&tmpRot);
               m_isRotating = true;
            } else {
               b->setCurrentRotation(&m_offsetRot);
               ret = true;
            }
         } else {
            /* current * 0.95 + target * 0.05 */
            b->getCurrentRotation(&tmpRot);
            tmpRot = tmpRot.slerp(m_offsetRot, btScalar(1.0f - PMDOBJECT_SPINSPEEDRATE));
            b->setCurrentRotation(&tmpRot);
            m_isRotating = true;
         }
      } else {
         /* set target offset directory if small difference */
         b->setCurrentRotation(&m_offsetRot);
         ret = true;
      }
      b->update();
   }

   return ret;
}

/* PMDObject::getAlias: get alias name */
char *PMDObject::getAlias()
{
   return m_alias;
}

/* PMDObject::setAlias: set alias name */
void PMDObject::setAlias(const char *alias)
{
   if(MMDAgent_strlen(alias) > 0 && m_alias != alias) {
      if(m_alias)
         free(m_alias);
      m_alias = MMDAgent_strdup(alias);
   }
}

/* PMDObject::getPMDModel: get PMDModel */
PMDModel *PMDObject::getPMDModel()
{
   return m_pmd;
}

/* PMDObject::getMotionManager: get MotionManager */
MotionManager *PMDObject::getMotionManager()
{
   return m_motionManager;
}

/* PMDObject::resetMotionManager: reset MotionManager */
void PMDObject::resetMotionManager()
{
   if (m_motionManager)
      delete m_motionManager;
   m_motionManager = new MotionManager(m_pmd);
}

/* PMDObject::createLipSyncMotion: create LipSync motion */
bool PMDObject::createLipSyncMotion(const char *str, unsigned char **rawData, unsigned int *rawSize)
{
   const char *ignoreLipList = NULL;

   if (m_shapeMap)
      ignoreLipList = m_shapeMap->getLipIgnoreList();
   if(m_localLipSync != NULL && m_localLipSync->createMotion(str, rawData, rawSize, ignoreLipList) == true)
      return true;
   if(m_globalLipSync != NULL && m_globalLipSync->createMotion(str, rawData, rawSize, ignoreLipList) == true)
      return true;
   return false;
}


/* PMDObject::getCurrentPosition: get current offset */
void PMDObject::getCurrentPosition(btVector3 *pos)
{
   m_pmd->getRootBone()->getOffset(pos);
}

/* PMDObject::getTargetPosition: get target offset */
void PMDObject::getTargetPosition(btVector3 *pos)
{
   (*pos) = m_offsetPos;
}

/* PMDObject::setPosition: set root bone offset */
void PMDObject::setPosition(btVector3 *pos)
{
   m_offsetPos = (*pos);
}

/* PMDObject::getCurrentRotation: get current rotation */
void PMDObject::getCurrentRotation(btQuaternion *rot)
{
   m_pmd->getRootBone()->getCurrentRotation(rot);
}

/* PMDObject::getTargetRotation: get target rotation */
void PMDObject::getTargetRotation(btQuaternion *rot)
{
   (*rot) = m_offsetRot;
}

/* PMDObject::setRotation: set root bone rotation */
void PMDObject::setRotation(btQuaternion *rot)
{
   m_offsetRot = (*rot);
}

/* PMDObject::setMoveSpeed: set move speed per second */
void PMDObject::setMoveSpeed(float speed)
{
   m_moveSpeed = speed;
}

/* PMDObject::setSpinSpeed: set spin seed per second */
void PMDObject::setSpinSpeed(float speed)
{
   m_spinSpeed = speed;
}

/* PMDObject::isMoving: return true when model move */
bool PMDObject::isMoving()
{
   return m_isMoving;
}

/* PMDObject::isRotating: return true when model spin */
bool PMDObject::isRotating()
{
   return m_isRotating;
}

/* PMDObject::isTruning: return true when model turn */
bool PMDObject::isTurning()
{
   return m_underTurn;
}

/* PMDObject::setTurnFlag: set turnning flag */
void PMDObject::setTurningFlag(bool flag)
{
   m_underTurn = flag;
}

/* PMDObject::isEnable: get enable flag */
bool PMDObject::isEnable()
{
   return m_isEnable;
}

/* PMDObject::setEnableFlag: set enable flag */
void PMDObject::setEnableFlag(bool flag)
{
   m_isEnable = flag;
}

/* PMDObject::useCartoonRendering: return true if cartoon rendering is enabled */
bool PMDObject::useCartoonRendering()
{
   return m_useCartoonRendering;
}

/* PMDObject::allowMotionFileDrop: return true if motion file drop is allowed */
bool PMDObject::allowMotionFileDrop()
{
   return m_allowMotionFileDrop;
}

/* PMDObject::getAssignedModel: get parent model */
PMDObject *PMDObject::getAssignedModel()
{
   return m_assignTo;
}

/* PMDObject::renderText: render model name, comment and error text */
void PMDObject::renderText(FTGLTextureFont *font, bool displayModelNameFlag)
{
   char buf[MMDAGENT_MAXBUFLEN];
   btVector3 pos;
   float w, h;
   float tpos[3];
   bool texture_loaded = false;
   GLfloat vertices[] = { -1, -1, 0, /* bottom left corner */
                          -1, 1, 0,  /* top left corner */
                          1, 1, 0,   /* top right corner */
                          1, -1, 0   /* bottom right corner */
                        };
   GLubyte indices[] = { 0, 1, 2, 0, 2, 3 };
   unsigned int currentIndex;

   if (font == NULL) return;

   /* threaded loading progress bar */
   if (m_loadingProgressRate > 0.0f) {
      w = 2.0f;
      h = 0.2f;
      pos = m_pmd->getCenterBone()->getTransform()->getOrigin();
      pos.setX(btScalar(pos.x() - w * 0.5f));
      pos.setY(btScalar(m_pmd->getMaxHeight() + 0.1f));
      pos.setZ(btScalar(pos.z() + 5.0f));
      w *= m_loadingProgressRate;
      glDisable(GL_LIGHTING);
      glPushMatrix();
      glEnableClientState(GL_VERTEX_ARRAY);
      glTranslatef(pos.x(), pos.y(), pos.z());
      glNormal3f(0.0, 0.0, 1.0);
      glColor4f(0.0f, 0.0f, 1.0f, 0.5f);
      vertices[0] = 0;
      vertices[1] = 0;
      vertices[2] = 0;
      vertices[3] = w;
      vertices[4] = 0;
      vertices[5] = 0;
      vertices[6] = w;
      vertices[7] = h;
      vertices[8] = 0;
      vertices[9] = 0;
      vertices[10] = h;
      vertices[11] = 0;
      glVertexPointer(3, GL_FLOAT, 0, vertices);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
      glDisableClientState(GL_VERTEX_ARRAY);
      glPopMatrix();
      glEnable(GL_LIGHTING);
   }

   /* comment */
   if (m_displayCommentFrame > 0.0 && m_pmd->getComment() != NULL) {
      if (m_commentElem.numIndices == 0) {
         /* first time since load, create rendering element */
         if (font->getTextDrawElements(m_pmd->getComment(), &m_commentElem, 0, 0.0f, 0.0f, 0.0f) == false) {
            m_commentElem.numIndices = 0xFFFF; /* error */
         }
      }
   }
   if (m_displayCommentFrame > 0.0 && m_commentElem.numIndices != 0 && m_commentElem.numIndices != 0xFFFF) {
      pos = m_pmd->getCenterBone()->getTransform()->getOrigin();
      w = 13.0f;
      h = 5.0f;
      pos.setX(btScalar(pos.x() - w * 0.5f));
      pos.setZ(btScalar(pos.z() + 5.2f));
      glDisable(GL_LIGHTING);
      if (texture_loaded == false) {
         glEnable(GL_TEXTURE_2D);
         glActiveTexture(GL_TEXTURE0);
         glClientActiveTexture(GL_TEXTURE0);
         glEnableClientState(GL_VERTEX_ARRAY);
         glEnableClientState(GL_TEXTURE_COORD_ARRAY);
         texture_loaded = true;
      }
      glPushMatrix();
      glTranslatef(pos.x() - 0.3f, pos.y() - 0.3f, pos.z() - 0.01f);
      glNormal3f(0.0, 0.0, 1.0);
      glColor4f(0.0f, 0.0f, 0.0f, 0.4f);
      glBindTexture(GL_TEXTURE_2D, 0);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      vertices[0] = 0;
      vertices[1] = 0;
      vertices[2] = 0;
      vertices[3] = w;
      vertices[4] = 0;
      vertices[5] = 0;
      vertices[6] = w;
      vertices[7] = h;
      vertices[8] = 0;
      vertices[9] = 0;
      vertices[10] = h;
      vertices[11] = 0;
      glVertexPointer(3, GL_FLOAT, 0, vertices);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glPopMatrix();
      glColor4f(0.7f, 0.8f, 0.5f, 1.0f);
      tpos[0] = pos.x();
      tpos[1] = pos.y() + 4.0f;
      tpos[2] = pos.z();
      glPushMatrix();
      glTranslatef(tpos[0], tpos[1], tpos[2]);
      glScalef(0.7f, 0.7f, 0.7f);
      glVertexPointer(3, GL_FLOAT, 0, m_commentElem.vertices);
      glTexCoordPointer(2, GL_FLOAT, 0, m_commentElem.texcoords);
      glBindTexture(GL_TEXTURE_2D, font->getTextureID());
      glDrawElements(GL_TRIANGLES, m_commentElem.numIndices, GL_INDICES, (const GLvoid *) m_commentElem.indices);
      glPopMatrix();
      glEnable(GL_LIGHTING);
   }

   /* error */
   m_pmd->getErrorTextureList(buf, MMDAGENT_MAXBUFLEN);
   if (MMDAgent_strlen(buf) > 0) {
      if (m_errorElem.numIndices == 0) {
         if (font->getTextDrawElements(buf, &m_errorElem, 0, 0.0f, 0.0f, 0.0f) == false) {
            m_errorElem.numIndices = 0xFFFF; /* error */
         }
      }
   }
   if (m_errorElem.numIndices != 0 && m_errorElem.numIndices != 0xFFFF) {
      pos = m_pmd->getCenterBone()->getTransform()->getOrigin();
      pos.setZ(btScalar(pos.z() + 5.0f));
      glDisable(GL_LIGHTING);
      if (texture_loaded == false) {
         glEnable(GL_TEXTURE_2D);
         glActiveTexture(GL_TEXTURE0);
         glClientActiveTexture(GL_TEXTURE0);
         glEnableClientState(GL_VERTEX_ARRAY);
         glEnableClientState(GL_TEXTURE_COORD_ARRAY);
         texture_loaded = true;
      }
      glPushMatrix();
      glTranslatef(pos.x() - 0.3f, pos.y() - 0.3f, pos.z() - 0.01f);
      w = 10.0f;
      h = 6.0f;
      glNormal3f(0.0, 0.0, 1.0);
      glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
      glBindTexture(GL_TEXTURE_2D, 0);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      vertices[0] = 0;
      vertices[1] = 0;
      vertices[2] = 0;
      vertices[3] = w;
      vertices[4] = 0;
      vertices[5] = 0;
      vertices[6] = w;
      vertices[7] = h;
      vertices[8] = 0;
      vertices[9] = 0;
      vertices[10] = h;
      vertices[11] = 0;
      glVertexPointer(3, GL_FLOAT, 0, vertices);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glPopMatrix();
      glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
      tpos[0] = pos.x();
      tpos[1] = pos.y() + 5.0f;
      tpos[2] = pos.z();
      glPushMatrix();
      glTranslatef(tpos[0], tpos[1], tpos[2]);
      glVertexPointer(3, GL_FLOAT, 0, m_errorElem.vertices);
      glTexCoordPointer(2, GL_FLOAT, 0, m_errorElem.texcoords);
      glBindTexture(GL_TEXTURE_2D, font->getTextureID());
      glDrawElements(GL_TRIANGLES, m_errorElem.numIndices, GL_INDICES, (const GLvoid *) m_errorElem.indices);
      glPopMatrix();
      glEnable(GL_LIGHTING);
   }

   /* model name */
   if (displayModelNameFlag) {
      m_nameElem.textLen = 0; /* reset */
      m_nameElem.numIndices = 0;
      pos = m_pmd->getCenterBone()->getTransform()->getOrigin();
      /* render model name */
      if (m_pmd->getName()) {
         MMDAgent_snprintf(buf, MMDAGENT_MAXBUFLEN, "%s(%s)", m_alias, m_pmd->getName());
      } else {
         MMDAgent_snprintf(buf, MMDAGENT_MAXBUFLEN, "%s", m_alias);
      }
      if (font->getTextDrawElements(buf, &m_nameElem, 0, pos.x() + 2.0f, m_pmd->getMaxHeight() + 2.0f, 0.0f) == false) {
         m_nameElem.textLen = 0; /* reset */
         m_nameElem.numIndices = 0;
      }
      /* render motion names */
      float dest;
      MotionPlayer *motionPlayer;
      currentIndex = m_nameElem.numIndices;
      for (dest = 1.1f, motionPlayer = getMotionManager()->getMotionPlayerList(); motionPlayer; motionPlayer = motionPlayer->next) {
         if (motionPlayer->active) {
            if (font->getTextDrawElements(motionPlayer->name, &m_nameElem, m_nameElem.textLen, pos.x() + 2.0f + dest, m_pmd->getMaxHeight() + 2.0f - dest, 0.0f) == false)
               continue;
            dest += 1.1f;
         }
      }
   }
   if (displayModelNameFlag && m_nameElem.numIndices != 0) {
      glDisable(GL_LIGHTING);
      if (texture_loaded == false) {
         glEnable(GL_TEXTURE_2D);
         glActiveTexture(GL_TEXTURE0);
         glClientActiveTexture(GL_TEXTURE0);
         glEnableClientState(GL_VERTEX_ARRAY);
         glEnableClientState(GL_TEXTURE_COORD_ARRAY);
         texture_loaded = true;
      }
      glBindTexture(GL_TEXTURE_2D, font->getTextureID());
      glPushMatrix();
      glVertexPointer(3, GL_FLOAT, 0, m_nameElem.vertices);
      glTexCoordPointer(2, GL_FLOAT, 0, m_nameElem.texcoords);
      glTranslatef(0.0f, 0.0f, pos.z());
      if (currentIndex > 0) {
         glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
         glDrawElements(GL_TRIANGLES, currentIndex, GL_INDICES, (const GLvoid *)m_nameElem.indices);
      }
      if (m_nameElem.numIndices > currentIndex) {
         glColor4f(0.0f, 1.0f, 1.0f, 1.0f);
         glDrawElements(GL_TRIANGLES, m_nameElem.numIndices - currentIndex, GL_INDICES, (const GLvoid *) & (m_nameElem.indices[currentIndex]));
      }
      glPopMatrix();
   }
   if (texture_loaded == true) {
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glDisableClientState(GL_VERTEX_ARRAY);
      glDisable(GL_TEXTURE_2D);
   }
}

/* PMDObject::renderModelDebug: render model debug */
void PMDObject::renderModelDebug()
{
   btVector3 pos, x, y;
   unsigned int i, j;
   float dest;
   MotionPlayer *motionPlayer;
   MotionControllerBoneElement *b;
   GLfloat v[6];
   GLfloat vertices0[12] = { -7.5f, 0.0f, -1.0f, 7.5f, 0.0f, -1.0f, -7.5f, 20.0f, -1.0f, 7.5f, 20.0f, -1.0f };
   GLfloat vertices1[12] = { -7.5f, 0.0f, -7.5f, 7.5f, 0.0f, -7.5f, -7.5f, 0.0f, 7.5f, 7.5f, 0.0f, 7.5f };

   glPushMatrix();

   /* render debug */
   m_pmd->renderDebug();

   /* render motion skeletons */
   pos = m_pmd->getCenterBone()->getTransform()->getOrigin();
   dest = 0.7f;
   glDisable(GL_LIGHTING);
   glEnableClientState(GL_VERTEX_ARRAY);
   glColor4f(0.0f, 1.0f, 1.0f, 1.0f);
   for (motionPlayer = getMotionManager()->getMotionPlayerList(); motionPlayer; motionPlayer = motionPlayer->next) {
      if(motionPlayer->active) {
         glPushMatrix();
         b = motionPlayer->mc.getBoneCtrlList();
         glTranslatef(pos.x() + 7.0f + dest, m_pmd->getMaxHeight() + 1.0f - dest, pos.z() + dest);
         glScalef(0.2f, 0.2f, 0.2f);
         m_pmd->resetBone();
         /* temporary apply bone positions / rotations of this motion to bones */
         for (i = 0; i < motionPlayer->mc.getNumBoneCtrl(); i++) {
            if (motionPlayer->ignoreStatic == true && b[i].motion->numKeyFrame <= 1)
               continue;
            b[i].bone->setCurrentPosition(&(b[i].pos));
            b[i].bone->setCurrentRotation(&(b[i].rot));
         }
         /* update bone position */
         m_pmd->updateBone(false);
         m_pmd->updateBone(true);
         /* draw bones */
         for (i = 0; i < motionPlayer->mc.getNumBoneCtrl(); i++) {
            if (b[i].bone->hasMotionIndependency() == true || b[i].bone->isSimulated() || b[i].bone->getType() == NO_DISP)
               continue;
            /* do not draw IK target bones if the IK chain is under simulation */
            if (b[i].bone->getType() == IK_TARGET && b[i].bone->getParentBone() && b[i].bone->getParentBone()->isSimulated()) continue;
            if (b[i].bone->getType() == UNDER_ROTATE) {
               /* if target bone is also controlled in this motion, draw it */
               for (j = 0; j < motionPlayer->mc.getNumBoneCtrl(); j++) {
                  if (motionPlayer->ignoreStatic == true && b[j].motion->numKeyFrame <= 1)
                     continue;
                  if (b[i].bone->getTargetBone() == b[j].bone)
                     break;
               }
               if (j >= motionPlayer->mc.getNumBoneCtrl())
                  continue;
            }
            /* draw bone */
            x = b[i].bone->getTransform()->getOrigin();
            y = b[i].bone->getParentBone()->getTransform()->getOrigin();
            if (motionPlayer->ignoreStatic == true && b[i].motion->numKeyFrame <= 1) {
               glColor4f(0.4f, 0.4f, 0.4f, 1.0f);
            } else if (b[i].bone == m_pmd->getCenterBone()) {
               glColor4f(1.0f, 0.4f, 0.4f, 1.0f);
            } else if (b[i].bone->getType() == IK_DESTINATION) {
               glColor4f(1.0f, 0.2f, 0.2f, 1.0f);
            } else {
               glColor4f(1.0f, 0.0f, 1.0f, 1.0f);
            }
            v[0] = x.x();
            v[1] = x.y();
            v[2] = x.z();
            v[3] = y.x();
            v[4] = y.y();
            v[5] = y.z();
            glVertexPointer(3, GL_FLOAT, 0, v);
            glDrawArrays(GL_LINES, 0, 2);
            if (b[i].bone->getType() == IK_DESTINATION) {
               /* when an IK is controlled in this motion, the releavant bones under the IK should also be drawn */
               for (j = 0; j < motionPlayer->mc.getNumBoneCtrl(); j++) {
                  if (motionPlayer->ignoreStatic == true && b[j].motion->numKeyFrame <= 1)
                     continue;
                  if (b[j].bone->hasMotionIndependency() == true || b[j].bone->isSimulated())
                     continue;
                  if (b[j].bone->getType() == UNDER_IK && b[j].bone->getTargetBone() == b[i].bone) {
                     x = b[j].bone->getTransform()->getOrigin();
                     y = b[j].bone->getParentBone()->getTransform()->getOrigin();
                     glColor4f(1.0f, 0.0f, 1.0f, 1.0f);
                     v[0] = x.x();
                     v[1] = x.y();
                     v[2] = x.z();
                     v[3] = y.x();
                     v[4] = y.y();
                     v[5] = y.z();
                     glVertexPointer(3, GL_FLOAT, 0, v);
                     glDrawArrays(GL_LINES, 0, 2);
                  }
               }
            }
         }
         /* draw motion progress square */
         if (motionPlayer->onEnd == 2) {
            /* once */
            float rate = (float)motionPlayer->mc.getCurrentFrame() / motionPlayer->mc.getMaxFrame();
            vertices0[7] = vertices0[10] = (1.0f - rate) * 20.0f;
            vertices1[1] = vertices1[4] = vertices1[7] = vertices1[10] = (1.0f - rate) * 20.0f;
            glDisable(GL_CULL_FACE);
            glColor4f(0.4f, 0.8f, 0.0f, 0.4f);
            glVertexPointer(3, GL_FLOAT, 0, vertices0);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glVertexPointer(3, GL_FLOAT, 0, vertices1);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glEnable(GL_CULL_FACE);
         }
         glPopMatrix();
         dest += 0.7f;
      }
   }
   glDisableClientState(GL_VERTEX_ARRAY);

   /* restore the bone positions */
   updateMotion(0.0);

   glEnable(GL_LIGHTING);

   glPopMatrix();
}

/* PMDObject::setControl: set bone/face control */
bool PMDObject::setBoneFaceControl(BoneFaceControl *control)
{
   int i;

   if (m_isEnable == false)
      return false;

   if (control == NULL)
      return true;

   for (i = 0; i < PMDOBJECT_MAXNUMBIND; i++)
      if (m_boneFaceControl[i] == NULL)
         break;
   if (i >= PMDOBJECT_MAXNUMBIND) {
      return false;
   }
   m_boneFaceControl[i] = control;
   m_boneFaceControl[i]->setModel(m_pmd);
   m_boneFaceControl[i]->update(0.0);

   return true;
}

/* PMDObject::unsetBoneFaceControl: unset bone/face control */
bool PMDObject::unsetBoneFaceControl(BoneFaceControl *control)
{
   bool found = false;

   if (m_isEnable == false)
      return false;

   if (control == NULL)
      return true;

   for (int i = 0; i < PMDOBJECT_MAXNUMBIND; i++) {
      if (m_boneFaceControl[i] && m_boneFaceControl[i]->match(control)) {
         m_boneFaceControl[i]->~BoneFaceControl();
         MMDFiles_alignedfree(m_boneFaceControl[i]);
         m_boneFaceControl[i] = NULL;
         found = true;
         break;
      }
   }
   return found;
}

/* PMDObject::findBoneFaceControl: find bone/face control */
bool PMDObject::findBoneFaceControl(BoneFaceControl *control)
{
   if (m_isEnable == false)
      return false;

   if (control == NULL)
      return false;

   for (int i = 0; i < PMDOBJECT_MAXNUMBIND; i++) {
      if (m_boneFaceControl[i] && m_boneFaceControl[i]->match(control)) {
         return true;
      }
   }
   return false;
}

/* PMDObject::clearLoadMessages: clear load model messages */
void PMDObject::clearLoadMessages()
{
   for (int i = 0; i < m_loadMessageNum; i++) {
      if (m_loadMessage[i]) {
         free(m_loadMessage[i]);
         m_loadMessage[i] = NULL;
      }
   }
   for (int i = 0; i < m_deleteMessageNum; i++) {
      if (m_deleteMessage[i]) {
         free(m_deleteMessage[i]);
         m_deleteMessage[i] = NULL;
      }
   }
   m_loadMessageNum = 0;
   m_deleteMessageNum = 0;
}

/* PMDObject::loadModelMessagesFromFile: load model messages from file */
void PMDObject::loadModelMessagesFromFile(const char *modelFileName)
{
   char *fileName;
   ZFile *zf;
   char buf[MMDAGENT_MAXBUFLEN];

   if (modelFileName == NULL)
      return;

   fileName = (char *)malloc(MMDAgent_strlen(modelFileName) + 13);
   strcpy(fileName, modelFileName);
   strcat(fileName, ".loadmessage");
   if (MMDAgent_exist(fileName)) {
      zf = new ZFile(g_enckey);
      if (zf->openAndLoad(fileName)) {
         while (zf->gets(buf, MMDAGENT_MAXBUFLEN) != NULL) {
            if (m_loadMessageNum < PMDOBJECT_MAXNUMMESSAGE)
               m_loadMessage[m_loadMessageNum++] = MMDAgent_strdup(buf);
         }
      }
      delete zf;
   }
   free(fileName);

   fileName = (char *)malloc(MMDAgent_strlen(modelFileName) + 15);
   strcpy(fileName, modelFileName);
   strcat(fileName, ".deletemessage");
   if (MMDAgent_exist(fileName)) {
      zf = new ZFile(g_enckey);
      if (zf->openAndLoad(fileName)) {
         while (zf->gets(buf, MMDAGENT_MAXBUFLEN) != NULL) {
            if (m_deleteMessageNum < PMDOBJECT_MAXNUMMESSAGE)
               m_deleteMessage[m_deleteMessageNum++] = MMDAgent_strdup(buf);
         }
      }
      delete zf;
   }
   free(fileName);
}

/* PMDObject::getLoadMessages: get list of messages for loading */
const char **PMDObject::getLoadMessages()
{
   if (m_loadMessageNum == 0)
      return NULL;
   return (const char **)m_loadMessage;
}

/* PMDObject::getLoadMessagesNum: get number of messages for loading */
int PMDObject::getLoadMessagesNum()
{
   return m_loadMessageNum;
}

/* PMDObject::getDeleteMessages: get list of messages for deletion */
const char **PMDObject::getDeleteMessages()
{
   if (m_deleteMessageNum == 0)
      return NULL;
   return (const char **)m_deleteMessage;
}

/* PMDObject::getDeleteMessages: get number of messages for deletion */
int PMDObject::getDeleteMessagesNum()
{
   return m_deleteMessageNum;
}

/* PMDObject::startCapture: start motion capture */
bool PMDObject::startCapture(const char *saveFileName, unsigned int maxMinutes)
{
   if (m_motionCaptureSaveFileName) /* already running */
      return false;

   /* store save filename */
   m_motionCaptureSaveFileName = MMDAgent_strdup(saveFileName);

   /* prepare data storage with initial size */
   if (m_motionCaptureDataStorage[0]) {
      free(m_motionCaptureDataStorage[0]);
   }
   if (m_motionCaptureDataStorage[1]) {
      free(m_motionCaptureDataStorage[1]);
   }
   m_motionCaptureDataAllocStep[0] = m_pmd->getNumBone() * PMDOBJECT_MOTIONCAPTURE_FRAMESTEP;
   m_motionCaptureDataAllocStep[1] = m_pmd->getNumFace() * PMDOBJECT_MOTIONCAPTURE_FRAMESTEP;
   m_motionCaptureDataAllocated[0] = m_motionCaptureDataAllocStep[0];
   m_motionCaptureDataAllocated[1] = m_motionCaptureDataAllocStep[1];
   m_motionCaptureDataStorage[0] = (unsigned char *)malloc(sizeof(VMDFile_BoneFrame) * m_motionCaptureDataAllocated[0]);
   m_motionCaptureDataStorage[1] = (unsigned char *)malloc(sizeof(VMDFile_FaceFrame) * m_motionCaptureDataAllocated[1]);

   /* prepare current data pointer */
   m_motionCaptureDataP[0] = m_motionCaptureDataStorage[0];
   m_motionCaptureDataP[1] = m_motionCaptureDataStorage[1];

   /* reset bone/face save cache */
   m_pmd->saveAsVmdKeyFrame(NULL, NULL, 0, NULL, NULL);

   /* reset counters */
   m_motionCaptureDataTotalNum[0] = m_motionCaptureDataTotalNum[1] = 0;
   m_motionCaptureLastFrame = 0;
   m_motionCaptureRestFrame = 0.0f;
   m_motionCaptureFirstFrame = true;
   m_motionCaptureMaxCaptureFrame = 1800 * (maxMinutes == 0 ? PMDOBJECT_MAXCAPTUREDEFAULT : maxMinutes);

   return true;
}

/* PMDObject::doCapture: do motion capture */
bool PMDObject::doCapture(double ellapsedFrame)
{
   double f;
   unsigned int n;
   unsigned int num1, num2;

   if (m_motionCaptureSaveFileName == NULL) /* not running */
      return true;

   f = m_motionCaptureRestFrame + ellapsedFrame;
   n = 0;
   while (f >= 1.0f) {
      f -= 1.0f;
      n++;
   }
   m_motionCaptureRestFrame = f;
   if (n == 0)
      return true;

   /* save the current pos/rot/rate as "n" frame ahead the last saved keys */
   if (m_motionCaptureFirstFrame) {
      m_motionCaptureFirstFrame = false;
      m_motionCaptureLastFrame = 0;
   } else {
      m_motionCaptureLastFrame += n;
   }

   if (m_motionCaptureLastFrame >= m_motionCaptureMaxCaptureFrame) {
      /* reaches maximum length */
      return false;
   }

   /* expand work area if needed */
   if (m_motionCaptureDataTotalNum[0] + m_pmd->getNumBone() >= m_motionCaptureDataAllocated[0]) {
      /* store current position */
      unsigned int len = m_motionCaptureDataP[0] - m_motionCaptureDataStorage[0];
      /* increment allocate step */
      m_motionCaptureDataAllocStep[0] *= 2;
      /* re-allocate */
      m_motionCaptureDataAllocated[0] += m_motionCaptureDataAllocStep[0];
      m_motionCaptureDataStorage[0] = (unsigned char *)realloc(m_motionCaptureDataStorage[0], sizeof(VMDFile_BoneFrame) * m_motionCaptureDataAllocated[0]);
      /* reset pointer */
      m_motionCaptureDataP[0] = m_motionCaptureDataStorage[0] + len;
   }
   if (m_motionCaptureDataTotalNum[1] + m_pmd->getNumFace() >= m_motionCaptureDataAllocated[1]) {
      unsigned int len = m_motionCaptureDataP[1] - m_motionCaptureDataStorage[1];
      m_motionCaptureDataAllocStep[1] *= 2;
      m_motionCaptureDataAllocated[1] += m_motionCaptureDataAllocStep[1];
      m_motionCaptureDataStorage[1] = (unsigned char *)realloc(m_motionCaptureDataStorage[1], sizeof(VMDFile_FaceFrame) * m_motionCaptureDataAllocated[1]);
      m_motionCaptureDataP[1] = m_motionCaptureDataStorage[1] + len;
   }

   /* save all pos/rot of a model as motion parameters */
   m_pmd->saveAsVmdKeyFrame(&(m_motionCaptureDataP[0]), &(m_motionCaptureDataP[1]), m_motionCaptureLastFrame, &num1, &num2);
   m_motionCaptureDataTotalNum[0] += num1;
   m_motionCaptureDataTotalNum[1] += num2;

   return true;
}

/* PMDObject::stopCapture: stop motion capture */
bool PMDObject::stopCapture()
{
   FILE *fp;
   VMDFile_Header header;
   char *sjisBuff;

   if (m_motionCaptureSaveFileName == NULL) /* not running */
      return true;

   /* save data to file */
   fp = MMDFiles_fopen(m_motionCaptureSaveFileName, "wb");
   free(m_motionCaptureSaveFileName);
   m_motionCaptureSaveFileName = NULL;
   if (fp == NULL) {
      free(m_motionCaptureDataStorage[0]);
      free(m_motionCaptureDataStorage[1]);
      m_motionCaptureDataStorage[0] = m_motionCaptureDataStorage[1] = NULL;
      return false;
   }

   /* save header */
   memset(&header, 0, sizeof(VMDFile_Header));
   strncpy(header.header, "Vocaloid Motion Data 0002", 30);
   sjisBuff = MMDFiles_strdup_from_utf8_to_sjis(m_pmd->getName());
   strncpy(header.name, sjisBuff, 20);
   free(sjisBuff);
   if (fwrite(&header, sizeof(VMDFile_Header), 1, fp) != 1) {
      fclose(fp);
      free(m_motionCaptureDataStorage[0]);
      free(m_motionCaptureDataStorage[1]);
      m_motionCaptureDataStorage[0] = m_motionCaptureDataStorage[1] = NULL;
      return false;
   }
   /* save bone frames */
   if (fwrite(&(m_motionCaptureDataTotalNum[0]), sizeof(unsigned int), 1, fp) != 1) {
      fclose(fp);
      free(m_motionCaptureDataStorage[0]);
      free(m_motionCaptureDataStorage[1]);
      m_motionCaptureDataStorage[0] = m_motionCaptureDataStorage[1] = NULL;
      return false;
   }
   if (fwrite(m_motionCaptureDataStorage[0], sizeof(VMDFile_BoneFrame), m_motionCaptureDataTotalNum[0], fp) != m_motionCaptureDataTotalNum[0]) {
      fclose(fp);
      free(m_motionCaptureDataStorage[0]);
      free(m_motionCaptureDataStorage[1]);
      m_motionCaptureDataStorage[0] = m_motionCaptureDataStorage[1] = NULL;
      return false;
   }
   /* save face frames */
   if (fwrite(&(m_motionCaptureDataTotalNum[1]), sizeof(unsigned int), 1, fp) != 1) {
      fclose(fp);
      free(m_motionCaptureDataStorage[0]);
      free(m_motionCaptureDataStorage[1]);
      m_motionCaptureDataStorage[0] = m_motionCaptureDataStorage[1] = NULL;
      return false;
   }
   if (fwrite(m_motionCaptureDataStorage[1], sizeof(VMDFile_FaceFrame), m_motionCaptureDataTotalNum[1], fp) != m_motionCaptureDataTotalNum[1]) {
      fclose(fp);
      free(m_motionCaptureDataStorage[0]);
      free(m_motionCaptureDataStorage[1]);
      m_motionCaptureDataStorage[0] = m_motionCaptureDataStorage[1] = NULL;
      return false;
   }
   fclose(fp);

   /* clear work area */
   free(m_motionCaptureDataStorage[0]);
   free(m_motionCaptureDataStorage[1]);
   m_motionCaptureDataStorage[0] = m_motionCaptureDataStorage[1] = NULL;

   return true;
}

/* PMDObject::getShapeMap: get shape map */
ShapeMap *PMDObject::getShapeMap()
{
   return m_shapeMap;
}

/* PMDObject::isShapeMapDefault: return true when using system default shape map */
bool PMDObject::isShapeMapDefault()
{
   return m_isShapeMapDefault;
}

/* PMDObject::setLoadingProgressRate: set loading progress rate */
void PMDObject::setLoadingProgressRate(float value)
{
   m_loadingProgressRate = value;
}

/* PMDObject::deleteModel: delete model */
void PMDObject::deleteModel()
{
   if (m_pmd) {
      m_pmd->~PMDModel();
      MMDFiles_alignedfree(m_pmd);
   }
   m_pmd = NULL;
}

/* PMDObject::lock: mutex lock */
void PMDObject::lock()
{
   glfwLockMutex(m_mutex);
}

/* PMDObject::unlock: mutex unlock */
void PMDObject::unlock()
{
   glfwUnlockMutex(m_mutex);
}
