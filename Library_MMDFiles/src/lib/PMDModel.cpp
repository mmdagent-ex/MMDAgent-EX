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

#include "MMDFiles.h"

/* PMDModel::initialize: initialize PMDModel */
void PMDModel::initialize()
{
   int i;

   m_name = NULL;
   m_comment = NULL;

   m_numVertex = 0;
   m_vertexList = NULL;
   m_normalList = NULL;
   m_texCoordList = NULL;
   m_bone1List = NULL;
   m_bone2List = NULL;
#ifdef MY_EXTRADEFORMATION
   m_bone3List = NULL;
   m_bone4List = NULL;
#endif /* MY_EXTRADEFORMATION */
   m_boneWeight1 = NULL;
#ifdef MY_EXTRADEFORMATION
   m_boneWeight2 = NULL;
   m_boneWeight3 = NULL;
   m_boneWeight4 = NULL;
   m_sdefC = NULL;
   m_sdefR0 = NULL;
   m_sdefR1 = NULL;
   m_sdefCR0 = NULL;
   m_sdefCR1 = NULL;
#endif /* MY_EXTRADEFORMATION */
   m_edgeWidth = NULL;
   m_edgeWidthMorphed = NULL;

   m_numSurface = 0;
   m_surfaceList = NULL;

   m_numMaterial = 0;
   m_material = NULL;

   m_numBone = 0;
   m_boneList = NULL;

   m_numIK = 0;
   m_IKList = NULL;

   m_numFace = 0;
   m_faceList = NULL;

   m_numRigidBody = 0;
   m_rigidBodyList = NULL;

   m_numConstraint = 0;
   m_constraintList = NULL;

   m_numBoneMorph = 0;
   m_boneMorphList = NULL;
   m_numVertexMorph = 0;
   m_vertexMorphList = NULL;
   m_numUVMorph = 0;
   m_uvMorphList = NULL;
   m_baseVertexList = NULL;
   m_baseTexCoordList = NULL;
   m_numMaterialMorph = 0;
   m_materialMorphList = NULL;
   m_numGroupMorph = 0;
   m_groupMorphList = NULL;

   for(i = 0; i < SYSTEMTEXTURE_NUMFILES; i++) {
      m_toonTextureID[i] = 0;
      m_localToonTexture[i].release();
   }

   m_boneSkinningTrans = NULL;
   m_numSurfaceForEdge = 0;
   m_vboMethod = MMDFiles_getVBOMethodDefault();
   m_vboBufData = NULL;
   m_vboBufDataLen = 0;
   m_vboBufDynamic = 0;
   m_vboBufStatic = 0;
   m_vboBufElement = 0;
   m_vboBufDynamicLen = 0;
   m_vboOffsetVertex = 0;
   m_vboOffsetNormal = 0;
   m_vboOffsetToon = 0;
   m_vboOffsetEdge = 0;
   m_vboOffsetSurfaceForEdge = 0;
   m_vboOffsetCoordForShadowMap = 0;
   m_numSurfaceForShadow = 0;
   m_numSurfaceForShadowMap = 0;
   m_vboOffsetSurfaceForShadow = 0;
   m_vboBufElementShadowMap = 0;

   m_centerBone = NULL;
   m_baseFace = NULL;
   m_orderedBoneList = NULL;
   m_hasSingleSphereMap = false;
   m_hasMultipleSphereMap = false;
   m_numRotateBone = 0;
   m_rotateBoneIDList = NULL;
   m_IKSimulated = NULL;
   m_enableSimulation = true;
   m_maxHeight = 0.0f;
   m_boundingSphereStep = PMDMODEL_BOUNDINGSPHEREPOINTSMIN;
   m_maxDistanceFromVertexAssignedBone = NULL;
   m_maxProcessLayer = 0;
   m_materialRenderOrder = NULL;
   m_materialDistance = NULL;

   /* initial values for variables that should be kept at model change */
   m_toon = false;
   m_light.setZero();
   m_lightEdge = true;
   m_globalAlpha = 1.0f;
   m_edgeOffset = 0.03f;
   m_selfShadowDrawing = false;
   m_selfShadowDensityCoef = 0.0f;
   m_edgeColor[0] = PMDMODEL_EDGECOLORR;
   m_edgeColor[1] = PMDMODEL_EDGECOLORG;
   m_edgeColor[2] = PMDMODEL_EDGECOLORB;
   m_edgeColor[3] = PMDMODEL_EDGECOLORA;
   m_forceEdge = false;
   m_hasExtParam = false;
   m_hasExtBoneParam = false;

#ifdef MY_RESETPHYSICS
   m_physicsRestore = false;
#endif
#ifdef MY_LUMINOUS
   m_luminousMode = PMDMODEL_LUMINOUS_NONE;
   m_hasLuminousMaterial = false;
#endif
   m_loadingProgressRate = 0.0f;

   m_bulletPhysics = NULL;
   m_rootBone.reset();

   m_showFlag = true;
}

/* PMDModel::clear: free PMDModel */
void PMDModel::clear()
{
   int i;

   if (m_vertexList)
      MMDFiles_alignedfree(m_vertexList);
   if (m_normalList)
      MMDFiles_alignedfree(m_normalList);
   if (m_texCoordList)
      free(m_texCoordList);
   if (m_bone1List)
      free(m_bone1List);
   if (m_bone2List)
      free(m_bone2List);
#ifdef MY_EXTRADEFORMATION
   if (m_bone3List)
      free(m_bone3List);
   if (m_bone4List)
      free(m_bone4List);
#endif /* MY_EXTRADEFORMATION */
   if (m_boneWeight1)
      free(m_boneWeight1);
#ifdef MY_EXTRADEFORMATION
   if (m_boneWeight2)
      free(m_boneWeight2);
   if (m_boneWeight3)
      free(m_boneWeight3);
   if (m_boneWeight4)
      free(m_boneWeight4);
   if (m_sdefC)
      MMDFiles_alignedfree(m_sdefC);
   if (m_sdefR0)
      MMDFiles_alignedfree(m_sdefR0);
   if (m_sdefR1)
      MMDFiles_alignedfree(m_sdefR1);
   if (m_sdefCR0)
      MMDFiles_alignedfree(m_sdefCR0);
   if (m_sdefCR1)
      MMDFiles_alignedfree(m_sdefCR1);
#endif /* MY_EXTRADEFORMATION */
   if (m_edgeWidth)
      free(m_edgeWidth);
   if (m_edgeWidthMorphed)
      free(m_edgeWidthMorphed);
   if (m_surfaceList)
      free(m_surfaceList);
   if (m_material)
      delete [] m_material;
   if (m_boneList) {
      for (int i = 0; i < m_numBone; i++)
         m_boneList[i].~PMDBone();
      MMDFiles_alignedfree(m_boneList);
   }
   if (m_IKList)
      delete [] m_IKList;
   if (m_faceList)
      delete [] m_faceList;
   if (m_constraintList)
      delete [] m_constraintList;
   if (m_rigidBodyList) {
      for (unsigned int i = 0; i < m_numRigidBody; i++)
         m_rigidBodyList[i].~PMDRigidBody();
      MMDFiles_alignedfree(m_rigidBodyList);
   }
   if (m_boneMorphList)
      delete [] m_boneMorphList;
   if (m_vertexMorphList)
      delete[] m_vertexMorphList;
   if (m_uvMorphList)
      delete[] m_uvMorphList;
   if (m_baseVertexList)
      MMDFiles_alignedfree(m_baseVertexList);
   if (m_baseTexCoordList)
      free(m_baseTexCoordList);
   if (m_materialMorphList)
      delete[] m_materialMorphList;
   if (m_groupMorphList)
      delete[] m_groupMorphList;

   if (m_boneSkinningTrans)
      MMDFiles_alignedfree(m_boneSkinningTrans);
   if (m_vboBufData)
      MMDFiles_alignedfree(m_vboBufData);
   if (m_vboBufDynamic != 0)
      glDeleteBuffers(1, &m_vboBufDynamic);
   if (m_vboBufStatic != 0)
      glDeleteBuffers(1, &m_vboBufStatic);
   if (m_vboBufElement != 0)
      glDeleteBuffers(1, &m_vboBufElement);
   if (m_vboBufElementShadowMap != 0)
      glDeleteBuffers(1, &m_vboBufElementShadowMap);
   if (m_orderedBoneList)
      free(m_orderedBoneList);
   if (m_rotateBoneIDList)
      free(m_rotateBoneIDList);
   if (m_IKSimulated)
      free(m_IKSimulated);
   if(m_comment)
      free(m_comment);
   if(m_name)
      free(m_name);

   if (m_maxDistanceFromVertexAssignedBone)
      free(m_maxDistanceFromVertexAssignedBone);
   if (m_materialRenderOrder)
      free(m_materialRenderOrder);
   if (m_materialDistance)
      free(m_materialDistance);

   for (i = 0; i < SYSTEMTEXTURE_NUMFILES; i++)
      m_localToonTexture[i].release();
   m_name2bone.release();
   m_name2face.release();
   m_name2bonemorph.release();
   m_name2vertexmorph.release();
   m_name2uvmorph.release();
   m_name2materialmorph.release();
   m_name2groupmorph.release();

   initialize();
}

/* PMDModel::PMDModel: constructor */
PMDModel::PMDModel()
{
   initialize();
}

/* PMDModel::~PMDModel: destructor */
PMDModel::~PMDModel()
{
   clear();
}

/* PMDModel::read: read from file */
bool PMDModel::read(const char *file, BulletPhysics *bullet, SystemTexture *systex)
{
   int len;
   ZFile *zf;
   char *dir;
   char *extname;
   bool loadTeX;
   bool ret;

   if (bullet == NULL || systex == NULL)
      return false;
   len = MMDFiles_strlen(file);
   if (len <= 0)
      return false;

   /* get model directory */
   dir = MMDFiles_dirname(file);

   /* open file */
   zf = new ZFile(g_enckey);
   if (zf->openAndLoad(file) == false) {
      delete zf;
      free(dir);
      return false;
   }

   /* check if EXT information exists */
   extname = (char *)malloc(MMDFiles_strlen(file) + 5);
   strcpy(extname, file);
   strcat(extname, ".csv");
   if (MMDFiles_exist(extname)) {
      /* if EXT exist, skip loading texture in first parse */
      loadTeX = false;
   } else {
      loadTeX = true;
   }

   /* initialize loading rate */
   m_loadingProgressRate = 0.0f;

   /* pre-fetch common texture loading work area */
   m_textureLoader.allocateTextureWorkArea(4096 * 4096 * 4);

   /* initialize and load from the data memories */
   ret = parse(zf->getData(), (unsigned long)zf->getSize(), bullet, systex, dir, loadTeX);

   /* release memory for reading */
   zf->close();

   if (ret == false) {
      m_textureLoader.freeTextureWorkArea();
      free(dir);
      return ret;
   }

   /* read EXT information if exist */
   if (loadTeX == false) {
      m_loadingProgressRate = 0.1f;
      if (parseExtCsv(extname, dir) == false) {
         m_textureLoader.freeTextureWorkArea();
         free(extname);
         return false;
      }
   }

   free(extname);
   free(dir);

   /* do model structure setup */
   ret = setupModel();
   if (ret == false) {
      m_textureLoader.freeTextureWorkArea();
      return false;
   }

   m_loadingProgressRate = 0.9f;

   m_textureLoader.freeTextureWorkArea();


   return true;
}

/* PMDModel::load: load from file name */
bool PMDModel::load(const char *file, BulletPhysics *bullet, SystemTexture *systex)
{
   int ret;

   /* read */
   ret = read(file, bullet, systex);
   if (ret == false)
      return false;

   /* do OpenGL objects setup */
   setupObjects();

   return ret;
}

/* PMDModel::getBone: find bone data by name */
PMDBone *PMDModel::getBone(const char *name)
{
   PMDBone *match;

   if (name == NULL)
      return NULL;

   if (m_name2bone.search(name, strlen(name), (void **)&match) == true)
      return match;
   else
      return NULL;
}

/* PMDModel::getFace: find face data by name */
PMDFace *PMDModel::getFace(const char *name)
{
   PMDFace *match;

   if (name == NULL)
      return NULL;

   if (m_name2face.search(name, strlen(name), (void **)&match) == true)
      return match;
   else
      return NULL;
}

/* PMDModel::getBoneMorph: find bone Morph data by name */
PMDBoneMorph *PMDModel::getBoneMorph(const char *name)
{
   PMDBoneMorph *match;

   if (name == NULL)
      return NULL;

   if (m_name2bonemorph.search(name, strlen(name), (void **)&match) == true)
      return match;
   else
      return NULL;

}

/* PMDModel::getVertexMorph: find vertex Morph data by name */
PMDVertexMorph *PMDModel::getVertexMorph(const char *name)
{
   PMDVertexMorph *match;

   if (name == NULL)
      return NULL;

   if (m_name2vertexmorph.search(name, strlen(name), (void **)&match) == true)
      return match;
   else
      return NULL;
}

/* PMDModel::getUVMorph: find uv morph data by name */
PMDUVMorph *PMDModel::getUVMorph(const char *name)
{
   PMDUVMorph *match;

   if (name == NULL)
      return NULL;

   if (m_name2uvmorph.search(name, strlen(name), (void **)&match) == true)
      return match;
   else
      return NULL;
}

/* PMDModel::getMaterialMorph: find material morph data by name */
PMDMaterialMorph *PMDModel::getMaterialMorph(const char *name)
{
   PMDMaterialMorph *match;

   if (name == NULL)
      return NULL;

   if (m_name2materialmorph.search(name, strlen(name), (void **)&match) == true)
      return match;
   else
      return NULL;
}

/* PMDModel::getGroupMorph: find group morph data by name */
PMDGroupMorph *PMDModel::getGroupMorph(const char *name)
{
   PMDGroupMorph *match;

   if (name == NULL)
      return NULL;

   if (m_name2groupmorph.search(name, strlen(name), (void **)&match) == true)
      return match;
   else
      return NULL;
}

/* PMDModel::getChildBoneList: return list of child bones, in decent order */
int PMDModel::getChildBoneList(PMDBone **bone, unsigned short numBone, PMDBone **childBoneList)
{
   int i, j, k;
   PMDBone *b1, *b2;
   bool find;
   int n = 0;

   for(i = 0; i < numBone; i++) {
      b1 = bone[i];
      for(j = 0; j < m_numBone; j++) {
         b2 = &(m_boneList[j]);
         if(b2->getParentBone() == b1 || (b2->getType() == UNDER_ROTATE && b2->getTargetBone() == b1) || (b2->getType() == FOLLOW_ROTATE && b2->getChildBone() == b1) || (b2->getType() == FOLLOW_MOVE && b2->getChildBone() == b1))
            childBoneList[n++] = b2;
      }
   }

   for(i = 0; i < n; i++) {
      b1 = childBoneList[i];
      for(j = 0; j < m_numBone; j++) {
         b2 = &(m_boneList[j]);
         if (b2->getParentBone() == b1 || (b2->getType() == UNDER_ROTATE && b2->getTargetBone() == b1) || (b2->getType() == FOLLOW_ROTATE && b2->getChildBone() == b1) || (b2->getType() == FOLLOW_MOVE && b2->getChildBone() == b1)) {
            /* check which child is already found */
            find = false;
            for(k = 0; k < n; k++) {
               if(childBoneList[k] == b2) {
                  find = true;
                  break;
               }
            }
            /* add */
            if(find == false)
               childBoneList[n++] = b2;
         }
      }
   }

   return n;
}

/* PMDModel::setPhysicsControl switch bone control by physics simulation */
#ifdef MY_RESETPHYSICS
void PMDModel::setPhysicsControl(bool flag, bool keepStatus)
#else
void PMDModel::setPhysicsControl(bool flag)
#endif
{
   unsigned long i;
   unsigned short j;

   if(flag == m_enableSimulation)
      return;

   m_enableSimulation = flag;

#ifdef MY_RESETPHYSICS
   if (flag == false)
      m_physicsRestore = keepStatus;
#endif

   /* when true, align all rigid bodies to corresponding bone by setting Kinematics flag */
   /* when false, all rigid bodies will have their own motion states according to the model definition */
   for (i = 0; i < m_numRigidBody; i++)
#ifdef MY_RESETPHYSICS
      m_rigidBodyList[i].setKinematic(!flag, m_physicsRestore);
#else
      m_rigidBodyList[i].setKinematic(!flag);
#endif

   if (flag == false) {
      /* save the current bone transform with no physics as a start transform for later resuming */
      updateBone(false);
      updateBone(true);
      m_rootBone.saveTrans();
      for (j = 0; j < m_numBone; j++)
         m_boneList[j].saveTrans();
   }
}

/* PMDModel::release: free PMDModel */
void PMDModel::release()
{
   clear();
}

/* PMDModel::setShowFlag: set show flag */
void PMDModel::setShowFlag(bool flag)
{
   m_showFlag = flag;
}

/* PMDModel:;setEdgeThin: set edge offset */
void PMDModel::setEdgeThin(float thin)
{
   /* also update edge width for each vertex */
   float newedge, rate;
   unsigned int i;

   newedge = thin * 0.03f;

   /* update vertex edge width */
   rate = newedge / m_edgeOffset;
   for (i = 0; i < m_numVertex; i++)
      m_edgeWidth[i] *= rate;

   /* set new value */
   m_edgeOffset = newedge;
}

/* PMDModel:;setToonFlag: set toon rendering flag */
void PMDModel::setToonFlag(bool flag)
{
   m_toon = flag;
}

/* PMDModel::getToonFlag: return true when enable toon rendering */
bool PMDModel::getToonFlag()
{
   return m_toon;
}

/* PMDModel::setSelfShadowDrawing: set self shadow drawing flag */
void PMDModel::setSelfShadowDrawing(bool flag)
{
   m_selfShadowDrawing = flag;
}

/* PMDModel::setEdgeColor: set edge color */
void PMDModel::setEdgeColor(float *color)
{
   int i;

   if(color == NULL)
      return;
   for(i = 0; i < 4; i++)
      m_edgeColor[i] = color[i];
}

/* PMDModel::setGlobalAlpha: set global alpha value */
void PMDModel::setGlobalAlpha(float alpha)
{
   m_globalAlpha = alpha;
}

/* PMDModel::getRootBone: get root bone */
PMDBone *PMDModel::getRootBone()
{
   return &m_rootBone;
}

/* PMDModel::getCenterBone: get center bone */
PMDBone *PMDModel::getCenterBone()
{
   return m_centerBone;
}

/* PMDModel::getName: get name */
char *PMDModel::getName()
{
   return m_name;
}

/* PMDModel::getNumVertex: get number of vertics */
unsigned int PMDModel::getNumVertex()
{
   return m_numVertex;
}

/* PMDModel::getNumSurface: get number of surface definitions */
unsigned int PMDModel::getNumSurface()
{
   return m_numSurface;
}

/* PMDModel::getNumMaterial: get number of material definitions */
unsigned int PMDModel::getNumMaterial()
{
   return m_numMaterial;
}

/* PMDModel::getNumBone: get number of bones */
unsigned short PMDModel::getNumBone()
{
   return m_numBone;
}

/* PMDModel::getNumIK: get number of IK chains */
unsigned short PMDModel::getNumIK()
{
   return m_numIK;
}

/* PMDModel::getNumFace: get number of faces */
unsigned short PMDModel::getNumFace()
{
   return m_numFace;
}

/* PMDModel::getNumBoneMorph: get number of bone morphs */
unsigned short PMDModel::getNumBoneMorph()
{
   return m_numBoneMorph;
}

/* PMDModel::getNumVertexMorph: get number of vertex morphs */
unsigned short PMDModel::getNumVertexMorph()
{
   return m_numVertexMorph;
}

/* PMDModel::getNumUVMorph: get number of uv morphs */
unsigned short PMDModel::getNumUVMorph()
{
   return m_numUVMorph;
}

/* PMDModel::getNumMaterialMorph: get number of material morphs */
unsigned short PMDModel::getNumMaterialMorph()
{
   return m_numMaterialMorph;
}

/* PMDModel::getNumGroupMorph: get number of group morphs */
unsigned short PMDModel::getNumGroupMorph()
{
   return m_numGroupMorph;
}

/* PMDModel::getNumRigidBody: get number of rigid bodies */
unsigned int PMDModel::getNumRigidBody()
{
   return m_numRigidBody;
}

/* PMDModel::getNumConstraint: get number of constraints */
unsigned int PMDModel::getNumConstraint()
{
   return m_numConstraint;
}

/* PMDModel::getErrorTextureList: get error texture list */
void PMDModel::getErrorTextureList(char *buf, int size)
{
   m_textureLoader.getErrorTextureString(buf, size);
}

/* PMDModel::getMaxHeight: get max height */
float PMDModel::getMaxHeight()
{
   return m_maxHeight;
}

/* PMDModel::getComment: get comment of PMD */
char *PMDModel::getComment()
{
   return m_comment;
}

/* PMDModel::setForceEdgeFlag: set force edge flag */
void PMDModel::setForceEdgeFlag(bool flag)
{
   m_forceEdge = flag;
}

#ifdef MY_LUMINOUS
bool PMDModel::hasLuminousMaterial()
{
   return m_hasLuminousMaterial;
}

void PMDModel::setLuminousMode(int flag)
{
   m_luminousMode = flag;
}
#endif

/* PMDModel::hasExtParam: return if this model has ext param */
bool PMDModel::hasExtParam()
{
   return m_hasExtParam;
}

/* PMDModel::getTextureLoader: get texture loader */
PMDTextureLoader *PMDModel::getTextureLoader()
{
   return &m_textureLoader;
}

/* PMDModel::saveAsVmdKeyFrame: save as vmd key frame */
void PMDModel::saveAsVmdKeyFrame(unsigned char **boneData, unsigned char **faceData, unsigned int keyFrame, unsigned int *numBoneRet, unsigned int *numFaceRet)
{
   int num1 = 0, num2 = 0;
   for (unsigned short i = 0; i < m_numBone; i++) {
      num1 += m_boneList[i].saveAsBoneFrame(boneData, keyFrame);
   }
   for (unsigned short i = 0; i < m_numFace; i++) {
      num2 += m_faceList[i].saveAsFaceFrame(faceData, keyFrame);
   }
   if (numBoneRet)
      *numBoneRet = num1;
   if (numFaceRet)
      *numFaceRet = num2;
}

/* PMDModel::getLoadingProcessRate: get loading process rate */
float PMDModel::getLoadingProcessRate()
{
   return m_loadingProgressRate;
}

/* PMDModel::setLightEdge: set light edge flag */
void PMDModel::setLightEdge(bool flag)
{
   m_lightEdge = flag;
}
