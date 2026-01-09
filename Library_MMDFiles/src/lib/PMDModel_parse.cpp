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

/* PMDModel::parse: initialize and load from data memories */
bool PMDModel::parse(const unsigned char *data, unsigned long size, BulletPhysics *bullet, SystemTexture *systex, const char *dir, bool loadTextureFlag)
{
   const unsigned char *start = data;
   bool ret = true;
   unsigned int i;
   btQuaternion defaultRot(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));

   char sjisBuff[257];

   PMDFile_Header fileHeader;
   PMDFile_Vertex fileVertex;
   PMDFile_Material fileMaterial;
   PMDFile_Bone fileBone;
   PMDFile_IK fileIK;
   PMDFile_Face fileFace;

   unsigned char numFaceDisp;
   unsigned char numBoneFrameDisp;
   unsigned int numBoneDisp;

   char buf[MMDFILES_MAXBUFLEN]; /* for toon texture */

   unsigned char englishNameExist;
   char *exToonBMPName;
   char *name;

   btVector3 tmpVector;
   PMDFile_RigidBody fileRigidBody;
   PMDFile_Constraint fileConstraint;

   unsigned short j, k, l;
   unsigned int surfaceFrom;
   PMDBone *bMatch;

   /* clear memory */
   clear();
   m_hasSingleSphereMap = false;
   m_hasMultipleSphereMap = false;
   m_baseFace = NULL;
   m_centerBone = NULL;

   /* reset root bone's rotation */
   m_rootBone.setCurrentRotation(&defaultRot);
   m_rootBone.update();

   /* set Bullet Physics */
   m_bulletPhysics = bullet;

   /* reset toon texture IDs by system default textures */
   for (j = 0; j < SYSTEMTEXTURE_NUMFILES; j++)
      m_toonTextureID[j] = systex->getTextureID(j);

   /* header */
   memcpy(&fileHeader, data, sizeof(PMDFile_Header));
   if (fileHeader.magic[0] != 'P' || fileHeader.magic[1] != 'm' || fileHeader.magic[2] != 'd')
      return false;
   if (fileHeader.version != 1.0f)
      return false;
   /* name */
   strncpy(sjisBuff, fileHeader.name, 20);
   sjisBuff[20] = '\0';
   m_name = MMDFiles_strdup_from_sjis_to_utf8(sjisBuff);
   /* comment */
   strncpy(sjisBuff, fileHeader.comment, 256);
   sjisBuff[256] = '\0';
   m_comment = MMDFiles_strdup_from_sjis_to_utf8(sjisBuff);
   data += sizeof(PMDFile_Header);

   /* vertex data and bone weights */
   /* relocate as separated list for later OpenGL calls */
   memcpy(&m_numVertex, data, sizeof(unsigned int));
   data += sizeof(unsigned int);
   m_vertexList = (btVector3 *)MMDFiles_alignedmalloc(sizeof(btVector3) * m_numVertex, 16);
   m_normalList = (btVector3 *)MMDFiles_alignedmalloc(sizeof(btVector3) * m_numVertex, 16);
   m_texCoordList = (TexCoord *)malloc(sizeof(TexCoord) * m_numVertex);
   m_bone1List = (short *) malloc(sizeof(short) * m_numVertex);
   m_bone2List = (short *) malloc(sizeof(short) * m_numVertex);
   m_boneWeight1 = (float *) malloc(sizeof(float) * m_numVertex);
   m_edgeWidth = (float *) malloc(sizeof(float) * m_numVertex);
   for (i = 0; i < m_numVertex; i++) {
      memcpy(&fileVertex, data + sizeof(PMDFile_Vertex) * i, sizeof(PMDFile_Vertex));
      m_vertexList[i].setValue(btScalar(fileVertex.pos[0]), btScalar(fileVertex.pos[1]), btScalar(fileVertex.pos[2]));
      m_normalList[i].setValue(btScalar(fileVertex.normal[0]), btScalar(fileVertex.normal[1]), btScalar(fileVertex.normal[2]));
      m_texCoordList[i].u = fileVertex.uv[0];
      m_texCoordList[i].v = fileVertex.uv[1];
      m_bone1List[i] = fileVertex.boneID[0];
      m_bone2List[i] = fileVertex.boneID[1];
      m_boneWeight1[i] = fileVertex.boneWeight1 * 0.01f;
      m_edgeWidth[i] = fileVertex.noEdgeFlag ? 0.0f : 1.0f;
   }
   data += sizeof(PMDFile_Vertex) * m_numVertex;

   /* surface data, 3 vertex indices for each */
   memcpy(&m_numSurface, data, sizeof(unsigned int));
   data += sizeof(unsigned int);
   m_surfaceList = (INDICES *) malloc(sizeof(INDICES) * m_numSurface);
   for (unsigned int ii = 0; ii < m_numSurface; ii++) {
      m_surfaceList[ii] = *((unsigned short *)data);
      data += sizeof(unsigned short);
   }

   /* material data (color, texture, toon parameter, edge flag) */
   memcpy(&m_numMaterial, data, sizeof(unsigned int));
   data += sizeof(unsigned int);
   m_material = new PMDMaterial[m_numMaterial];
   surfaceFrom = 0;
   if (loadTextureFlag)
      m_loadingProgressRate = 0.1f;
   for (i = 0; i < m_numMaterial; i++) {
      memcpy(&fileMaterial, data + sizeof(PMDFile_Material) * i, sizeof(PMDFile_Material));
      if (!m_material[i].setup(&fileMaterial, loadTextureFlag ? &m_textureLoader : NULL, dir, surfaceFrom)) {
         /* ret = false; */
      }
      surfaceFrom += m_material[i].getNumSurface();
      if (loadTextureFlag)
         m_loadingProgressRate = 0.1f + (0.7f * i) / m_numMaterial;
   }
   data += sizeof(PMDFile_Material) * m_numMaterial;
   if (loadTextureFlag)
      m_loadingProgressRate = 0.8f;

   /* bone data */
   memcpy(&m_numBone, data, sizeof(unsigned short));
   data += sizeof(unsigned short);

   m_boneList = (PMDBone*)MMDFiles_alignedmalloc(sizeof(PMDBone) * m_numBone, 16);
   for (i = 0; i < m_numBone; i++) {
      new (m_boneList + i) PMDBone();
      memcpy(&fileBone, data + sizeof(PMDFile_Bone) * i, sizeof(PMDFile_Bone));
      if (!m_boneList[i].setup(&fileBone, m_boneList, m_numBone, &m_rootBone))
         ret = false;
      m_boneList[i].setId(i);
      if (MMDFiles_strequal(m_boneList[i].getName(), PMDMODEL_CENTERBONENAME) == true && m_centerBone == NULL)
         m_centerBone = &(m_boneList[i]);
   }
   if (!m_centerBone && m_numBone >= 1) {
      /* if no bone is named "center," use the first bone as center */
      m_centerBone = &(m_boneList[0]);
   }
   /* make ordered bone list */
   if (m_numBone > 0) {
      m_orderedBoneList = (PMDBone **) malloc(sizeof(PMDBone *) * m_numBone);
      k = 0;
      for (j = 0; j < m_numBone; j++) {
         memcpy(&fileBone, data + sizeof(PMDFile_Bone) * j, sizeof(PMDFile_Bone));
         if (fileBone.parentBoneID == -1)
            m_orderedBoneList[k++] = &(m_boneList[j]);
      }
      l = k;
      for (j = 0; j < m_numBone; j++) {
         memcpy(&fileBone, data + sizeof(PMDFile_Bone) * j, sizeof(PMDFile_Bone));
         if (fileBone.parentBoneID != -1)
            m_orderedBoneList[l++] = &(m_boneList[j]);
      }
      do {
         i = 0;
         for (j = k; j < m_numBone; j++) {
            for (l = 0; l < j; l++) {
               if (m_orderedBoneList[l] == m_orderedBoneList[j]->getParentBone())
                  break;
            }
            if (l >= j) {
               bMatch = m_orderedBoneList[j];
               if (j < m_numBone - 1)
                  memmove(&m_orderedBoneList[j], &m_orderedBoneList[j + 1], sizeof(PMDBone *) * (m_numBone - 1 - j));
               m_orderedBoneList[m_numBone - 1] = bMatch;
               i = 1;
            }
         }
      } while (i != 0);
   }
   /* calculate bone offset after all bone positions are loaded */
   for (i = 0; i < m_numBone; i++)
      m_boneList[i].computeOffset();

   data += sizeof(PMDFile_Bone) * m_numBone;

   /* IK data */
   memcpy(&m_numIK, data, sizeof(unsigned short));
   data += sizeof(unsigned short);
   if (m_numIK > 0) {
      m_IKList = new PMDIK[m_numIK];
      for (i = 0; i < m_numIK; i++) {
         memcpy(&fileIK, data, sizeof(PMDFile_IK));
         data += sizeof(PMDFile_IK);
         m_IKList[i].setup(&fileIK, data, m_boneList);
         data += sizeof(short) * fileIK.numLink;
      }
   }

   /* face data */
   memcpy(&m_numFace, data, sizeof(unsigned short));
   data += sizeof(unsigned short);
   if (m_numFace > 0) {
      m_faceList = new PMDFace[m_numFace];
      for (i = 0; i < m_numFace; i++) {
         memcpy(&fileFace, data, sizeof(PMDFile_Face));
         data += sizeof(PMDFile_Face);
         m_faceList[i].setup(&fileFace, data);
         data += sizeof(PMDFile_Face_Vertex) * fileFace.numVertex;
      }
      m_baseFace = &(m_faceList[0]); /* store base face */
      /* convert base-relative index to the index of original vertices */
      for (i = 1; i < m_numFace; i++)
         m_faceList[i].convertIndex(m_baseFace);
   }

   /* display names (skip) */
   /* indices for faces which should be displayed in "face" region */
   memcpy(&numFaceDisp, data, sizeof(unsigned char));
   data += sizeof(unsigned char) + sizeof(unsigned short) * numFaceDisp;
   /* bone frame names */
   memcpy(&numBoneFrameDisp, data, sizeof(unsigned char));
   data += sizeof(unsigned char) + 50 * numBoneFrameDisp;
   /* indices for bones which should be displayed in each bone region */
   memcpy(&numBoneDisp, data, sizeof(unsigned int));
   data += sizeof(unsigned int) + (sizeof(short) + sizeof(unsigned char)) * numBoneDisp;

   /* end of base format */
   /* check for remaining data */
   if ((uintptr_t) data - (uintptr_t) start >= size) {
      /* no extension data remains */
      m_numRigidBody = 0;
      m_numConstraint = 0;
      /* assign default toon textures for toon shading */
      for (j = 0; j <= 10; j++) {
         if (j == 0)
            MMDFiles_snprintf(buf, MMDFILES_MAXBUFLEN, "%s%ctoon0.bmp", dir, MMDFILES_DIRSEPARATOR);
         else
            MMDFiles_snprintf(buf, MMDFILES_MAXBUFLEN, "%s%ctoon%02d.bmp", dir, MMDFILES_DIRSEPARATOR, j);
         /* if "toon??.bmp" exist at the same directory as PMD file, use it */
         /* if not exist or failed to read, use system default toon textures */
         if (MMDFiles_exist(buf)) {
            if (m_localToonTexture[j].load(buf) == true) {
               m_toonTextureID[j] = m_localToonTexture[j].getID();
               /* set GL_CLAMP_TO_EDGE for toon texture to avoid texture interpolation at edge */
               glBindTexture(GL_TEXTURE_2D, m_toonTextureID[j]);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
               glBindTexture(GL_TEXTURE_2D, 0);
            }
         }
      }
   } else {
      /* English display names (skip) */
      memcpy(&englishNameExist, data, sizeof(unsigned char));
      data += sizeof(unsigned char);
      if (englishNameExist != 0) {
         /* model name and comments in English */
         data += 20 + 256;
         /* bone names in English */
         data += 20 * m_numBone;
         /* face names in English */
         if (m_numFace > 0) data += 20 * (m_numFace - 1); /* "base" not included in English list */
         /* bone frame names in English */
         data += 50 * numBoneFrameDisp;
      }

      /* toon texture file list (replace toon01.bmp - toon10.bmp) */
      /* the "toon0.bmp" should be loaded separatedly */
      MMDFiles_snprintf(buf, MMDFILES_MAXBUFLEN, "%s%ctoon0.bmp", dir, MMDFILES_DIRSEPARATOR);
      if (MMDFiles_exist(buf)) {
         if (m_localToonTexture[0].load(buf) == true) {
            m_toonTextureID[0] = m_localToonTexture[0].getID();
            /* set GL_CLAMP_TO_EDGE for toon texture to avoid texture interpolation at edge */
            glBindTexture(GL_TEXTURE_2D, m_toonTextureID[0]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_2D, 0);
         }
      }
      for (i = 1; i <= 10; i++) {
         exToonBMPName = MMDFiles_strdup_from_sjis_to_utf8((char *)data);
         MMDFiles_snprintf(buf, MMDFILES_MAXBUFLEN, "%s%c%s", dir, MMDFILES_DIRSEPARATOR, exToonBMPName);
         if (MMDFiles_exist(buf)) {
            if (m_localToonTexture[i].load(buf) == true) {
               m_toonTextureID[i] = m_localToonTexture[i].getID();
               /* set GL_CLAMP_TO_EDGE for toon texture to avoid texture interpolation at edge */
               glBindTexture(GL_TEXTURE_2D, m_toonTextureID[i]);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
               glBindTexture(GL_TEXTURE_2D, 0);
            }
         }
         free(exToonBMPName);
         data += 100;
      }

      /* check for remaining data */
      if ((uintptr_t) data - (uintptr_t) start >= size) {
         /* no rigid body / constraint data exist */
         m_numRigidBody = 0;
         m_numConstraint = 0;
      } else if (!m_bulletPhysics) {
         /* check if we have given a bulletphysics engine */
         m_numRigidBody = 0;
         m_numConstraint = 0;
      } else {
         /* get model offset */
         m_rootBone.getOffset(&tmpVector);
         /* update bone matrix to apply root bone offset to bone position */
         for (i = 0; i < m_numBone; i++)
            m_orderedBoneList[i]->update();

         /* Bullet Physics rigidbody data */
         memcpy(&m_numRigidBody, data, sizeof(unsigned int));
         data += sizeof(unsigned int);
         if (m_numRigidBody > 0) {
            m_rigidBodyList = (PMDRigidBody*)MMDFiles_alignedmalloc(sizeof(PMDRigidBody) * m_numRigidBody, 16);
            for (i = 0; i < m_numRigidBody; i++) {
               new (m_rigidBodyList + i)PMDRigidBody();
               memcpy(&fileRigidBody, data + sizeof(PMDFile_RigidBody) * i, sizeof(PMDFile_RigidBody));
               if (! m_rigidBodyList[i].setup(&fileRigidBody, (fileRigidBody.boneID == 0xFFFF) ? & (m_boneList[0]) : & (m_boneList[fileRigidBody.boneID])))
                  ret = false;
               /* flag the bones under simulation in order to skip IK solving for those bones */
               if (fileRigidBody.type != 0 && fileRigidBody.boneID != 0xFFFF)
                  m_boneList[fileRigidBody.boneID].setSimulatedFlag(true);
            }
            data += sizeof(PMDFile_RigidBody) * m_numRigidBody;
         }

         /* BulletPhysics constraint data */
         memcpy(&m_numConstraint, data, sizeof(unsigned int));
         data += sizeof(unsigned int);
         if (m_numConstraint > 0) {
            m_constraintList = new PMDConstraint[m_numConstraint];
            for (i = 0; i < m_numConstraint; i++) {
               memcpy(&fileConstraint, data + sizeof(PMDFile_Constraint) * i, sizeof(PMDFile_Constraint));
               if (!m_constraintList[i].setup(&fileConstraint, m_rigidBodyList, &tmpVector)) /* apply model offset */
                  ret = false;
            }
            data += sizeof(PMDFile_Constraint) * m_numConstraint;
         }
      }
   }

   if (ret == false) return false;

   /* build name->entity index for fast lookup */
   for (j = 0; j < m_numBone; j++) {
      name = m_boneList[j].getName();
      if (name) m_name2bone.add(name, (int)strlen(name), &(m_boneList[j]));
   }
   for (j = 0; j < m_numFace; j++) {
      name = m_faceList[j].getName();
      if (name) m_name2face.add(name, (int)strlen(name), &(m_faceList[j]));
   }

   return ret;
}

/* PMDModel::setupModel: set up model work area after parses */
bool PMDModel::setupModel()
{
   unsigned int i;
   btQuaternion defaultRot;

   btVector3 modelOffset;
   btVector3 tmpVector;

   unsigned short j, k;
   float f;

#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
   /* left-handed system: PMD, DirectX */
   /* right-handed system: OpenGL, bulletphysics */
   /* convert the left-handed vertices to right-handed system */
   /* 1. vectors should be (x, y, -z) */
   /* 2. rotation should be (-rx, -ry, z) */
   /* 3. surfaces should be reversed */
   /* reverse Z value on vertices */
   for (i = 0; i < m_numVertex; i++) {
      m_vertexList[i].setZ(-m_vertexList[i].z());
      m_normalList[i].setZ(-m_normalList[i].z());
   }
#ifdef MY_EXTRADEFORMATION
   if (m_sdefC != NULL) {
      for (i = 0; i < m_numVertex; i++) {
         m_sdefC[i].setZ(-m_sdefC[i].z());
         m_sdefR0[i].setZ(-m_sdefR0[i].z());
         m_sdefR1[i].setZ(-m_sdefR1[i].z());
      }
   }
#endif /* MY_EXTRADEFORMATION */
   /* reverse surface, swapping vartex order [0] and [1] in a triangle surface */
   for (i = 0; i < m_numSurface; i += 3) {
      INDICES jj = m_surfaceList[i];
      m_surfaceList[i] = m_surfaceList[i + 1];
      m_surfaceList[i + 1] = jj;
   }
#endif /* MMDFILES_CONVERTCOORDINATESYSTEM */
   /* get center vertices of materials */
   for (i = 0; i < m_numMaterial; i++) {
      m_material[i].computeCenterVertex(m_vertexList, m_surfaceList);
   }

   /* prepare work area */
   /* transforms for skinning */
   m_boneSkinningTrans = (btTransform *)MMDFiles_alignedmalloc(sizeof(btTransform) * m_numBone, 16);
   /* surface list to be rendered at edge drawing (skip non-edge materials) */
   m_numSurfaceForEdge = 0;
   for (i = 0; i < m_numMaterial; i++)
      if (m_material[i].getEdgeFlag())
         m_numSurfaceForEdge += m_material[i].getNumSurface();
   m_numSurfaceForShadowMap = 0;
   for (i = 0; i < m_numMaterial; i++)
      if (m_material[i].getShadowMapRenderFlag())
         m_numSurfaceForShadowMap += m_material[i].getNumSurface();
   m_numSurfaceForShadow = 0;
   if (m_hasExtParam) {
      /* surface list to be rendered for shadow (only when EXT information is loaded, else use same list as edge) */
      for (i = 0; i < m_numMaterial; i++)
         if (m_material[i].getShadowFlag())
            m_numSurfaceForShadow += m_material[i].getNumSurface();
   }

   /* save base vertex if has vertex morph */
   if (m_vertexMorphList) {
      m_baseVertexList = (btVector3 *)MMDFiles_alignedmalloc(sizeof(btVector3) * m_numVertex, 16);
      memcpy(m_baseVertexList, m_vertexList, sizeof(btVector3) * m_numVertex);
   }

   /* save base texture coordinates if has uv morph */
   if (m_uvMorphList) {
      m_baseTexCoordList = (TexCoord *)malloc(sizeof(TexCoord) * m_numVertex);
      memcpy(m_baseTexCoordList, m_texCoordList, sizeof(TexCoord) * m_numVertex);
   }

   /* compute edge width */
   bool *checked = (bool *)malloc(sizeof(bool) * m_numVertex);
   for (i = 0; i < m_numVertex; i++)
      checked[i] = false;
   for (i = 0; i < m_numMaterial; i++) {
      unsigned int surfaceIndex = m_material[i].getSurfaceListIndex();
      unsigned int numSurface = m_material[i].getNumSurface();
      for (unsigned int ii = 0; ii < numSurface; ii++) {
         if (checked[m_surfaceList[surfaceIndex + ii]] == false) {
            m_edgeWidth[m_surfaceList[surfaceIndex + ii]] *= m_material[i].getEdgeWidth();
            checked[m_surfaceList[surfaceIndex + ii]] = true;
         }
      }
   }
   free(checked);
   for (i = 0; i < m_numVertex; i++)
      m_edgeWidth[i] *= m_edgeOffset;

#ifdef MY_EXTRADEFORMATION
   /* for SDEF vertices, pre-compute static coefs for skinning */
   if (m_sdefC != NULL) {
      m_sdefCR0 = (btVector3 *)MMDFiles_alignedmalloc(sizeof(btVector3) * m_numVertex, 16);
      m_sdefCR1 = (btVector3 *)MMDFiles_alignedmalloc(sizeof(btVector3) * m_numVertex, 16);
      for (i = 0; i < m_numVertex; i++) {
         if (m_bone3List[i] == -2) {
            btVector3 wp, r0, r1;
            wp = m_sdefR1[i].lerp(m_sdefR0[i], m_boneWeight1[i]);
            r0 = m_sdefC[i] + m_sdefR0[i] - wp;
            r1 = m_sdefC[i] + m_sdefR1[i] - wp;
            m_sdefCR0[i] = m_sdefC[i].lerp(r0, 0.5f);
            m_sdefCR1[i] = m_sdefC[i].lerp(r1, 0.5f);
         }
      }
   }
#endif /* MY_EXTRADEFORMATION */

#ifdef MY_LUMINOUS
   /* check if this model has luminous material */
   m_hasLuminousMaterial = false;
   for (i = 0; i < m_numMaterial; i++) {
      if (m_material[i].getLimunousFlag() == true) {
         m_hasLuminousMaterial = true;
         break;
      }
   }
#endif

   /* initialize material order */
   m_materialRenderOrder = (unsigned int *) malloc(sizeof(unsigned int) * m_numMaterial);
   m_materialDistance = (MaterialDistanceData *) malloc(sizeof(MaterialDistanceData) * m_numMaterial);
   for (i = 0; i < m_numMaterial; i++)
      m_materialRenderOrder[i] = i;
   /* check if spheremap is used (single or multiple) */
   for (i = 0; i < m_numMaterial; i++) {
      if (m_material[i].hasSingleSphereMap())
         m_hasSingleSphereMap = true;
      if (m_material[i].hasMultipleSphereMap())
         m_hasMultipleSphereMap = true;
   }

   /* make index of rotation-subjective bones (type == UNDER_ROTATE) and subjective bones */
   if (m_numBone > 0) {
      m_rotateBoneIDList = (unsigned short *) malloc(sizeof(unsigned short) * m_numBone);
      for (j = 0; j < m_numBone; j++) {
         if (m_boneList[j].getType() == UNDER_ROTATE)
            m_rotateBoneIDList[m_numRotateBone++] = j;
      }
      if(m_numRotateBone > 0) {
         do {
            i = 0;
            for (j = 0; j < m_numBone; j++) {
               for (k = 0; k < m_numRotateBone; k++) {
                  if (m_rotateBoneIDList[k] == j)
                     break;
               }
               if (k >= m_numRotateBone) {
                  for (k = 0; k < m_numRotateBone; k++) {
                     if (&(m_boneList[m_rotateBoneIDList[k]]) == m_boneList[j].getParentBone()) {
                        m_rotateBoneIDList[m_numRotateBone++] = j;
                        i = 1;
                        break;
                     }
                  }
               }
            }
         } while(i == 1);
      }
   }

   /* get max layer */
   if (m_numBone > 0) {
      short maxlayer = 0;
      for (j = 0; j < m_numBone; j++) {
         if (m_boneList[j].getProcessLayer() > maxlayer)
            maxlayer = m_boneList[j].getProcessLayer();
      }
      m_maxProcessLayer = maxlayer;
   }

   /* check if some IK solvers can be disabled since the bones are simulated by physics */
   if (m_numIK > 0) {
      m_IKSimulated = (bool *) malloc(sizeof(bool) * m_numIK);
      for (j = 0; j < m_numIK; j++) {
         /* this IK will be disabled when the leaf bone is controlled by physics simulation */
         m_IKSimulated[j] = m_IKList[j].isSimulated();
         /* update IK info */
         m_IKList[j].updateInfo();
      }
   }

   /* mark motion independency for each bone */
   for (j = 0; j < m_numBone; j++)
      m_boneList[j].setMotionIndependency();

   /* make vertex assigned bone max distance */
   if (m_numBone > 0) {
      m_maxDistanceFromVertexAssignedBone = (float *) malloc(sizeof(float) * m_numBone);
      for (j = 0; j < m_numBone; j++)
         m_maxDistanceFromVertexAssignedBone[j] = -1.0f;
      for (i = 0; i < m_numVertex; i++) {
         if (m_boneWeight1[i] >= PMDMODEL_MINBONEWEIGHT) {
            m_boneList[m_bone1List[i]].getOriginPosition(&tmpVector);
            f = tmpVector.distance2(m_vertexList[i]);
            if (m_maxDistanceFromVertexAssignedBone[m_bone1List[i]] < f)
               m_maxDistanceFromVertexAssignedBone[m_bone1List[i]] = f;
         }
         if (m_boneWeight1[i] <= 1.0f - PMDMODEL_MINBONEWEIGHT) {
            m_boneList[m_bone2List[i]].getOriginPosition(&tmpVector);
            f = tmpVector.distance2(m_vertexList[i]);
            if (m_maxDistanceFromVertexAssignedBone[m_bone2List[i]] < f)
               m_maxDistanceFromVertexAssignedBone[m_bone2List[i]] = f;
         }
      }
      for (j = 0; j < m_numBone; j++)
         if (m_maxDistanceFromVertexAssignedBone[j] != -1.0f)
            m_maxDistanceFromVertexAssignedBone[j] = sqrtf(m_maxDistanceFromVertexAssignedBone[j]);
   }

   /* get maximum height */
   if (m_numVertex > 0) {
      m_maxHeight = m_vertexList[0].y();
      for (i = 1; i < m_numVertex; i++)
         if (m_maxHeight < m_vertexList[i].y())
            m_maxHeight = m_vertexList[i].y();
   }

   /* get bounding sphere step */
   m_boundingSphereStep = m_numVertex / PMDMODEL_BOUNDINGSPHEREPOINTS;
   if (m_boundingSphereStep < PMDMODEL_BOUNDINGSPHEREPOINTSMIN) m_boundingSphereStep = PMDMODEL_BOUNDINGSPHEREPOINTSMIN;
   if (m_boundingSphereStep > PMDMODEL_BOUNDINGSPHEREPOINTSMAX) m_boundingSphereStep = PMDMODEL_BOUNDINGSPHEREPOINTSMAX;

   /* simulation is currently off, so change bone status */
   if (!m_enableSimulation)
#ifdef MY_RESETPHYSICS
      setPhysicsControl(false, true);
#else
      setPhysicsControl(false);
#endif

   return true;
}

/* PMDModel::setupObjects: set up OpenGL Objects after parses */
void PMDModel::setupObjects()
{
   unsigned int i;
   unsigned int surfaceFrom, surfaceTo;

   /* join rigid bodies to world */
   if (m_bulletPhysics) {
      for (i = 0; i < m_numRigidBody; i++)
         m_rigidBodyList[i].joinWorld(m_bulletPhysics->getWorld());
      for (i = 0; i < m_numConstraint; i++)
         m_constraintList[i].joinWorld(m_bulletPhysics->getWorld());
      m_bulletPhysics->setModifiedFlag();
   }

   /* prepare vertex buffers as work area */

   /* the fist buffer contains dynamic data: vertex, normal, toon coodinates, edge vertex */
   glGenBuffers(1, &m_vboBufDynamic);
   glBindBuffer(GL_ARRAY_BUFFER, m_vboBufDynamic);
   m_vboBufDynamicLen = (sizeof(btVector3) * 3 + sizeof(TexCoord)) * m_numVertex;
   glBufferData(GL_ARRAY_BUFFER, m_vboBufDynamicLen, NULL, GL_DYNAMIC_DRAW);
   /* store initial values and set offset for each part */
   m_vboOffsetVertex = 0;
   glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)m_vboOffsetVertex, sizeof(btVector3) * m_numVertex, m_vertexList);
   m_vboOffsetNormal = m_vboOffsetVertex + sizeof(btVector3) * m_numVertex;
   glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)m_vboOffsetNormal, sizeof(btVector3) * m_numVertex, m_normalList);
   m_vboOffsetEdge = m_vboOffsetNormal + sizeof(btVector3) * m_numVertex;
   m_vboOffsetToon = m_vboOffsetEdge + sizeof(btVector3) * m_numVertex;

   /* the second buffer contains static data: texture coordinates */
   glGenBuffers(1, &m_vboBufStatic);
   glBindBuffer(GL_ARRAY_BUFFER, m_vboBufStatic);
   if (m_uvMorphList)
      glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoord) * m_numVertex, m_texCoordList, GL_DYNAMIC_DRAW);
   else
      glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoord) * m_numVertex, m_texCoordList, GL_STATIC_DRAW);

   /* the third buffer contains static element data: original and for edge */
   glGenBuffers(1, &m_vboBufElement);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboBufElement);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(INDICES) * (m_numSurface + m_numSurfaceForEdge + m_numSurfaceForShadow), NULL, GL_STATIC_DRAW);
   m_vboOffsetSurfaceForEdge = sizeof(INDICES) * m_numSurface;
   if (m_hasExtParam)
      m_vboOffsetSurfaceForShadow = m_vboOffsetSurfaceForEdge + sizeof(INDICES) * m_numSurfaceForEdge;
   if (m_numSurfaceForEdge == 0)
      m_vboOffsetSurfaceForEdge = 0;
   if (m_hasExtParam)
      if (m_numSurfaceForShadow == 0)
         m_vboOffsetSurfaceForShadow = 0;
   glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, (GLintptr)0, sizeof(INDICES) * m_numSurface, m_surfaceList);
   if (m_numSurfaceForEdge != 0) {
      surfaceFrom = 0;
      surfaceTo = m_vboOffsetSurfaceForEdge;
      for (i = 0; i < m_numMaterial; i++) {
         if (m_material[i].getEdgeFlag()) {
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, (GLintptr)surfaceTo, sizeof(INDICES) * m_material[i].getNumSurface(), &(m_surfaceList[surfaceFrom]));
            surfaceTo += sizeof(INDICES) * m_material[i].getNumSurface();
         }
         surfaceFrom += m_material[i].getNumSurface();
      }
   }
   if (m_hasExtParam) {
      /* surface list to be rendered for shadow (only when EXT information is loaded, else use same list as edge) */
      if (m_numSurfaceForShadow > 0) {
         surfaceFrom = 0;
         surfaceTo = m_vboOffsetSurfaceForShadow;
         for (i = 0; i < m_numMaterial; i++) {
            if (m_material[i].getShadowFlag()) {
               glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, (GLintptr)surfaceTo, sizeof(INDICES) * m_material[i].getNumSurface(), &(m_surfaceList[surfaceFrom]));
               surfaceTo += sizeof(INDICES) * m_material[i].getNumSurface();
            }
            surfaceFrom += m_material[i].getNumSurface();
         }
      }
   }

   /* unbind buffer */
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   m_loadingProgressRate = 1.0f;
}

/* PMDModel::parseExtCsv: load EXT information from csv file */
bool PMDModel::parseExtCsv(const char *file, const char *dir)
{
   ZFile *zf;
   char buf[4096];
   char *p, *q, save;
   unsigned long i, k;
   unsigned long numMaterial;
   float col[4];
   float alpha, edgeWidth;
   bool face, edge, shadow, shadowMapDrop, shadowMapRender;
   char *s, *tex, *sphere;
   unsigned short sphereMode;
   bool PMX2Recent = false;
   unsigned int surface_idx = 0;
   bool PMX257format = false;
   unsigned short numBone;

   /*

   csv converted by PMDEditor:

      "Material" has 29 fields
      "Bone" has 38 fields
      "Joint" has 29 fields

   csv converted by PMXEditor:

      "Material" has 31 fields (+2)
         #18 vertex color (0/1)
         #19 draw (0:Tri/1:Point/2:Line)
      "Bone" has 40 fields (+2)
         #19 local add (0/1)
         #35 outer parent (0/1)
      "Joint" has 30 fields (+1)
         #5  joint type (0 to 5)
   */

   zf = new ZFile(g_enckey);
   if (zf->openAndLoad(file) == false) {
      delete zf;
      return false;
   }

   /* 1st pass: check format, and get bone names */
   /* assuming that ext file contains the same bone list keeping order in pmd */
   numBone = 0;
   while (zf->gets(buf, 4096) != NULL) {
      p = buf;
      q = p;
      while (*q != '\0' && *q != ',') q++;
      save = *q;
      *q = '\0';
      if (MMDFiles_strequal(p, ";PmxHeader")) {
         /* check if this is an csv file built by PMXEditor newer than 0257 */
         PMX257format = true;
         PMX2Recent = true;
         continue;
      } else if (MMDFiles_strequal(p, ";Material") || MMDFiles_strequal(p, ";Bone") || MMDFiles_strequal(p, ";Joint")) {
         /* check if this is an csv file built by recent PMXEditor */
         unsigned int numField;
         if (MMDFiles_strequal(p, ";Material"))
            numField = 31;
         else if (MMDFiles_strequal(p, ";Bone"))
            numField = 40;
         else if (MMDFiles_strequal(p, ";Joint"))
            numField = 30;
         k = 1;
         *q = save;
         while (*q != '\0') {
            if (*q == '"') q++;
            if (*q == '\0') break;
            p = q + 1;
            if (*p == '\0') break;
            if (*p == ',') {
               q = p;
               k++;
               continue;
            }
            if (*p == '"') p++;
            q = p;
            while (*q != '\0' && *q != ',' && *q != '"') q++;
            save = *q;
            *q = '\0';
            k++;
            *q = save;
         }
         if (k >= numField)
            PMX2Recent = true;
         continue;
      } else if (MMDFiles_strequal(p, "Bone") || MMDFiles_strequal(p, "PmxBone")) {
         char *s;

         m_hasExtBoneParam = true;

         k = 1;
         *q = save;
         while (*q != '\0') {
            if (*q == '"') q++;
            if (*q == '\0') break;
            p = q + 1;
            if (*p == '\0') break;
            if (*p == ',') {
               q = p;
               k++;
               continue;
            }
            if (*p == '"') p++;
            q = p;
            while (*q != '\0' && *q != ',' && *q != '"') q++;
            save = *q;
            *q = '\0';
            if (q - 1 >= p && *(q - 1) == '"') *(q - 1) = '\0';
            if (k == 1) {
               /* bone name */
               if (numBone >= m_numBone)
                  continue;
               if (PMX257format)
                  s = p;
               else
                  s = MMDFiles_strdup_from_sjis_to_utf8(p);
               if (MMDFiles_strequal(m_boneList[numBone].getName(), s) == false) {
                  // override the name of the bone with the newly given name
                  m_boneList[numBone].setName(s);
                  // add mapping from the new name to the bone
                  m_name2bone.add(s, (int)MMDFiles_strlen(s), &(m_boneList[numBone]));
               }
               if (PMX257format == false)
                  free(s);
            }
            k++;
            *q = save;
         }
         numBone++;
         m_hasExtParam = true;
      }
   }
   if (numBone > 0 && numBone != m_numBone)
      return false;

   /* 2nd pass: parse all */
   zf->rewind();
   numMaterial = 0;
   while (zf->gets(buf, 4096) != NULL) {
      p = buf;
      q = p;
      while (*q != '\0' && *q != ',') q++;
      save = *q;
      *q = '\0';

      if (MMDFiles_strequal(p, "Material") || MMDFiles_strequal(p, "PmxMaterial")) {
         for (i = 0; i < 4; i++) col[i] = 0.0f;
         alpha = 1.0f;
         face = false;
         shadow = false;
         shadowMapDrop = false;
         shadowMapRender = false;
         edge = false;
         edgeWidth = 1.0f;
         s = NULL;
         tex = NULL;
         sphere = NULL;
         k = 1;
         *q = save;
         while (*q != '\0') {
            if (*q == '"') q++;
            if (*q == '\0') break;
            p = q + 1;
            if (*p == '\0') break;
            if (*p == ',') {
               q = p;
               k++;
               continue;
            }
            if (*p == '"') p++;
            q = p;
            while (*q != '\0' && *q != ',' && *q != '"') q++;
            save = *q;
            *q = '\0';
            if (k == 1) {
               if (PMX257format)
                  s = MMDFiles_strdup(p);
               else
                  s = MMDFiles_strdup_from_sjis_to_utf8(p);
            } else if (PMX2Recent == false && k >= 20 && k <= 23) {
               col[k - 20] = (float)atof(p);
            } else if (PMX2Recent == true && k >= 22 && k <= 25) {
               col[k - 22] = (float)atof(p);
            } else if (k == 6) {
               alpha = (float)atof(p);
            } else if (k == 14) {
               face = atoi(p) ? true : false;
            } else if (k == 15) {
               shadow = atoi(p) ? true : false;
            } else if (k == 16) {
               shadowMapDrop = atoi(p) ? true : false;
            } else if (k == 17) {
               shadowMapRender = atoi(p) ? true : false;
            } else if ((PMX2Recent == false && k == 18) || (PMX2Recent == true && k == 20)) {
               edge = atoi(p) ? true : false;
            } else if ((PMX2Recent == false && k == 19) || (PMX2Recent == true && k == 21)) {
               edgeWidth = (float)atof(p);
            } else if ((PMX2Recent == false && k == 24) || (PMX2Recent == true && k == 26)) {
               if (MMDFiles_strlen(p) > 0) {
                  if (PMX257format)
                     tex = MMDFiles_strdup(p);
                  else
                     tex = MMDFiles_strdup_from_sjis_to_utf8(p);
               }
            } else if ((PMX2Recent == false && k == 25) || (PMX2Recent == true && k == 27)) {
               if (MMDFiles_strlen(p) > 0) {
                  if (PMX257format)
                     sphere = MMDFiles_strdup(p);
                  else
                     sphere = MMDFiles_strdup_from_sjis_to_utf8(p);
               }
            } else if ((PMX2Recent == false && k == 26) || (PMX2Recent == true && k == 28)) {
               sphereMode = atoi(p);
            }
            k++;
            *q = save;
         }
         if (numMaterial >= m_numMaterial)
            continue;
         /* set EXT parameters to the material */
         m_material[numMaterial].setName(s);
         m_material[numMaterial].setExtParam(edge, edgeWidth, &(col[0]), alpha, face, shadow, shadowMapDrop, shadowMapRender, tex, sphere, sphereMode, dir, &m_textureLoader);
         if (s)
            free(s);
         if (tex)
            free(tex);
         if (sphere)
            free(sphere);
         m_hasExtParam = true;
         numMaterial++;
         m_loadingProgressRate = 0.1f + (0.7f * numMaterial) / m_numMaterial;
#ifdef MY_EXTRADEFORMATION
      } else if (MMDFiles_strequal(p, "Vertex") || MMDFiles_strequal(p, "PmxVertex")) {
         unsigned long idx;
         float edgeWidth;
         int kind = 0;
         char *s;
         short bid[4];
         float bwt[4];
         PMDBone *b;
         btVector3 c, r0, r1;
         c.setZero();
         r0.setZero();
         r1.setZero();
         if (m_bone3List == NULL) {
            m_bone3List = (short *)malloc(sizeof(short) * m_numVertex);
            m_bone4List = (short *)malloc(sizeof(short) * m_numVertex);
            m_boneWeight2 = (float *)malloc(sizeof(float) * m_numVertex);
            m_boneWeight3 = (float *)malloc(sizeof(float) * m_numVertex);
            m_boneWeight4 = (float *)malloc(sizeof(float) * m_numVertex);
            m_sdefC = (btVector3 *)MMDFiles_alignedmalloc(sizeof(btVector3) * m_numVertex, 16);
            m_sdefR0 = (btVector3 *)MMDFiles_alignedmalloc(sizeof(btVector3) * m_numVertex, 16);
            m_sdefR1 = (btVector3 *)MMDFiles_alignedmalloc(sizeof(btVector3) * m_numVertex, 16);
            for (i = 0; i < m_numVertex; i++) {
               m_bone3List[i] = m_bone4List[i] = -1;
               m_boneWeight2[i] = 1.0f - m_boneWeight1[i];
               m_boneWeight3[i] = m_boneWeight4[i] = 0.0f;
            }
         }
         k = 1;
         *q = save;
         while (*q != '\0') {
            if (*q == '"') q++;
            if (*q == '\0') break;
            p = q + 1;
            if (*p == '\0') break;
            if (*p == ',') {
               q = p;
               k++;
               continue;
            }
            if (*p == '"') p++;
            q = p;
            while (*q != '\0' && *q != ',' && *q != '"') q++;
            save = *q;
            *q = '\0';
            if (k == 1) {
               idx = atoi(p);
            } else if (k == 8) {
               edgeWidth = (float)atof(p);
            } else if (k == 27) {
               kind = atoi(p); // 0:BDEF1 1:BDEF2 2:BDEF4 3:SDEF)
            } else if ((k == 28 || k == 30 || k == 32 || k == 34) && kind >= 2) {
               if (PMX257format)
                  s = p;
               else
                  s = MMDFiles_strdup_from_sjis_to_utf8(p);
               if (s) {
                  b = getBone(s);
                  if (b != NULL)
                     bid[(k - 28) / 2] = b->getId();
                  else
                     bid[(k - 28) / 2] = 0;
               } else {
                  bid[(k - 28) / 2] = 0;
               }
               if (PMX257format == false)
                  free(s);
            } else if ((k == 29 || k == 31 || k == 33 || k == 35) && kind >= 2) {
               bwt[(k - 29) / 2] = (float)atof(p);
            } else if (k >= 36 && k <= 44 && kind == 3) {
               switch (k - 36) {
               case 0: c.setX((float)atof(p)); break;
               case 1: c.setY((float)atof(p)); break;
               case 2: c.setZ((float)atof(p)); break;
               case 3: r0.setX((float)atof(p)); break;
               case 4: r0.setY((float)atof(p)); break;
               case 5: r0.setZ((float)atof(p)); break;
               case 6: r1.setX((float)atof(p)); break;
               case 7: r1.setY((float)atof(p)); break;
               case 8: r1.setZ((float)atof(p)); break;
               }
            }
            k++;
            *q = save;
         }
         m_edgeWidth[idx] = edgeWidth;
         if (kind == 2) {
            m_bone1List[idx] = bid[0];
            m_bone2List[idx] = bid[1];
            m_bone3List[idx] = bid[2];
            m_bone4List[idx] = bid[3];
            m_boneWeight1[idx] = bwt[0];
            m_boneWeight2[idx] = bwt[1];
            m_boneWeight3[idx] = bwt[2];
            m_boneWeight4[idx] = bwt[3];
         } else if (kind == 3) {
            m_bone1List[idx] = bid[0];
            m_bone2List[idx] = bid[1];
            m_boneWeight1[idx] = bwt[0];
            m_boneWeight2[idx] = bwt[1];
            // workaround to disappearing vertex whem SDEF is abnormal
            if (c != r0 || c != r1) {
               m_bone3List[idx] = -2;
               m_sdefC[idx] = c;
               m_sdefR0[idx] = r0;
               m_sdefR1[idx] = r1;
            }
         }
#endif /* MY_EXTRADEFORMATION */
         m_hasExtParam = true;
      } else if (MMDFiles_strequal(p, "BoneMorph") || MMDFiles_strequal(p, "PmxBoneMorph")) {
         unsigned long numBoneMorphAllocated = 1000;
         unsigned long id;
         PMDBone *bone = NULL;
         float f[3];
         btVector3 pos;
         btQuaternion rot;
         char *s;
         bool local_index_skipped = false;

         if (m_boneMorphList == NULL) {
            m_boneMorphList = new PMDBoneMorph[numBoneMorphAllocated];
            m_numBoneMorph = 0;
         }
         if (m_numBoneMorph >= numBoneMorphAllocated) {
            /* num of bone morph exceeds limit */
            delete zf;
            return false;
         }
         k = 1;
         *q = save;
         while (*q != '\0') {
            if (*q == '"') q++;
            if (*q == '\0') break;
            p = q + 1;
            if (*p == '\0') break;
            if (*p == ',') {
               q = p;
               k++;
               continue;
            }
            if (*p == '"') p++;
            q = p;
            while (*q != '\0' && *q != ',' && *q != '"') q++;
            save = *q;
            *q = '\0';
            if (q - 1 >= p && *(q - 1) == '"') *(q - 1) = '\0';
            if (k == 1) {
               if (PMX257format)
                  s = p;
               else
                  s = MMDFiles_strdup_from_sjis_to_utf8(p);
               for (i = 0; i < m_numBoneMorph; i++) {
                  if (MMDFiles_strequal(m_boneMorphList[i].getName(), s))
                     break;
               }
               if (i < m_numBoneMorph) {
                  id = i;
               } else {
                  id = m_numBoneMorph;
                  m_numBoneMorph++;
                  m_boneMorphList[id].setup(s);
               }
               if (PMX257format == false)
                  free(s);
            } else if (k == 2) {
               if (PMX257format && local_index_skipped == false) {
                  /* PMX257format has extra local offset index at k = 2, skip it */
                  local_index_skipped = true;
                  *q = save;
                  continue;
               }
               if (PMX257format) {
                  bone = getBone(p);
               } else {
                  s = MMDFiles_strdup_from_sjis_to_utf8(p);
                  bone = getBone(s);
                  free(s);
               }
            } else if (k >= 3 && k <= 5) {
               f[k - 3] = (float)atof(p);
               if (k == 5) {
#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
                  pos.setValue(f[0], f[1], -f[2]);
#else
                  pos.setValue(f[0], f[1], f[2]);
#endif /* MMDFILES_CONVERTCOORDINATESYSTEM */
               }
            } else if (k >= 6 && k <= 8) {
               f[k - 6] = (float)atof(p);
               if (k == 8) {
#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
                  rot.setEuler(btScalar(MMDFILES_RAD(-f[1])), btScalar(MMDFILES_RAD(-f[0])), btScalar(MMDFILES_RAD(f[2])));
#else
                  rot.setEulerZYX(btScalar(MMDFILES_RAD(f[2])), btScalar(MMDFILES_RAD(f[1])), btScalar(MMDFILES_RAD(f[0])));
#endif /* MMDFILES_CONVERTCOORDINATESYSTEM */
                  m_boneMorphList[id].add(bone, &pos, &rot);
               }
            }
            k++;
            *q = save;
         }
         m_hasExtParam = true;
      } else if (MMDFiles_strequal(p, "VertexMorph") || MMDFiles_strequal(p, "PmxVertexMorph")) {
         /* format 1: header, name, idx, x, y, z */
         /* format 2: header, name, local_idx, idx, x, y, z */
         unsigned long numVertexMorphAllocated = 1000;
         unsigned long id;
         unsigned long idx;
         float f[3];
         btVector3 pos;
         char *s;
         bool local_index_skipped = false;

         if (m_vertexMorphList == NULL) {
            m_vertexMorphList = new PMDVertexMorph[numVertexMorphAllocated];
            m_numVertexMorph = 0;
         }
         if (m_numVertexMorph >= numVertexMorphAllocated) {
            /* num of vertex morph exceeds limit */
            delete zf;
            return false;
         }
         k = 1;
         *q = save;
         while (*q != '\0') {
            if (*q == '"') q++;
            if (*q == '\0') break;
            p = q + 1;
            if (*p == '\0') break;
            if (*p == ',') {
               q = p;
               k++;
               continue;
            }
            if (*p == '"') p++;
            q = p;
            while (*q != '\0' && *q != ',' && *q != '"') q++;
            save = *q;
            *q = '\0';
            if (q - 1 >= p && *(q - 1) == '"') *(q - 1) = '\0';
            if (k == 1) {
               if (PMX257format)
                  s = p;
               else
                  s = MMDFiles_strdup_from_sjis_to_utf8(p);
               for (i = 0; i < m_numVertexMorph; i++) {
                  if (MMDFiles_strequal(m_vertexMorphList[i].getName(), s))
                     break;
               }
               if (i < m_numVertexMorph) {
                  id = i;
               } else {
                  id = m_numVertexMorph;
                  m_numVertexMorph++;
                  m_vertexMorphList[id].setup(s);
               }
               if (PMX257format == false)
                  free(s);
            } else if (k == 2) {
               if (PMX257format && local_index_skipped == false) {
                  /* PMX257format has extra local offset index at k = 2, skip it */
                  local_index_skipped = true;
                  *q = save;
                  continue;
               }
               idx = atoi(p);
            } else if (k >= 3 && k <= 5) {
               f[k - 3] = (float)atof(p);
               if (k == 5) {
#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
                  pos.setValue(f[0], f[1], -f[2]);
#else
                  pos.setValue(f[0], f[1], f[2]);
#endif /* MMDFILES_CONVERTCOORDINATESYSTEM */
                  m_vertexMorphList[id].add(idx, &pos);
               }
            }
            k++;
            *q = save;
         }
         m_hasExtParam = true;
      } else if (MMDFiles_strequal(p, "UVMorph") || MMDFiles_strequal(p, "PmxUVMorph")) {
         unsigned long numUVMorphAllocated = 1000;
         unsigned long id;
         unsigned long idx;
         float f[2];
         TexCoord tex;
         char *s;
         bool local_index_skipped = false;

         if (m_uvMorphList == NULL) {
            m_uvMorphList = new PMDUVMorph[numUVMorphAllocated];
            m_numUVMorph = 0;
         }
         if (m_numUVMorph >= numUVMorphAllocated) {
            /* num of uv morph exceeds limit */
            delete zf;
            return false;
         }
         k = 1;
         *q = save;
         while (*q != '\0') {
            if (*q == '"') q++;
            if (*q == '\0') break;
            p = q + 1;
            if (*p == '\0') break;
            if (*p == ',') {
               q = p;
               k++;
               continue;
            }
            if (*p == '"') p++;
            q = p;
            while (*q != '\0' && *q != ',' && *q != '"') q++;
            save = *q;
            *q = '\0';
            if (q - 1 >= p && *(q - 1) == '"') *(q - 1) = '\0';
            if (k == 1) {
               if (PMX257format)
                  s = p;
               else
                  s = MMDFiles_strdup_from_sjis_to_utf8(p);
               for (i = 0; i < m_numUVMorph; i++) {
                  if (MMDFiles_strequal(m_uvMorphList[i].getName(), s))
                     break;
               }
               if (i < m_numUVMorph) {
                  id = i;
               } else {
                  id = m_numUVMorph;
                  m_numUVMorph++;
                  m_uvMorphList[id].setup(s);
               }
               if (PMX257format == false)
                  free(s);
            } else if (k == 2) {
               if (PMX257format && local_index_skipped == false) {
                  /* PMX257format has extra local offset index at k = 2, skip it */
                  local_index_skipped = true;
                  *q = save;
                  continue;
               }
               idx = atoi(p);
            } else if (k >= 3 && k <= 4) {
               f[k - 3] = (float)atof(p);
               if (k == 4) {
                  tex.u = f[0];
                  tex.v = f[1];
                  m_uvMorphList[id].add(idx, &tex);
               }
            }
            k++;
            *q = save;
         }
         m_hasExtParam = true;
      } else if (MMDFiles_strequal(p, "MaterialMorph") || MMDFiles_strequal(p, "PmxMaterialMorph")) {
         unsigned long numMaterialMorphAllocated = 1000;
         unsigned long id;
         char *s;
         PMDMaterialMorphElem m;
         bool local_index_skipped = false;

         if (m_materialMorphList == NULL) {
            m_materialMorphList = new PMDMaterialMorph[numMaterialMorphAllocated];
            m_numMaterialMorph = 0;
         }
         if (m_numMaterialMorph >= numMaterialMorphAllocated) {
            /* num of material morph exceeds limit */
            delete zf;
            return false;
         }
         m.midx = -1;
         k = 1;
         *q = save;
         while (*q != '\0') {
            if (*q == '"') q++;
            if (*q == '\0') break;
            p = q + 1;
            if (*p == '\0') break;
            if (*p == ',') {
               q = p;
               k++;
               continue;
            }
            if (*p == '"') p++;
            q = p;
            while (*q != '\0' && *q != ',' && *q != '"') q++;
            save = *q;
            *q = '\0';
            if (q - 1 >= p && *(q - 1) == '"') *(q - 1) = '\0';
            if (k == 1) {
               if (PMX257format)
                  s = p;
               else
                  s = MMDFiles_strdup_from_sjis_to_utf8(p);
               for (i = 0; i < m_numMaterialMorph; i++) {
                  if (MMDFiles_strequal(m_materialMorphList[i].getName(), s))
                     break;
               }
               if (i < m_numMaterialMorph) {
                  id = i;
               } else {
                  id = m_numMaterialMorph;
                  m_numMaterialMorph++;
                  m_materialMorphList[id].setup(s);
               }
               if (PMX257format == false)
                  free(s);
            } else if (k == 2) {
               if (PMX257format && local_index_skipped == false) {
                  /* PMX257format has extra local offset index at k = 2, skip it */
                  local_index_skipped = true;
                  *q = save;
                  continue;
               }
               if (PMX257format)
                  s = p;
               else
                  s = MMDFiles_strdup_from_sjis_to_utf8(p);
               for (i = 0; i < m_numMaterial; i++) {
                  const char *pp;
                  pp = m_material[i].getName();
                  if (MMDFiles_strequal(pp, s)) {
                     m.midx = i;
                     break;
                  }
               }
               if (PMX257format == false)
                  free(s);
            } else if (k == 3) {
               m.addflag = (atoi(p) == 1) ? true : false;
            } else if (k >= 4 && k <= 7) {
               m.diffuse[k - 4] = (float)atof(p);
            } else if (k >= 8 && k <= 10) {
               m.specular[k - 8] = (float)atof(p);
            } else if (k == 11) {
               m.shiness = (float)atof(p);
            } else if (k >= 12 && k <= 14) {
               m.ambient[k - 12] = (float)atof(p);
            } else if (k == 15) {
               m.edgesize = (float)atof(p);
            } else if (k >= 16 && k <= 19) {
               m.edgecol[k - 16] = (float)atof(p);
            } else if (k >= 20 && k <= 23) {
               m.tex[k - 20] = (float)atof(p);
            } else if (k >= 24 && k <= 27) {
               m.sphere[k - 24] = (float)atof(p);
            } else if (k >= 28 && k <= 31) {
               m.toon[k - 28] = (float)atof(p);
            }
            k++;
            *q = save;
         }
         m_materialMorphList[id].add(&m);
         m_hasExtParam = true;
      } else if (MMDFiles_strequal(p, "GroupMorph") || MMDFiles_strequal(p, "PmxGroupMorph")) {
         unsigned long numGroupMorphAllocated = 1000;
         unsigned long id;
         char *s;
         char *morphname;
         float rate;
         bool local_index_skipped = false;

         morphname = NULL;
         if (m_groupMorphList == NULL) {
            m_groupMorphList = new PMDGroupMorph[numGroupMorphAllocated];
            m_numGroupMorph = 0;
         }
         if (m_numGroupMorph >= numGroupMorphAllocated) {
            /* num of group morph exceeds limit */
            delete zf;
            return false;
         }
         k = 1;
         *q = save;
         while (*q != '\0') {
            if (*q == '"') q++;
            if (*q == '\0') break;
            p = q + 1;
            if (*p == '\0') break;
            if (*p == ',') {
               q = p;
               k++;
               continue;
            }
            if (*p == '"') p++;
            q = p;
            while (*q != '\0' && *q != ',' && *q != '"') q++;
            save = *q;
            *q = '\0';
            if (q - 1 >= p && *(q - 1) == '"') *(q - 1) = '\0';
            if (k == 1) {
               if (PMX257format)
                  s = p;
               else
                  s = MMDFiles_strdup_from_sjis_to_utf8(p);
               for (i = 0; i < m_numGroupMorph; i++) {
                  if (MMDFiles_strequal(m_groupMorphList[i].getName(), s))
                     break;
               }
               if (i < m_numGroupMorph) {
                  id = i;
               } else {
                  id = m_numGroupMorph;
                  m_numGroupMorph++;
                  m_groupMorphList[id].setup(s);
               }
               if (PMX257format == false)
                  free(s);
            } else if (k == 2) {
               if (PMX257format && local_index_skipped == false) {
                  /* PMX257format has extra local offset index at k = 2, skip it */
                  local_index_skipped = true;
                  *q = save;
                  continue;
               }
               if (PMX257format)
                  morphname = MMDFiles_strdup(p);
               else
                  morphname = MMDFiles_strdup_from_sjis_to_utf8(p);
            } else if (k == 3) {
               rate = (float)atof(p);
               if (morphname != NULL) {
                  m_groupMorphList[id].add(morphname, rate);
                  free(morphname);
                  morphname = NULL;
               }
            }
            k++;
            *q = save;
         }
         if (morphname) {
            free(morphname);
            morphname = NULL;
         }
         m_hasExtParam = true;
      } else if (MMDFiles_strequal(p, "Bone") || MMDFiles_strequal(p, "PmxBone")) {
         char *s;
         PMDBone *bone;
         PMDBone *boneTarget;
         bool afterPhysics;
         short processLayer;
         bool rotate_add;
         bool move_add;
         float follow_rate;

         m_hasExtBoneParam = true;

         bone = NULL;
         afterPhysics = false;
         processLayer = 0;
         rotate_add = false;
         move_add = false;
         follow_rate = 0.0f;
         k = 1;
         *q = save;
         while (*q != '\0') {
            if (*q == '"') q++;
            if (*q == '\0') break;
            p = q + 1;
            if (*p == '\0') break;
            if (*p == ',') {
               q = p;
               k++;
               continue;
            }
            if (*p == '"') p++;
            q = p;
            while (*q != '\0' && *q != ',' && *q != '"') q++;
            save = *q;
            *q = '\0';
            if (q - 1 >= p && *(q - 1) == '"') *(q - 1) = '\0';
            if (k == 1) {
               if (PMX257format)
                  s = p;
               else
                  s = MMDFiles_strdup_from_sjis_to_utf8(p);
               bone = getBone(s);
               if (PMX257format == false)
                  free(s);
            } else if (k == 3) {
               processLayer = (short)atoi(p);
            } else if (k == 4) {
               afterPhysics = (atoi(p) == 0 ? false : true);
            } else if ((PMX2Recent == true && k == 20) || (PMX2Recent == false && k == 19)) {
               if (atoi(p) == 1)
                  rotate_add = true;
            } else if ((PMX2Recent == true && k == 21) || (PMX2Recent == false && k == 20)) {
               if (atoi(p) == 1)
                  move_add = true;
            } else if ((PMX2Recent == true && k == 22) || (PMX2Recent == false && k == 21)) {
               follow_rate = (float)atof(p);
            } else if ((PMX2Recent == true && k == 23) || (PMX2Recent == false && k == 22)) {
               if (PMX257format) {
                  boneTarget = getBone(p);
               } else {
                  s = MMDFiles_strdup_from_sjis_to_utf8(p);
                  boneTarget = getBone(s);
                  free(s);
               }
            }
            k++;
            *q = save;
         }
         if (bone) {
            if (afterPhysics) {
               bone->enableProcessingAfterPhysics();
            } else {
               bone->disableProcessingAfterPhysics();
            }
            bone->setProcessLayer(processLayer);
            if (rotate_add) {
               bone->setFollowRotate(follow_rate, boneTarget);
            }
            if (move_add) {
               bone->setFollowMove(follow_rate, boneTarget);
            }
         }
         m_hasExtParam = true;
      } else if (MMDFiles_strequal(p, "Face") || MMDFiles_strequal(p, "PmxFace")) {
         /* assume surfaces are in material order */
         /* assume surface indices are successive */
         INDICES indices[3];
         k = 1;
         *q = save;
         while (*q != '\0') {
            if (*q == '"') q++;
            if (*q == '\0') break;
            p = q + 1;
            if (*p == '\0') break;
            if (*p == ',') {
               q = p;
               k++;
               continue;
            }
            if (*p == '"') p++;
            q = p;
            while (*q != '\0' && *q != ',' && *q != '"') q++;
            save = *q;
            *q = '\0';
            if (q - 1 >= p && *(q - 1) == '"') *(q - 1) = '\0';
            if (k == 2) {
               /* surface id per material (not used) */
            } else if (k >= 3 && k <= 5) {
               indices[k - 3] = (INDICES)atoi(p);
            }
            k++;
            *q = save;
         }
         memcpy(&(m_surfaceList[surface_idx * 3]), indices, sizeof(INDICES) * 3);
         surface_idx++;
         m_hasExtParam = true;
      }
   }

   if (surface_idx > 0 && surface_idx * 3 != m_numSurface) {
      return false;
   }

   /* make name index */
   for (unsigned short j = 0; j < m_numBoneMorph; j++) {
      char *name = m_boneMorphList[j].getName();
      if (name) m_name2bonemorph.add(name, (int)strlen(name), &(m_boneMorphList[j]));
   }
   for (unsigned short j = 0; j < m_numVertexMorph; j++) {
      char *name = m_vertexMorphList[j].getName();
      if (name) m_name2vertexmorph.add(name, (int)strlen(name), &(m_vertexMorphList[j]));
   }
   for (unsigned short j = 0; j < m_numUVMorph; j++) {
      char *name = m_uvMorphList[j].getName();
      if (name) m_name2uvmorph.add(name, (int)strlen(name), &(m_uvMorphList[j]));
   }
   for (unsigned short j = 0; j < m_numMaterialMorph; j++) {
      char *name = m_materialMorphList[j].getName();
      if (name) m_name2materialmorph.add(name, (int)strlen(name), &(m_materialMorphList[j]));
   }
   for (unsigned short j = 0; j < m_numGroupMorph; j++) {
      char *name = m_groupMorphList[j].getName();
      if (name) m_name2groupmorph.add(name, (int)strlen(name), &(m_groupMorphList[j]));
   }
   /* serialize data */
   for (unsigned short j = 0; j < m_numVertexMorph; j++)
      m_vertexMorphList[j].serialize();
   for (unsigned short j = 0; j < m_numUVMorph; j++)
      m_uvMorphList[j].serialize();

   if (m_groupMorphList) {
      /* set target morph link */
      PMDGroupMorphElem *p;
      unsigned short i;

      for (i = 0; i < m_numGroupMorph; i++) {
         m_groupMorphList[i].setHasBoneFlag(false);
         m_groupMorphList[i].setHasUVFlag(false);
         for (p = m_groupMorphList[i].getList(); p; p = p->next) {
            p->b = getBoneMorph(p->name);
            p->v = getVertexMorph(p->name);
            p->u = getUVMorph(p->name);
            p->m = getMaterialMorph(p->name);
            if (p->b)
               m_groupMorphList[i].setHasBoneFlag(true);
            if (p->u)
               m_groupMorphList[i].setHasUVFlag(true);
         }
      }
   }

   delete zf;
   return true;
}
