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
#ifdef __ANDROID__
#include "android/log.h"
#endif

#include <omp.h>

/* PMDModel::resetBone: reset bones */
void PMDModel::resetBone()
{
   unsigned short i;
   btVector3 zeroPos(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f));
   btQuaternion zeroRot(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));

   /* set zero position for IK-controlled bones before applying motion */
   for (i = 0; i < m_numBone; i++)
      switch(m_boneList[i].getType()) {
      case UNDER_IK:
      case IK_TARGET:
         m_boneList[i].setCurrentPosition(&zeroPos);
         m_boneList[i].setCurrentRotation(&zeroRot);
         break;
      }
}

/* PMDModel::updateBone: update bones */
void PMDModel::updateBone(bool afterPhysics)
{
   unsigned short i;
   short layer;

   if (m_hasExtBoneParam == false) {

      /* PMD style processing: all is before-physics, no layer, re-oder considering parent, resolve in the order of bones, IKs, under-rotate bones */
      if (afterPhysics == true)
         return;

      /* 1. update bone matrix from current position and rotation */
      for (i = 0; i < m_numBone; i++)
         m_orderedBoneList[i]->update();

      /* 2. solve IK chains */
      if (m_enableSimulation) {
         /* IK with simulated bones can be skipped */
         for (i = 0; i < m_numIK; i++) {
            if (!m_IKSimulated[i]) m_IKList[i].solve();
         }
      } else {
         /* all IK should be solved when simulation is off */
         for (i = 0; i < m_numIK; i++)
            m_IKList[i].solve();
      }

      /* 3. apply under-rotate effects */
      for (i = 0; i < m_numRotateBone; i++)
         m_boneList[m_rotateBoneIDList[i]].update();

   } else {

      /* extended style processing: before/after-physics, layered, no special re-order, keep index, bone and IK mixed */

      PMDIK *ik;

      for (layer = 0; layer <= m_maxProcessLayer; layer++) {
         for (i = 0; i < m_numBone; i++) {
            if (m_boneList[i].processAfterPhysics() != afterPhysics) continue;
            if (m_boneList[i].getProcessLayer() != layer) continue;
            if (afterPhysics == true)
               m_boneList[i].updateAfterSimulation();
            else
               m_boneList[i].update();
            ik = m_boneList[i].getIK();
            if (ik) {
               /* extended style seems to require updates of link bones at each IK call */
               ik->updateLinkBones();
               ik->solve();
            }
         }
         /* additionally apply follow-rotate bone with IK target finally */
         for (i = 0; i < m_numBone; i++) {
            if (m_boneList[i].processAfterPhysics() != afterPhysics) continue;
            if (m_boneList[i].getProcessLayer() != layer) continue;
            if (m_boneList[i].getType() == FOLLOW_ROTATE) {
               if (afterPhysics == true)
                  m_boneList[i].updateAfterSimulation();
               else
                  m_boneList[i].update();
            }
         }
      }
   }
}

/* PMDModel::updateBoneFromSimulation: update bone transform from rigid body */
void PMDModel::updateBoneFromSimulation()
{
   unsigned int i;
   short j;

   for (j = 0; j < m_numBone; j++)
      m_boneList[j].clearTransBySimulationFlag();

   for (i = 0; i < m_numRigidBody; i++)
      m_rigidBodyList[i].applyTransformToBone();
}

/* PMDModel::updateFace: update face morph from current face weights */
void PMDModel::updateFace()
{
   unsigned short i;
   unsigned int j;
   bool updated;

   if (m_vertexMorphList)
      memcpy(m_vertexList, m_baseVertexList, sizeof(btVector3) * m_numVertex);
   if (m_faceList) {
      if (m_vertexMorphList == NULL)
         m_baseFace->apply(m_vertexList);
      for (i = 0; i < m_numFace; i++)
         if (m_faceList[i].getWeight() > PMDMODEL_MINFACEWEIGHT)
            m_faceList[i].add(m_vertexList, m_faceList[i].getWeight());
   }
   if (m_vertexMorphList) {
      for (i = 0; i < m_numVertexMorph; i++)
         if (m_vertexMorphList[i].getWeight() > PMDMODEL_MINFACEWEIGHT)
            m_vertexMorphList[i].apply(m_vertexList);
   }
   if (m_groupMorphList) {
      PMDGroupMorphElem *p;
      float r;
      float w;
      for (i = 0; i < m_numGroupMorph; i++) {
         for (p = m_groupMorphList[i].getList(); p; p = p->next) {
            if (p->v) {
               r = m_groupMorphList[i].getWeight() * p->rate;
               if (r > PMDMODEL_MINFACEWEIGHT) {
                  w = p->v->getWeight();
                  p->v->setWeight(r);
                  p->v->apply(m_vertexList);
                  p->v->setWeight(w);
               }
            }
         }
      }
   }
   if (m_uvMorphList) {
      updated = false;
      for (i = 0; i < m_numUVMorph; i++) {
         if (m_uvMorphList[i].modified()) {
            updated = true;
            break;
         }
      }
      if (updated == false && m_groupMorphList) {
         for (i = 0; i < m_numGroupMorph; i++) {
            if (m_groupMorphList[i].getHasUVFlag() == true && m_groupMorphList[i].modified()) {
               updated = true;
               break;
            }
         }
      }
      if (updated) {
         TexCoord *ptr = m_texCoordList;
         memcpy(ptr, m_baseTexCoordList, sizeof(TexCoord) * m_numVertex);
         for (i = 0; i < m_numUVMorph; i++) {
            if (m_uvMorphList[i].getWeight() != 0.0f)
               m_uvMorphList[i].apply(ptr);
            m_uvMorphList[i].resetModifiedFlag();
         }
         if (m_groupMorphList) {
            for (i = 0; i < m_numGroupMorph; i++) {
               if (m_groupMorphList[i].getHasUVFlag() == true) {
                  PMDGroupMorphElem *p;
                  for (p = m_groupMorphList[i].getList(); p; p = p->next) {
                     if (p->u) {
                        float w = p->u->getWeight();
                        p->u->setWeight(m_groupMorphList[i].getWeight() * p->rate);
                        p->u->apply(ptr);
                        p->u->setWeight(w);
                        p->u->resetModifiedFlag();
                     }
                  }
               }
               m_groupMorphList[i].resetModifiedFlag();
            }
         }
         glBindBuffer(GL_ARRAY_BUFFER, m_vboBufStatic);
         glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoord) * m_numVertex, NULL, GL_DYNAMIC_DRAW);
         glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(TexCoord) * m_numVertex, ptr);
         glBindBuffer(GL_ARRAY_BUFFER, 0);
      }
   }
   if (m_materialMorphList) {
      if (m_edgeWidthMorphed == NULL)
         m_edgeWidthMorphed = (float *)malloc(sizeof(float) * m_numVertex);
      memcpy(m_edgeWidthMorphed, m_edgeWidth, sizeof(float) * m_numVertex);
      for (j = 0; j < m_numMaterial; j++)
         m_material[j].resetMorphParam();
      for (i = 0; i < m_numMaterialMorph; i++)
         m_materialMorphList[i].setParam(m_material, m_numMaterial);
      if (m_groupMorphList) {
         for (i = 0; i < m_numGroupMorph; i++) {
            PMDGroupMorphElem *p;
            for (p = m_groupMorphList[i].getList(); p; p = p->next) {
               if (p->m) {
                  float w = p->m->getWeight();
                  p->m->setWeight(m_groupMorphList[i].getWeight() * p->rate);
                  p->m->setParam(m_material, m_numMaterial);
                  p->m->setWeight(w);
               }
            }
         }
      }
      for (j = 0; j < m_numMaterial; j++)
         m_material[j].updateMorphedEdge(m_surfaceList, m_edgeWidth, m_edgeWidthMorphed);
   }
}

/* PMDModel::updateSkin: update skin data from bone orientation, toon and edges */
void PMDModel::updateSkin()
{
   unsigned short i;
   int j, numVertex;
   btVector3 *vertexList, *normalList, *edgeVertexList = NULL;
   TexCoord *texCoordList = NULL;
   char *ptr;

   /* calculate transform matrix for skinning (global -> local) */
   for (i = 0; i < m_numBone; i++)
      m_boneList[i].calcSkinningTrans(&(m_boneSkinningTrans[i]));

   /* prepare VBO buffer */
   glBindBuffer(GL_ARRAY_BUFFER, m_vboBufDynamic);

   if (m_vboMethod == PMDMODEL_VBO_AUTO) {

      // auto-detect now
      ptr = NULL;
      if (ptr == NULL) {
         /* test 1: glMapBufferRange with orphaning */
         ptr = (char *)glMapBufferRange(GL_ARRAY_BUFFER, 0, m_vboBufDynamicLen, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
         if (ptr) {
#ifdef __ANDROID__
            __android_log_print(ANDROID_LOG_WARN, "MMDAgent", "PMDModel_update: glMapBufferRange chosen");
#endif
            m_vboMethod = PMDMODEL_VBO_MAPBUFFERRANGE;
         }
      }
      if (ptr == NULL) {
         /* test 2: glMapBuffer with orphaning */
         glBufferData(GL_ARRAY_BUFFER, m_vboBufDynamicLen, NULL, GL_DYNAMIC_DRAW);
         ptr = (char *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
         if (ptr) {
#ifdef __ANDROID__
            __android_log_print(ANDROID_LOG_WARN, "MMDAgent", "PMDModel_update: glMapBuffer chosen");
#endif
            m_vboMethod = PMDMODEL_VBO_MAPBUFFER;
         }
      }
      if (ptr == NULL) {
         /* test 3: glBuffer*Data with orphaning */
         glBufferData(GL_ARRAY_BUFFER, m_vboBufDynamicLen, NULL, GL_DYNAMIC_DRAW);
         if (m_vboBufData != NULL && m_vboBufDataLen < m_vboBufDynamicLen) {
            MMDFiles_alignedfree(m_vboBufData);
            m_vboBufData = NULL;
         }
         if (m_vboBufData == NULL) {
            m_vboBufData = (char *)MMDFiles_alignedmalloc(m_vboBufDynamicLen, 16);
            m_vboBufDataLen = m_vboBufDynamicLen;
         }
         ptr = m_vboBufData;
         m_vboMethod = PMDMODEL_VBO_BUFFERDATA;
      }

   } else {

      /* use specified VBO function */

      switch (m_vboMethod) {
      case PMDMODEL_VBO_MAPBUFFERRANGE:
         ptr = (char *)glMapBufferRange(GL_ARRAY_BUFFER, 0, m_vboBufDynamicLen, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
         break;
      case PMDMODEL_VBO_MAPBUFFER:
         glBufferData(GL_ARRAY_BUFFER, m_vboBufDynamicLen, NULL, GL_DYNAMIC_DRAW);
         ptr = (char *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
         break;
      case PMDMODEL_VBO_BUFFERDATA:
         glBufferData(GL_ARRAY_BUFFER, m_vboBufDynamicLen, NULL, GL_DYNAMIC_DRAW);
         if (m_vboBufData != NULL && m_vboBufDataLen < m_vboBufDynamicLen) {
            MMDFiles_alignedfree(m_vboBufData);
            m_vboBufData = NULL;
         }
         if (m_vboBufData == NULL) {
            m_vboBufData = (char *)MMDFiles_alignedmalloc(m_vboBufDynamicLen, 16);
            m_vboBufDataLen = m_vboBufDynamicLen;
         }
         ptr = m_vboBufData;
         break;
      }
   }

   vertexList = (btVector3 *)(ptr + m_vboOffsetVertex);
   normalList = (btVector3 *)(ptr + m_vboOffsetNormal);
   if (m_toon) {
      texCoordList = (TexCoord *)(ptr + m_vboOffsetToon);
      edgeVertexList = (btVector3 *)(ptr + m_vboOffsetEdge);
   }

   numVertex = (int)m_numVertex;

   /* do skinning */
#ifdef MY_EXTRADEFORMATION
   if (m_bone3List != NULL) {
#pragma omp parallel for
      for (j = 0; j < numVertex; j++) {
         btVector3 v, v2, n, n2, vv, nn;
         if (m_bone3List[j] >= 0) {
            /* BDEF4 */
            v = m_boneSkinningTrans[m_bone1List[j]] * m_vertexList[j];
            n = m_boneSkinningTrans[m_bone1List[j]].getBasis() * m_normalList[j];
            v2 = m_boneSkinningTrans[m_bone2List[j]] * m_vertexList[j];
            n2 = m_boneSkinningTrans[m_bone2List[j]].getBasis() * m_normalList[j];
            vv = v * m_boneWeight1[j] + v2 * m_boneWeight2[j];
            nn = n * m_boneWeight1[j] + n2 * m_boneWeight2[j];
            v = m_boneSkinningTrans[m_bone3List[j]] * m_vertexList[j];
            n = m_boneSkinningTrans[m_bone3List[j]].getBasis() * m_normalList[j];
            v2 = m_boneSkinningTrans[m_bone4List[j]] * m_vertexList[j];
            n2 = m_boneSkinningTrans[m_bone4List[j]].getBasis() * m_normalList[j];
            vv += v * m_boneWeight3[j] + v2 * m_boneWeight4[j];
            nn += n * m_boneWeight3[j] + n2 * m_boneWeight4[j];
         } else if (m_bone3List[j] == -2) {
            /* SDEF */
            btTransform t;
            btQuaternion r1, r2;
            r1 = m_boneList[m_bone1List[j]].getTransform()->getRotation();
            r2 = m_boneList[m_bone2List[j]].getTransform()->getRotation();
            if (r2.dot(r1) < 0.0)
               r2 = -r2;
            t.setIdentity();
            t.setRotation(r2.slerp(r1, btScalar(m_boneWeight1[j])));
            v = m_boneSkinningTrans[m_bone1List[j]] * m_sdefCR0[j];
            v2 = m_boneSkinningTrans[m_bone2List[j]] * m_sdefCR1[j];
            vv = v2.lerp(v, btScalar(m_boneWeight1[j]));
            vv = vv + t * (m_vertexList[j] - m_sdefC[j]);
            n = m_boneSkinningTrans[m_bone1List[j]].getBasis() * m_normalList[j];
            n2 = m_boneSkinningTrans[m_bone2List[j]].getBasis() * m_normalList[j];
            nn = n2.lerp(n, btScalar(m_boneWeight1[j]));
         } else {
            /* normal BDEF2 */
            if (m_boneWeight1[j] >= 1.0f - PMDMODEL_MINBONEWEIGHT) {
               /* bone 1 */
               vv = m_boneSkinningTrans[m_bone1List[j]] * m_vertexList[j];
               nn = m_boneSkinningTrans[m_bone1List[j]].getBasis() * m_normalList[j];
            } else if (m_boneWeight1[j] <= PMDMODEL_MINBONEWEIGHT) {
               /* bone 2 */
               vv = m_boneSkinningTrans[m_bone2List[j]] * m_vertexList[j];
               nn = m_boneSkinningTrans[m_bone2List[j]].getBasis() * m_normalList[j];
            } else {
               /* lerp */
               v = m_boneSkinningTrans[m_bone1List[j]] * m_vertexList[j];
               n = m_boneSkinningTrans[m_bone1List[j]].getBasis() * m_normalList[j];
               v2 = m_boneSkinningTrans[m_bone2List[j]] * m_vertexList[j];
               n2 = m_boneSkinningTrans[m_bone2List[j]].getBasis() * m_normalList[j];
               vv = v2.lerp(v, btScalar(m_boneWeight1[j]));
               nn = n2.lerp(n, btScalar(m_boneWeight1[j]));
            }
         }
         vertexList[j] = vv;
         normalList[j] = nn;
         if (m_toon) {
            texCoordList[j].u = 0.0f;
            texCoordList[j].v = (1.0f - m_light.dot(nn)) * 0.5f;
            edgeVertexList[j] = vv + nn * (m_edgeWidthMorphed ? m_edgeWidthMorphed[j] : m_edgeWidth[j]);
         }
      }

   } else {
#endif /* MY_EXTRADEFORMATION */
#pragma omp parallel for
      for (j = 0; j < numVertex; j++) {
         btVector3 v, v2, n, n2, vv, nn;
         if (m_boneWeight1[j] >= 1.0f - PMDMODEL_MINBONEWEIGHT) {
            /* bone 1 */
            vv = m_boneSkinningTrans[m_bone1List[j]] * m_vertexList[j];
            nn = m_boneSkinningTrans[m_bone1List[j]].getBasis() * m_normalList[j];
         } else if (m_boneWeight1[j] <= PMDMODEL_MINBONEWEIGHT) {
            /* bone 2 */
            vv = m_boneSkinningTrans[m_bone2List[j]] * m_vertexList[j];
            nn = m_boneSkinningTrans[m_bone2List[j]].getBasis() * m_normalList[j];
         } else {
            /* lerp */
            v = m_boneSkinningTrans[m_bone1List[j]] * m_vertexList[j];
            n = m_boneSkinningTrans[m_bone1List[j]].getBasis() * m_normalList[j];
            v2 = m_boneSkinningTrans[m_bone2List[j]] * m_vertexList[j];
            n2 = m_boneSkinningTrans[m_bone2List[j]].getBasis() * m_normalList[j];
            vv = v2.lerp(v, btScalar(m_boneWeight1[j]));
            nn = n2.lerp(n, btScalar(m_boneWeight1[j]));
         }
         vertexList[j] = vv;
         normalList[j] = nn;
         if (m_toon) {
            texCoordList[j].u = 0.0f;
            texCoordList[j].v = (1.0f - m_light.dot(nn)) * 0.5f;
            edgeVertexList[j] = vv + nn * (m_edgeWidthMorphed ? m_edgeWidthMorphed[j] : m_edgeWidth[j]);
         }
      }
#ifdef MY_EXTRADEFORMATION
   }
#endif /* MY_EXTRADEFORMATION */

   if (m_vboMethod == PMDMODEL_VBO_MAPBUFFER || m_vboMethod == PMDMODEL_VBO_MAPBUFFERRANGE)
      glUnmapBuffer(GL_ARRAY_BUFFER);
   else
      glBufferSubData(GL_ARRAY_BUFFER, 0, m_vboBufDynamicLen, ptr);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/* PMDModel::setToonLight: set light direction for toon coordinates */
void PMDModel::setToonLight(btVector3 *light)
{
   m_light = *light;
}

/* PMDModel::updateShadowColorTexCoord: update / create pseudo toon coordinates for shadow rendering pass on shadow mapping */
void PMDModel::updateShadowColorTexCoord(float coef)
{
   unsigned int i;
   TexCoord *tmp;

   if (!m_toon) return;

   if (m_vboOffsetCoordForShadowMap == 0 || m_selfShadowDensityCoef != coef) {
      glBindBuffer(GL_ARRAY_BUFFER, m_vboBufStatic);
      if (m_uvMorphList)
         glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoord) * 2 * m_numVertex, NULL, GL_DYNAMIC_DRAW);
      else
         glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoord) * 2 * m_numVertex, NULL, GL_STATIC_DRAW);
      glBufferSubData(GL_ARRAY_BUFFER, (GLintptr) NULL, sizeof(TexCoord) * m_numVertex, m_texCoordList);
      m_vboOffsetCoordForShadowMap = sizeof(TexCoord) * m_numVertex;
      tmp = (TexCoord *) malloc(sizeof(TexCoord) * m_numVertex);
      for (i = 0; i < m_numVertex; i++) {
         tmp[i].u = 0.0f;
         tmp[i].v = coef;
      }
      glBufferSubData(GL_ARRAY_BUFFER, (GLintptr) m_vboOffsetCoordForShadowMap, sizeof(TexCoord) * m_numVertex, tmp);
      free(tmp);
      m_selfShadowDensityCoef = coef;
      glBindBuffer(GL_ARRAY_BUFFER, 0);
   }
}

/* PMDModel::calculateBoundingSphereRange: calculate the bounding sphere for depth texture rendering on shadow mapping */
float PMDModel::calculateBoundingSphereRange(btVector3 *cpos)
{
   unsigned short i;
   btVector3 centerPos = btVector3(0, 0, 0), tmpPos;
   float maxRange = 0.0f, tmpRange;

   if (m_centerBone) {
      centerPos = m_centerBone->getTransform()->getOrigin();
      for (i = 0; i < m_numBone; i++) {
         if (m_maxDistanceFromVertexAssignedBone[i] == -1.0f) continue;
         tmpPos = m_boneList[i].getTransform()->getOrigin();
         tmpRange = centerPos.distance(tmpPos) + m_maxDistanceFromVertexAssignedBone[i];
         if (maxRange < tmpRange)
            maxRange = tmpRange;
      }
      maxRange *= 1.2f;
   } else {
      maxRange = 0.0f;
   }
   if (cpos) *cpos = centerPos;
   return maxRange;
}

/* PMDModel::smearAllBonesToDefault: smear all bone pos/rot into default value (rate 1.0 = keep, rate 0.0 = reset) */
void PMDModel::smearAllBonesToDefault(float rate)
{
   unsigned short i;
   const btVector3 v(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f));
   const btQuaternion q(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));
   btVector3 tmpv;
   btQuaternion tmpq;

   for (i = 0; i < m_numBone; i++) {
      m_boneList[i].getCurrentPosition(&tmpv);
      tmpv = v.lerp(tmpv, btScalar(rate));
      m_boneList[i].setCurrentPosition(&tmpv);
      m_boneList[i].getCurrentRotation(&tmpq);
      tmpq = q.slerp(tmpq, btScalar(rate));
      m_boneList[i].setCurrentRotation(&tmpq);
   }
   for (i = 0; i < m_numFace; i++) {
      m_faceList[i].setWeight(m_faceList[i].getWeight() * rate);
   }
}

#if !defined(MMDFILES_DONTSORTORDERFORALPHARENDERING) && !defined(MMDFILES_DONTUSEGLMAPBUFFER)
/* compareAlphaDepth: qsort function for reordering material */
static int compareAlphaDepth(const void *a, const void *b)
{
   MaterialDistanceData *x = (MaterialDistanceData *) a;
   MaterialDistanceData *y = (MaterialDistanceData *) b;

   if (x->alpha < 1.0f && y->alpha < 1.0f) {
      if (x->dist == y->dist)
         return 0;
      return ( (x->dist > y->dist) ? 1 : -1 );
   } else if (x->alpha == 1.0f && y->alpha < 1.0f) {
      return -1;
   } else if (x->alpha < 1.0f && y->alpha == 1.0f) {
      return 1;
   } else {
      return 0;
   }
}
#endif /* !MMDFILES_DONTSORTORDERFORALPHARENDERING && !MMDFILES_DONTUSEGLMAPBUFFER */

/* PMDModel::updateMaterialOrder: update material order */
void PMDModel::updateMaterialOrder(btTransform *trans)
{
#if !defined(MMDFILES_DONTSORTORDERFORALPHARENDERING) && !defined(MMDFILES_DONTUSEGLMAPBUFFER)
   unsigned int i;
   btVector3 pos;
   btVector3 *vertexList;
   char *ptr;

   glBindBuffer(GL_ARRAY_BUFFER, m_vboBufDynamic);
   ptr = (char *) glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
   if (!ptr) {
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      return;
   }
   vertexList = (btVector3 *)(ptr + m_vboOffsetVertex);

   for (i = 0; i < m_numMaterial; i++) {
      pos = vertexList[m_material[i].getCenterPositionIndex()];
      pos = *trans * pos;
      m_materialDistance[i].dist = pos.z() + m_material[i].getCenterVertexRadius();
      if (m_material[i].getAlpha() == 1.0f && m_material[i].getTexture() != NULL && m_material[i].getTexture()->isTransparent())
         m_materialDistance[i].alpha = 0.99f;
      else
         m_materialDistance[i].alpha = m_material[i].getAlpha();
      m_materialDistance[i].id = i;
   }

   glUnmapBuffer(GL_ARRAY_BUFFER);
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   qsort(m_materialDistance, m_numMaterial, sizeof(MaterialDistanceData), compareAlphaDepth);
   for (i = 0; i < m_numMaterial; i++)
      m_materialRenderOrder[i] = m_materialDistance[i].id;
#endif /* !MMDFILES_DONTSORTORDERFORALPHARENDERING && !MMDFILES_DONTUSEGLMAPBUFFER */
}

/* PMDModel::getMaterialRenderOrder: get material rendering order */
unsigned int *PMDModel::getMaterialRenderOrder()
{
   return m_materialRenderOrder;
}

/* PMDModel::setRenderingVBOFunction: set rendering VBO function */
bool PMDModel::setRenderingVBOFunction(int flag)
{
   switch(flag) {
   case PMDMODEL_VBO_MAPBUFFERRANGE:
      m_vboMethod = PMDMODEL_VBO_MAPBUFFERRANGE;
      break;
   case PMDMODEL_VBO_MAPBUFFER:
      m_vboMethod = PMDMODEL_VBO_MAPBUFFER;
      break;
   case PMDMODEL_VBO_BUFFERDATA:
      m_vboMethod = PMDMODEL_VBO_BUFFERDATA;
      break;
   case PMDMODEL_VBO_AUTO:
      m_vboMethod = PMDMODEL_VBO_AUTO;
      break;
   default:
      return false;
   }
   return true;
}
