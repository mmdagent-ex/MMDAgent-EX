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
#ifdef __ANDROID__
#include <sys/system_properties.h>
#endif /* __ANDROID__ */

/* compareDepth: qsort function for reordering */
static int compareDepth(const void *a, const void *b)
{
   RenderDepthData *x = (RenderDepthData *) a;
   RenderDepthData *y = (RenderDepthData *) b;

   if (x->dist == y->dist)
      return 0;
   return ( (x->dist > y->dist) ? 1 : -1 );
}

/* Render::updateProjectionMatrix: update view information */
void Render::updateProjectionMatrix()
{
   glViewport(0, 0, m_width, m_height);

   /* camera setting */
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   applyProjectionMatrix(m_viewPointFrustumNear, RENDER_VIEWPOINTFRUSTUMFAR);
   glMatrixMode(GL_MODELVIEW);
}

/* Render::applyProjectionMatirx: update projection matrix */
void Render::applyProjectionMatrix(double nearVal, double farVal)
{
   double y = tan(MMDFILES_RAD(m_currentFovy) * 0.5) * nearVal;
   double x = y * m_width / m_height;
   glFrustum(-x, x, -y, y, nearVal, farVal);
}

/* Render::updateModelViewMatrix: update model view matrix */
void Render::updateModelViewMatrix()
{
   m_transMatrix.setIdentity();
   m_transMatrix.setRotation(m_currentRot);
   m_transMatrix.setOrigin(m_transMatrix * (-m_currentTrans) - btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(m_currentDistance)));
   if (m_baseBone) {
      if (m_modelFollow) {
         btTransform tr = m_baseBone->getTransform()->inverse();
         btVector3 pos = tr.getOrigin();
         pos.setY(0.0);
         pos.setZ(0.0);
         pos = m_lastFollowPos.lerp(pos, btScalar(0.01f));
         m_lastFollowPos = pos;
         btTransform tr2 = btTransform();
         tr2.setIdentity();
         tr2.setOrigin(pos);
         m_transMatrix *= tr2;
      } else {
         m_transMatrix *= m_baseBone->getTransform()->inverse();
      }
   }
   m_transMatrixInv = m_transMatrix.inverse();
   m_transMatrix.getOpenGLMatrix(m_rotMatrix);
   m_transMatrixInv.getOpenGLMatrix(m_rotMatrixInv);
}

/* Render::updateTransRotMatrix:  update trans and rotation matrix */
bool Render::updateTransRotMatrix(double ellapsedTimeForMove)
{
   float diff1, diff2;
   btVector3 trans;
   btQuaternion rot;

   /* if no difference, return */
   if (m_currentRot == m_rot && m_currentTrans == m_trans)
      return false;

   if (m_viewMoveTime == 0.0 || m_viewControlledByMotion == true) {
      /* immediately apply the target */
      m_currentRot = m_rot;
      m_currentTrans = m_trans;
   } else if (m_viewMoveTime > 0.0) {
      /* constant move */
      if (ellapsedTimeForMove >= m_viewMoveTime) {
         m_currentRot = m_rot;
         m_currentTrans = m_trans;
      } else {
         m_currentTrans = m_viewMoveStartTrans.lerp(m_trans, btScalar(ellapsedTimeForMove / m_viewMoveTime));
         m_currentRot = m_viewMoveStartRot.slerp(m_rot, btScalar(ellapsedTimeForMove / m_viewMoveTime));
      }
   } else {
      /* calculate difference */
      trans = m_trans;
      trans -= m_currentTrans;
      diff1 = trans.length2();
      rot = m_rot;
      rot -= m_currentRot;
      diff2 = rot.length2();

      if (diff1 > RENDER_MINMOVEDIFF)
         m_currentTrans = m_currentTrans.lerp(m_trans, btScalar(1.0f - RENDER_MOVESPEEDRATE)); /* current * 0.9 + target * 0.1 */
      else
         m_currentTrans = m_trans;
      if (diff2 > RENDER_MINSPINDIFF)
         m_currentRot = m_currentRot.slerp(m_rot, btScalar(1.0f - RENDER_SPINSPEEDRATE)); /* current * 0.9 + target * 0.1 */
      else
         m_currentRot = m_rot;
   }

   return true;
}

/* Render::updateRotationFromAngle: update rotation quaternion from angle */
void Render::updateRotationFromAngle()
{
   m_rot = btQuaternion(btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(1.0f)), btScalar(MMDFILES_RAD(m_angle.z())))
           * btQuaternion(btVector3(btScalar(1.0f), btScalar(0.0f), btScalar(0.0f)), btScalar(MMDFILES_RAD(m_angle.x())))
           * btQuaternion(btVector3(btScalar(0.0f), btScalar(1.0f), btScalar(0.0f)), btScalar(MMDFILES_RAD(m_angle.y())));
}

/* Render::updateDistance: update distance */
bool Render::updateDistance(double ellapsedTimeForMove)
{
   float diff;

   /* if no difference, return */
   if (m_currentDistance == m_distance)
      return false;

   if (m_viewMoveTime == 0.0 || m_viewControlledByMotion == true) {
      /* immediately apply the target */
      m_currentDistance = m_distance;
   } else if (m_viewMoveTime > 0.0) {
      /* constant move */
      if (ellapsedTimeForMove >= m_viewMoveTime) {
         m_currentDistance = m_distance;
      } else {
         m_currentDistance = m_viewMoveStartDistance + (m_distance - m_viewMoveStartDistance) * (float)(ellapsedTimeForMove / m_viewMoveTime);
      }
   } else {
      diff = (float)fabs(m_currentDistance - m_distance);
      if (diff < RENDER_MINDISTANCEDIFF) {
         m_currentDistance = m_distance;
      } else {
         m_currentDistance = m_currentDistance * (RENDER_DISTANCESPEEDRATE) + m_distance * (1.0f - RENDER_DISTANCESPEEDRATE);
      }
   }

   return true;
}

/* Render::updateFovy: update fovy */
bool Render::updateFovy(double ellapsedTimeForMove)
{
   float diff;

   /* if no difference, return */
   if (m_currentFovy == m_fovy)
      return false;

   if (m_viewMoveTime == 0.0 || m_viewControlledByMotion == true) {
      /* immediately apply the target */
      m_currentFovy = m_fovy;
   } else if (m_viewMoveTime > 0.0) {
      /* constant move */
      if (ellapsedTimeForMove >= m_viewMoveTime) {
         m_currentFovy = m_fovy;
      } else {
         m_currentFovy = m_viewMoveStartFovy + (m_fovy - m_viewMoveStartFovy) * (float)(ellapsedTimeForMove / m_viewMoveTime);
      }
   } else {
      diff = (float)fabs(m_currentFovy - m_fovy);
      if (diff < RENDER_MINFOVYDIFF) {
         m_currentFovy = m_fovy;
      } else {
         m_currentFovy = m_currentFovy * (RENDER_FOVYSPEEDRATE) + m_fovy * (1.0f - RENDER_FOVYSPEEDRATE);
      }
   }

   return true;
}

/* Render::initializeShadowMap: initialize OpenGL for shadow mapping */
void Render::initializeShadowMap(int textureSize)
{
#ifndef MMDAGENT_DONTUSESHADOWMAP
   static const GLdouble genfunc[][4] = {
      { 1.0, 0.0, 0.0, 0.0 },
      { 0.0, 1.0, 0.0, 0.0 },
      { 0.0, 0.0, 1.0, 0.0 },
      { 0.0, 0.0, 0.0, 1.0 },
   };

   /* initialize model view matrix */
   glPushMatrix();
   glLoadIdentity();

   /* use 4th texture unit for depth texture, make it current */
   glActiveTexture(GL_TEXTURE3);

   /* prepare a texture object for depth texture rendering in frame buffer object */
   glGenTextures(1, &m_depthTextureID);
   glBindTexture(GL_TEXTURE_2D, m_depthTextureID);

   /* assign depth component to the texture */
   glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, textureSize, textureSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

   /* set texture parameters for shadow mapping */
#ifdef RENDER_SHADOWPCF
   /* use hardware PCF */
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#else
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#endif /* RENDER_SHADOWPCF */

   /* tell OpenGL to compare the R texture coordinates to the (depth) texture value */
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

   /* also tell OpenGL to get the compasiron result as alpha value */
   glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_ALPHA);

   /* set texture coordinates generation mode to use the raw texture coordinates (S, T, R, Q) in eye view */
   glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
   glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
   glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
   glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
   glTexGendv(GL_S, GL_EYE_PLANE, genfunc[0]);
   glTexGendv(GL_T, GL_EYE_PLANE, genfunc[1]);
   glTexGendv(GL_R, GL_EYE_PLANE, genfunc[2]);
   glTexGendv(GL_Q, GL_EYE_PLANE, genfunc[3]);

   /* finished configuration of depth texture: unbind the texture */
   glBindTexture(GL_TEXTURE_2D, 0);

   /* allocate a frame buffer object (FBO) for depth buffer rendering */
   glGenFramebuffers(1, &m_fboID);
   /* switch to the newly allocated FBO */
   glBindFramebuffer(GL_FRAMEBUFFER_EXT, m_fboID);
   /* bind the texture to the FBO, telling that it should render the depth information to the texture */
   glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, m_depthTextureID, 0);
   /* also tell OpenGL not to draw and read the color buffers */
   glDrawBuffer(GL_NONE);
   glReadBuffer(GL_NONE);
   /* check FBO status */
   if (glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT) {
      /* cannot use FBO */
   }
   /* finished configuration of FBO, now switch to default frame buffer */
   glBindFramebuffer(GL_FRAMEBUFFER_EXT, m_defaultFrameBuffer);

   /* reset the current texture unit to default */
   glActiveTexture(GL_TEXTURE0);

   /* restore the model view matrix */
   glPopMatrix();
#endif /* !MMDAGENT_DONTUSESHADOWMAP */
}

/* Render::renderSceneShadowMap: shadow mapping */
void Render::renderSceneShadowMap(PMDObject *objs, const int *order, int num, Stage *stage, bool useMMDLikeCartoon, bool useCartoonRendering, float lightIntensity, const float *lightDirection, const float *lightColor, int shadowMappingTextureSize, float shadowMappingSelfDensity)
{
#ifndef MMDAGENT_DONTUSESHADOWMAP
   short i;
   GLint viewport[4]; /* store viewport */
   GLdouble modelview[16]; /* store model view transform */
   GLdouble projection[16]; /* store projection transform */
   bool toonLight = true;

#ifdef RENDER_SHADOWAUTOVIEW
   float eyeDist;
   btVector3 v;
#endif /* RENDER_SHADOWAUTOVIEW */

   static GLfloat lightdim[] = { 0.2f, 0.2f, 0.2f, 1.0f };
   static const GLfloat lightblk[] = { 0.0f, 0.0f, 0.0f, 1.0f };

   /* render the depth texture */
   /* store the current viewport */
   glGetIntegerv(GL_VIEWPORT, viewport);

   /* store the current projection matrix */
   glGetDoublev(GL_PROJECTION_MATRIX, projection);

   /* switch to FBO for depth buffer rendering */
   glBindFramebuffer(GL_FRAMEBUFFER_EXT, m_fboID);

   /* clear the buffer */
   /* clear only the depth buffer, since other buffers will not be used */
   glClear(GL_DEPTH_BUFFER_BIT);

   /* set the viewport to the required texture size */
   glViewport(0, 0, shadowMappingTextureSize, shadowMappingTextureSize);

   /* reset the projection matrix */
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   /* set the model view matrix to make the light position as eye point and capture the whole scene in the view */
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

#ifdef RENDER_SHADOWAUTOVIEW
   /* set the distance to cover all the model range */
   eyeDist = m_shadowMapAutoViewRadius / sinf(RENDER_SHADOWAUTOVIEWANGLE * 0.5f * 3.1415926f / 180.0f);
   /* set the perspective */
   gluPerspective(RENDER_SHADOWAUTOVIEWANGLE, 1.0, 1.0, eyeDist + m_shadowMapAutoViewRadius + 50.0f); /* +50.0f is needed to cover the background */
   /* the viewpoint should be at eyeDist far toward light direction from the model center */
   v = m_lightVec * eyeDist + m_shadowMapAutoViewEyePoint;
   gluLookAt(v.x(), v.y(), v.z(), m_shadowMapAutoViewEyePoint.x(), m_shadowMapAutoViewEyePoint.y(), m_shadowMapAutoViewEyePoint.z(), 0.0, 1.0, 0.0);
#else
   /* fixed view */
   gluPerspective(25.0, 1.0, 1.0, 120.0);
   gluLookAt(30.0, 77.0, 30.0, 0.0, 17.0, 0.0, 0.0, 1.0, 0.0);
#endif /* RENDER_SHADOWAUTOVIEW */

   /* keep the current model view for later process */
   glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

   /* do not write into frame buffer other than depth information */
   glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

   /* also, lighting is not needed */
   glDisable(GL_LIGHTING);

   /* disable rendering the front surface to get the depth of back face */
   glCullFace(GL_FRONT);

   /* disable alpha test */
   glDisable(GL_ALPHA_TEST);

   /* we are now writing to depth texture using FBO, so disable the depth texture mapping here */
   glActiveTexture(GL_TEXTURE3);
   glDisable(GL_TEXTURE_2D);
   glActiveTexture(GL_TEXTURE0);

   /* set polygon offset to avoid "moire" */
   glEnable(GL_POLYGON_OFFSET_FILL);
   glPolygonOffset(4.0f, 4.0f);

   /* render objects for depth */
   /* only objects that wants to drop shadow should be rendered here */
   for (i = 0; i < num; i++) {
      if (objs[order[i]].isEnable() == true) {
         objs[order[i]].getPMDModel()->renderForShadowMap();
      }
   }

   /* reset the polygon offset */
   glDisable(GL_POLYGON_OFFSET_FILL);

   /* switch to default FBO */
   glBindFramebuffer(GL_FRAMEBUFFER_EXT, m_defaultFrameBuffer);

   /* revert configurations to normal rendering */
   glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
   glMatrixMode(GL_PROJECTION);
   glLoadMatrixd(projection);
   glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
   glEnable(GL_LIGHTING);
   glCullFace(GL_BACK);
   glEnable(GL_ALPHA_TEST);

   /* clear all the buffers */
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   /* render the full scene */
   /* set model view matrix, as the same as normal rendering */
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glMultMatrixf(m_rotMatrix);

   /* render the whole scene */

   /* render light setting, later render only the shadow part with dark setting */
   if (m_backgroundForTransparent == false)
      stage->renderBackground();
   /* if stage is larger than frustum far, render it with large frustum, disgarding depth */
   if (stage->getRange() > RENDER_VIEWPOINTFRUSTUMFAR) {
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      applyProjectionMatrix(2.0, stage->getRange());
      glMatrixMode(GL_MODELVIEW);
      stage->renderFloor();
      /* restore projection matrix */
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
      /* clear depth buffer */
      glClear(GL_DEPTH_BUFFER_BIT);
      /* then render only the depth buffer again with normal frustum */
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
      stage->renderFloor();
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
   } else {
      stage->renderFloor();
   }
   /* render doppel shadow if enabled */
   if (m_doppelShadowFlag) {
      glDisable(GL_DEPTH_TEST);
      glDisable(GL_LIGHTING);
      glColor4f(m_doppelShadowColor[0], m_doppelShadowColor[1], m_doppelShadowColor[2], m_backgroundForTransparent ? m_backgroundTransparentColor[3] : 1.0f);
      glPushMatrix();
      glLoadIdentity();
      glTranslatef(m_doppelShadowOffset[0], m_doppelShadowOffset[1], m_doppelShadowOffset[2]);
      glMultMatrixf(m_rotMatrix);
      for (i = 0; i < num; i++) {
         if (objs[order[i]].isEnable() == true) {
            objs[order[i]].getPMDModel()->renderForShadow();
         }
      }
      glPopMatrix();
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_LIGHTING);
   }
   for (i = 0; i < num; i++) {
      if (objs[order[i]].isEnable() == true) {
         if (objs[order[i]].getPMDModel()->getToonFlag() == false && toonLight == true) {
            /* disable toon lighting */
            updateLight(true, false, lightIntensity, lightDirection, lightColor);
            toonLight = false;
         } else if (objs[order[i]].getPMDModel()->getToonFlag() == true && toonLight == false) {
            /* enable toon lighting */
            updateLight(useMMDLikeCartoon, useCartoonRendering, lightIntensity, lightDirection, lightColor);
            toonLight = true;
         }
         objs[order[i]].getPMDModel()->renderModel(true);
      }
   }
   if (toonLight == false) {
      /* restore toon lighting */
      updateLight(useMMDLikeCartoon, useCartoonRendering, lightIntensity, lightDirection, lightColor);
   }

   /* render the part clipped by the depth texture */
   /* activate the texture unit for shadow mapping and make it current */
   glActiveTexture(GL_TEXTURE3);

   /* set texture matrix (note: matrices should be set in reverse order) */
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   /* move the range from [-1,1] to [0,1] */
   glTranslatef(0.5f, 0.5f, 0.5f);
   glScalef(0.5f, 0.5f, 0.5f);
   /* multiply the model view matrix when the depth texture was rendered */
   glMultMatrixd(modelview);
   /* multiply the inverse matrix of current model view matrix */
   glMultMatrixf(m_rotMatrixInv);

   /* revert to model view matrix mode */
   glMatrixMode(GL_MODELVIEW);

   /* enable texture mapping with texture coordinate generation */
   glEnable(GL_TEXTURE_2D);
   glEnable(GL_TEXTURE_GEN_S);
   glEnable(GL_TEXTURE_GEN_T);
   glEnable(GL_TEXTURE_GEN_R);
   glEnable(GL_TEXTURE_GEN_Q);

   /* bind the depth texture rendered at the first step */
   glBindTexture(GL_TEXTURE_2D, m_depthTextureID);

   /* depth texture set up was done, now switch current texture unit to default */
   glActiveTexture(GL_TEXTURE0);

#ifdef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
   /* set depth func to allow overwrite for the same surface in the following rendering */
   glDepthFunc(GL_LEQUAL);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */

   /* the area clipped by depth texture by alpha test is dark part */
   glAlphaFunc(GL_GEQUAL, 0.1f);

   /* light setting for non-toon objects */
   lightdim[0] = lightdim[1] = lightdim[2] = 0.34f - 0.13f * shadowMappingSelfDensity;
   glLightfv(GL_LIGHT0, GL_DIFFUSE, lightdim);
   glLightfv(GL_LIGHT0, GL_AMBIENT, lightdim);
   glLightfv(GL_LIGHT0, GL_SPECULAR, lightblk);

   /* not modify alpha value of target since we just blend color, not alpha */
   glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

   /* render the non-toon objects (back, floor, non-toon models) */
   if (!stage->getPMD() || stage->getPMD()->getToonFlag() == false) {
      if (m_backgroundForTransparent == false)
         stage->renderBackground();
      if (!m_doppelShadowFlag)
         // avoid drawing floor/stage shadow when doppel shadow is enabled
         stage->renderFloor();
   }
   for (i = 0; i < num; i++) {
      if (objs[order[i]].isEnable() == true && objs[order[i]].getPMDModel()->getToonFlag() == false)
         objs[order[i]].getPMDModel()->renderModel(false);
   }

   /* for toon objects, they should apply the model-defined toon texture color at texture coordinates (0, 0) for shadow rendering */
   /* so restore the light setting */
   if (useCartoonRendering == true)
      updateLight(useMMDLikeCartoon, useCartoonRendering, lightIntensity, lightDirection, lightColor);
   /* render the toon objects */
   if (stage->getPMD() && stage->getPMD()->getToonFlag() == true) {
      if (m_backgroundForTransparent == false)
         stage->renderBackground();
      if (!m_doppelShadowFlag)
         // avoid drawing floor/stage shadow when doppel shadow is enabled
         stage->renderFloor();
   }
   for (i = 0; i < num; i++) {
      if (objs[order[i]].isEnable() == true && objs[order[i]].getPMDModel()->getToonFlag() == true) {
         /* set texture coordinates for shadow mapping */
         objs[order[i]].getPMDModel()->updateShadowColorTexCoord(shadowMappingSelfDensity);
         /* tell model to render with the shadow corrdinates */
         objs[order[i]].getPMDModel()->setSelfShadowDrawing(true);
         /* render model and edge */
         objs[order[i]].getPMDModel()->renderModel(false);
         /* disable shadow rendering */
         objs[order[i]].getPMDModel()->setSelfShadowDrawing(false);
      }
   }
   if (useCartoonRendering == false)
      updateLight(useMMDLikeCartoon, useCartoonRendering, lightIntensity, lightDirection, lightColor);

   /* reset settings */
   glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
#ifdef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
   glDepthFunc(GL_LESS);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */
   glAlphaFunc(GL_GREATER, 0.0f);

   glActiveTexture(GL_TEXTURE3);
   glDisable(GL_TEXTURE_GEN_S);
   glDisable(GL_TEXTURE_GEN_T);
   glDisable(GL_TEXTURE_GEN_R);
   glDisable(GL_TEXTURE_GEN_Q);
   glDisable(GL_TEXTURE_2D);
   glActiveTexture(GL_TEXTURE0);
#endif /* !MMDAGENT_DONTUSESHADOWMAP */
}

/* Render::renderScene: render scene */
void Render::renderScene(PMDObject *objs, const int *order, int num, Stage *stage, bool useMMDLikeCartoon, bool useCartoonRendering, float lightIntensity, const float *lightDirection, const float *lightColor, float shadowDensity)
{
   short i;
   bool toonLight = true;

   /* clear rendering buffer */
   if (m_backgroundForTransparent) {
      glClearColor(m_backgroundTransparentColor[0], m_backgroundTransparentColor[1], m_backgroundTransparentColor[2], m_backgroundTransparentColor[3]);
   } else {
      glClearColor(m_backgroundColor[0], m_backgroundColor[1], m_backgroundColor[2], 1.0f);
   }
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glEnable(GL_CULL_FACE);
   glEnable(GL_BLEND);

   /* set model viwe matrix */
   glLoadIdentity();
   glMultMatrixf(m_rotMatrix);

   /* stage and shadow */

   /* background */
   if (m_backgroundForTransparent == false)
      stage->renderBackground();
   /* floor */
   /* if stage is larger than frustum far, render it with large frustum, disgarding depth */
   if (stage->getRange() > RENDER_VIEWPOINTFRUSTUMFAR) {
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      applyProjectionMatrix(2.0, stage->getRange());
      glMatrixMode(GL_MODELVIEW);
      stage->renderFloor();
      /* restore projection matrix */
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
      /* clear depth buffer */
      glClear(GL_DEPTH_BUFFER_BIT);
      /* then render only the depth buffer again with normal frustum */
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
      stage->renderFloor();
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
   } else {
      stage->renderFloor();
   }
   if (m_doppelShadowFlag) {
      glDisable(GL_DEPTH_TEST);
      glDisable(GL_LIGHTING);
      glColor4f(m_doppelShadowColor[0], m_doppelShadowColor[1], m_doppelShadowColor[2], m_backgroundForTransparent ? m_backgroundTransparentColor[3] : 1.0f);
      glPushMatrix();
      glLoadIdentity();
      glTranslatef(m_doppelShadowOffset[0], m_doppelShadowOffset[1], m_doppelShadowOffset[2]);
      glMultMatrixf(m_rotMatrix);
      for (i = 0; i < num; i++) {
         if (objs[order[i]].isEnable() == true) {
            objs[order[i]].getPMDModel()->renderForShadow();
         }
      }
      glPopMatrix();
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_LIGHTING);
   }
   if (m_useShadow && ! m_doppelShadowFlag) { // avoid drawing floor/stage shadow when doppel shadow is enabled
      /* enable writing stencil bits */
      glStencilMask(0xFFFF);
      /* clear stencil buffer */
      glClear(GL_STENCIL_BUFFER_BIT);
      /* enable stencil test */
      glEnable(GL_STENCIL_TEST);
      /* render only stencil */
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
      glDepthMask(GL_FALSE);
      /* set stencil function to allow writing to all pixels */
      glStencilFunc(GL_ALWAYS, 1, ~0);
      /* set stencil operation to set 1 to written pixel */
      glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
      /* set depth function */
#ifdef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
      glDepthFunc(GL_LEQUAL);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(-1.0f, -1.0f);
      /* render model shadow */
      glPushMatrix();
      glMultMatrixf(stage->getShadowMatrix());
      for (i = 0; i < num; i++) {
         if (objs[order[i]].isEnable() == true) {
            objs[order[i]].getPMDModel()->renderForShadow();
         }
      }
      glPopMatrix();
      glDisable(GL_POLYGON_OFFSET_FILL);
      /* allow to write color pixels */
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      /* if stencil is 1, render shadow with blend on */
      glStencilFunc(GL_EQUAL, 1, ~0);
      /* disable writing stencil bits */
      glStencilMask(0x0);
      glDisable(GL_DEPTH_TEST);
      glDisable(GL_LIGHTING);
      glColor4f(shadowDensity, shadowDensity, shadowDensity, m_backgroundForTransparent ? m_backgroundTransparentColor[3] : 1.0f);
      stage->renderFloor();
      glEnable(GL_DEPTH_TEST);
      glDepthMask(GL_TRUE);
#ifdef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
      glDepthFunc(GL_LESS);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */
      glDisable(GL_STENCIL_TEST);
      glEnable(GL_LIGHTING);
   }

   /* render model */
   for (i = 0; i < num; i++) {
      if (objs[order[i]].isEnable() == true) {
         if (objs[order[i]].getPMDModel()->getToonFlag() == false && toonLight == true) {
            /* disable toon lighting */
            updateLight(true, false, lightIntensity, lightDirection, lightColor);
            toonLight = false;
         } else if (objs[order[i]].getPMDModel()->getToonFlag() == true && toonLight == false) {
            /* enable toon lighting */
            updateLight(useMMDLikeCartoon, useCartoonRendering, lightIntensity, lightDirection, lightColor);
            toonLight = true;
         }
         objs[order[i]].getPMDModel()->renderModel(true);
      }
   }
   if (toonLight == false) {
      /* restore toon lighting */
      updateLight(useMMDLikeCartoon, useCartoonRendering, lightIntensity, lightDirection, lightColor);
   }
}

#ifdef MY_LUMINOUS

#define TEXTURE_DIV_FACTOR 1 // texture size for bloom rendering, 1/N of screen size

/* Render::initBloomRendering: initialize for bloom effect rendering */
void Render::initBloomRendering()
{
#ifndef MMDAGENT_DONTUSELUMINOUS
   if (m_bloomFboID != LUMINOUS_FBO_UNINITIALIZEDID)
      return;

   /* allocate a frame buffer object (FBO) for bloom rendering */
   glGenFramebuffers(1, &m_bloomFboID);

   /* prepare texture and render buffer */
   updateBloomRendering();
#endif /* MMDAGENT_DONTUSELUMINOUS */
}

/* Render::clearBloomRendering: clear bloom effect rendering */
void Render::clearBloomRendering()
{
#ifndef MMDAGENT_DONTUSELUMINOUS
   if (m_bloomFboID == LUMINOUS_FBO_UNINITIALIZEDID)
      return;

   /* delete textures and buffers */
   glDeleteTextures(1, &m_bloomTextureID);
   glDeleteRenderbuffers(1, &m_bloomRboID);
   glDeleteFramebuffers(1, &m_bloomFboID);

   m_bloomFboID = LUMINOUS_FBO_UNINITIALIZEDID;
   m_bloomTextureID = PMDTEXTURE_UNINITIALIZEDID;
#endif /* MMDAGENT_DONTUSELUMINOUS */
}

/* Render::updateBloomRendering: update texture for bloom effect rendering */
void Render::updateBloomRendering()
{
#ifndef MMDAGENT_DONTUSELUMINOUS
   int textureWidth, textureHeight;

   if (m_bloomFboID == LUMINOUS_FBO_UNINITIALIZEDID)
      return;

   /* set texture size */
   textureWidth = m_width / TEXTURE_DIV_FACTOR;
   textureHeight = m_height / TEXTURE_DIV_FACTOR;
   if (m_bloomTextureID != PMDTEXTURE_UNINITIALIZEDID && m_bloomTextureWidth == textureWidth && m_bloomTextureHeight == textureHeight)
      /* unchanged */
      return;

   /* if the bloom texture and render buffer is already assigned, delete them */
   if (m_bloomTextureID != PMDTEXTURE_UNINITIALIZEDID) {
      glDeleteTextures(1, &m_bloomTextureID);
      glDeleteRenderbuffers(1, &m_bloomRboID);
   }

   /* prepare a texture object for bloom rendering to be written by the frame buffer object */
   glGenTextures(1, &m_bloomTextureID);
   glBindTexture(GL_TEXTURE_2D, m_bloomTextureID);

   /* assign color texture component to the texture with NULL data */
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

   /* establish a mipmap chain for the texture */
   glGenerateMipmap(GL_TEXTURE_2D);

   /* set texture parameters */
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

   /* finished configuration of bloom texture: unbind the texture */
   glBindTexture(GL_TEXTURE_2D, 0);

   /* switch to the newly allocated FBO */
   glBindFramebuffer(GL_FRAMEBUFFER, m_bloomFboID);

   /* bind the texture to the FBO, telling that it should render color information to the texture */
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_bloomTextureID, 0);

   /* also prepare a render object and assign it to the FBO for depth-enabled rendering */
   glGenRenderbuffers(1, &m_bloomRboID);
   glBindRenderbuffer(GL_RENDERBUFFER, m_bloomRboID);
   glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, textureWidth, textureHeight);
   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_bloomRboID);

   /* check FBO status */
   if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      /* cannot use FBO */
   }

   /* finished configuration of the FBO, now switch to default frame buffer */
   glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFrameBuffer);

   m_bloomTextureWidth = textureWidth;
   m_bloomTextureHeight = textureHeight;
#endif /* MMDAGENT_DONTUSELUMINOUS */
}

/* Render::renderBloomTexture: render bloom texture */
void Render::renderBloomTexture(PMDObject *objs, const int *order, int num, Stage *stage, bool useMMDLikeCartoon, bool useCartoonRendering, float lightIntensity, const float *lightDirection, const float *lightColor)
{
#ifndef MMDAGENT_DONTUSELUMINOUS
   GLdouble projection[16];
   short i;
   bool toonLight = true;
   PMDModel *pmd;
   const float defaultLightColor[3] = { 1.0f, 1.0f, 1.0f };

   if (m_bloomFboID == LUMINOUS_FBO_UNINITIALIZEDID)
      return;

   /* save the current projection matrix */
   glGetDoublev(GL_PROJECTION_MATRIX, projection);

   /* if stage is larger than frustum far, render it with large frustum */
   if (stage->getRange() > RENDER_VIEWPOINTFRUSTUMFAR) {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      applyProjectionMatrix(2.0, stage->getRange());
      glMatrixMode(GL_MODELVIEW);
   }

   /* set the viewport to the required texture size */
   /* keeping ratio with the screen buffer size, so the projection matrix need not be changed */
   glViewport(0, 0, m_bloomTextureWidth, m_bloomTextureHeight);

   /* switch to FBO for bloom rendering */
   glBindFramebuffer(GL_FRAMEBUFFER, m_bloomFboID);

   /* clear rendering buffer in black */
   glClearColor(0.0f, 0.0f, 0.0f, m_backgroundForTransparent ? m_backgroundTransparentColor[3] : 1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   if (m_backgroundForTransparent) {
      glClearColor(m_backgroundTransparentColor[0], m_backgroundTransparentColor[1], m_backgroundTransparentColor[2], m_backgroundTransparentColor[3]);
   } else {
      glClearColor(m_backgroundColor[0], m_backgroundColor[1], m_backgroundColor[2], 1.0f);
   }

   /* render depth information for bloom */
   /* write only depth information */
   glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
   /* disable alpha test */
   glDisable(GL_ALPHA_TEST);

   if (stage->getPMD() != NULL) {
      glPushMatrix();
      stage->getPMD()->renderForPlain();
      glPopMatrix();
   }
   for (i = 0; i < num; i++) {
      if (objs[order[i]].isEnable() == true) {
         pmd = objs[order[i]].getPMDModel();
         if (pmd->getToonFlag() == false && toonLight == true) {
            /* disable toon lighting */
            updateLight(true, false, lightIntensity, lightDirection, lightColor);
            toonLight = false;
         }
         else if (pmd->getToonFlag() == true && toonLight == false) {
            /* enable toon lighting */
            updateLight(useMMDLikeCartoon, useCartoonRendering, lightIntensity, lightDirection, lightColor);
            toonLight = true;
         }
         /* render all vertices without color/texture with a single call */
         pmd->renderForPlain();
      }
   }
   /* restore status */
   glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
   glEnable(GL_ALPHA_TEST);

#ifdef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
   /* set depth func to allow overwrite of the same surface in the following rendering */
   glDepthFunc(GL_LEQUAL);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */

   /* force white light */
   updateLight(useMMDLikeCartoon, useCartoonRendering, OPTION_LIGHTINTENSITY_DEF, lightDirection, defaultLightColor);
   toonLight = true;

   /* render the bloom part */
   if (stage->getPMD() != NULL) {
      glPushMatrix();
      if (stage->getPMD()->hasLuminousMaterial()) {
         /* render luminous materials */
         stage->getPMD()->setLuminousMode(PMDMODEL_LUMINOUS_ON);
         stage->getPMD()->renderModel(false);
      }
      /* reset status */
      stage->getPMD()->setLuminousMode(PMDMODEL_LUMINOUS_NONE);
      glPopMatrix();
   }
   for (i = 0; i < num; i++) {
      if (objs[order[i]].isEnable() == true) {
         pmd = objs[order[i]].getPMDModel();
         if (pmd->getToonFlag() == false && toonLight == true) {
            /* disable toon lighting */
            updateLight(true, false, OPTION_LIGHTINTENSITY_DEF, lightDirection, defaultLightColor);
            toonLight = false;
         }
         else if (pmd->getToonFlag() == true && toonLight == false) {
            /* enable toon lighting */
            updateLight(useMMDLikeCartoon, useCartoonRendering, OPTION_LIGHTINTENSITY_DEF, lightDirection, defaultLightColor);
            toonLight = true;
         }
         if (pmd->hasLuminousMaterial()) {
            /* render luminous materials */
            pmd->setLuminousMode(PMDMODEL_LUMINOUS_ON);
            pmd->renderModel(false);
         }
         /* reset status */
         pmd->setLuminousMode(PMDMODEL_LUMINOUS_NONE);
      }
   }

#ifdef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
   /* restore depth function status */
   glDepthFunc(GL_LESS);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */

   /* restore toon lighting */
   updateLight(useMMDLikeCartoon, useCartoonRendering, lightIntensity, lightDirection, lightColor);

   /* switch to default FBO */
   glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFrameBuffer);

   /* restore viewport */
   glViewport(0, 0, m_width, m_height);

   /* set texture blend function for additive blending: multiply color for black dimming */
   glBlendFunc(GL_ONE, GL_ONE);

   /* disable lighting */
   glDisable(GL_LIGHTING);
   /* disable depth test */
   glDisable(GL_DEPTH_TEST);

   /* set buffers for rendering */
   GLfloat vertices[] = { 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f };
   GLindices indices[] = { 0, 1, 2, 0, 2, 3 };
   GLfloat texcoords[] = { 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f };
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, 0, vertices);
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glTexCoordPointer(2, GL_FLOAT, 0, texcoords);

   /* bind the rendered texture */
   glBindTexture(GL_TEXTURE_2D, m_bloomTextureID);
   glEnable(GL_TEXTURE_2D);

   /* generate mipmap with maximum level limit */
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 6);
   glGenerateMipmap(GL_TEXTURE_2D);
   /* set up projection matrix and model matrix to render the texture onto the screen */
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, 1, 0, 1, -1, 1);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();

   /* render the texture by adding each mip-mapped (hardware-blurred) textures multiple times */
   //   GLfloat biaslist[]  = {0.0f, 1.0f, 2.0f, 4.0f}; // 2
   //   GLfloat alphalist[] = {0.3f, 0.5f, 0.9f, 1.0f};
   GLfloat biaslist[] = { 1.0f, 2.0f, 3.0f, 5.0f }; // 1
   GLfloat alphalist[] = { 0.3f, 0.5f, 0.6f, 0.8f };
   for (int i = 0; i < 4; i++) {
      glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, biaslist[i]);
      glColor4f(alphalist[i] * m_bloomIntensity, alphalist[i] * m_bloomIntensity, alphalist[i] * m_bloomIntensity, alphalist[i] * m_bloomIntensity);
      glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)indices);
   }
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glBindTexture(GL_TEXTURE_2D, 0);

   /* restore projection matrix */
   glMatrixMode(GL_PROJECTION);
   glLoadMatrixd(projection);

   /* reset model view */
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

   /* restore parameters for normal rendering */
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glDisable(GL_TEXTURE_2D);
   glEnable(GL_LIGHTING);
   glEnable(GL_DEPTH_TEST);
#endif /* MMDAGENT_DONTUSELUMINOUS */
}

/* Render::toggleLuminous: toggle luminous effect directly */
void Render::toggleLuminous()
{
#ifndef MMDAGENT_DONTUSELUMINOUS
   if (m_bloomFboID == LUMINOUS_FBO_UNINITIALIZEDID) {
      /* enable */
      initBloomRendering();
   } else {
      if (m_bloomIntensity == 1.0f) {
         m_bloomIntensity = 0.7f;
      } else if (m_bloomIntensity == 0.7f) {
         m_bloomIntensity = 0.4f;
      } else if (m_bloomIntensity == 0.4f) {
         m_bloomIntensity = 1.0f;
         /* disable */
         clearBloomRendering();
      }
   }
#endif /* MMDAGENT_DONTUSELUMINOUS */
}

/* Render::getLuminousIntensity: get luminous intensity */
float Render::getLuminousIntensity()
{
#ifndef MMDAGENT_DONTUSELUMINOUS
   if (m_bloomFboID != LUMINOUS_FBO_UNINITIALIZEDID)
      return m_bloomIntensity;
   return 0.0f;
#else
   return 0.0f;
#endif /* MMDAGENT_DONTUSELUMINOUS */
}


/* Render::updateLuminous: update scene status for luminous rendering */
void Render::updateLuminous(PMDObject *objs, int num, Stage *stage)
{
#ifndef MMDAGENT_DONTUSELUMINOUS
   int i;
   bool exist = false;

   if (stage->getPMD() != NULL && stage->getPMD()->hasLuminousMaterial())
      exist = true;

   for (i = 0; i < num; i++) {
      if (objs[i].isEnable() == false) continue;
      if (objs[i].getPMDModel()->hasLuminousMaterial()) {
         exist = true;
         break;
      }
   }
   if (exist == true && m_existLuminousModel == false) {
      /* enable */
      initBloomRendering();
   }
   else if (exist == false && m_existLuminousModel == true) {
      /* disable */
      clearBloomRendering();
   }
   m_existLuminousModel = exist;
#endif /* MMDAGENT_DONTUSELUMINOUS */
}

#endif /* MY_LUMINOUS */

/* Render::initialize: initialzie Render */
void Render::initialize()
{
   m_width = 0;
   m_height = 0;

   m_trans_set = btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f));
   m_angle_set = btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f));
   m_distance_set = 100.0f;
   m_fovy_set = 16.0f;
   m_baseBone_set = NULL;
   m_baseBoneName = NULL;
   m_modelFollow = false;
   m_lastFollowPos = btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f));
   resetCamera();

   m_currentTrans = m_trans_set;
   m_currentRot = m_rot;
   m_currentDistance = m_distance_set;
   m_currentFovy = m_fovy_set;

   m_viewMoveTime = -1.0;
   m_viewControlledByMotion = false;

   m_transMatrix.setIdentity();
   updateModelViewMatrix();

   m_useShadow = true;
   m_shadowMapInitialized = false;
   m_lightVec = btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f));
   m_shadowMapAutoViewEyePoint = btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f));
   m_shadowMapAutoViewRadius = 0.0f;
   m_viewPointFrustumNear = RENDER_VIEWPOINTFRUSTUMNEAR;
#ifdef __ANDROID__
   /* tweak to avoid crush on old devices where < 1.0 frustum near causes crush */
   /* SOV32: = Xperia Z5 SOV32 */
   {
      char pstr[PROP_VALUE_MAX + 1];
      int len;
      len = __system_property_get("ro.product.model", pstr);
      if (len > 0 && MMDAgent_strheadmatch(pstr, "SOV32")) {
         m_viewPointFrustumNear = 5.0f;
      }
      len = __system_property_get("ro.product.name", pstr);
      if (len > 0 && MMDAgent_strheadmatch(pstr, "SOV32")) {
         m_viewPointFrustumNear = 5.0f;
      }
      len = __system_property_get("ro.product.device", pstr);
      if (len > 0 && MMDAgent_strheadmatch(pstr, "SOV32")) {
         m_viewPointFrustumNear = 5.0f;
      }
   }
#endif /* __ANDROID__ */

#ifdef MY_LUMINOUS
   m_existLuminousModel = false;
   m_bloomFboID = LUMINOUS_FBO_UNINITIALIZEDID;
   m_bloomTextureID = PMDTEXTURE_UNINITIALIZEDID;
   m_bloomRboID = 0;
   m_bloomTextureHeight = 0;
   m_bloomTextureWidth = 0;
   m_bloomIntensity = 1.0f;
#endif

   m_depth = NULL;

   for (int i = 0; i < 3; i++) {
      m_doppelShadowColor[i] = 0.0f;
      m_doppelShadowOffset[i] = 0.0f;
   }
   m_doppelShadowFlag = false;

   m_defaultFrameBuffer = 0;

   m_backgroundForTransparent = false;
}

/* Render::clear: free Render */
void Render::clear()
{
   if(m_depth)
      free(m_depth);
   if (m_baseBoneName)
      free(m_baseBoneName);
   initialize();
}

/* Render::Render: constructor */
Render::Render()
{
   initialize();
}

/* Render::~Render: destructor */
Render::~Render()
{
   clear();
}

/* Render::initSurface: initialize rendering surface */
bool Render::initSurface()
{
   /* set clear color */
   if (m_backgroundForTransparent) {
      glClearColor(m_backgroundTransparentColor[0], m_backgroundTransparentColor[1], m_backgroundTransparentColor[2], m_backgroundTransparentColor[3]);
   } else {
      glClearColor(m_backgroundColor[0], m_backgroundColor[1], m_backgroundColor[2], 1.0f);
   }

   glClearStencil(0);

   /* enable depth test */
   glEnable(GL_DEPTH_TEST);
#ifndef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
   /* set default depth func to allow overwrite of the same surface */
   glDepthFunc(GL_LEQUAL);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */

   /* enable texture */
   glEnable(GL_TEXTURE_2D);

   /* enable face culling */
   glEnable(GL_CULL_FACE);

   /* not render the back surface */
   glCullFace(GL_BACK);

   /* enable alpha blending */
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   /* enable alpha test, to avoid zero-alpha surfaces to depend on the rendering order */
   glEnable(GL_ALPHA_TEST);
   glAlphaFunc(GL_GREATER, 0.0f);

   /* disable writing stencil bits by default */
   glStencilMask(0x0);

   /* enable lighting */
   glEnable(GL_LIGHT0);
   glEnable(GL_LIGHTING);

   return true;
}

/* Render::setup: initialize and setup Render */
bool Render::setup(const int *size, const float *color, const float *trans, const float *rot, float distance, float fovy, bool useShadow, bool useShadowMapping, int shadowMappingTextureSize, int maxNumModel)
{
   if(size == NULL || color == NULL || rot == NULL || trans == NULL)
      return false;

   setCameraView(trans, rot, distance, fovy, NULL, false);
   setViewMoveTimer(0.0);

   for (int i = 0; i < 3; i++)
      m_backgroundColor[i] = color[i];

   /* initialize surface */
   initSurface();

   /* initialize for shadow */
   setShadow(useShadow);

   /* initialization for shadow mapping */
   setShadowMapping(useShadowMapping, shadowMappingTextureSize);

   /* set rendering size */
   setSize(size[0], size[1]);

   if (m_depth)
      free(m_depth);
   m_depth = (RenderDepthData *) malloc(sizeof(RenderDepthData) * maxNumModel);

   return true;
}

/* Render::setSize: set size */
void Render::setSize(int sw, int sh)
{
   int w, h;

#if defined(__APPLE__) && !TARGET_OS_IPHONE
   /* obtain current surface size from cocoa API in GLFW, regardless of the given width/height */
   glfwGetRenderingSize(&w, &h);
#else
   w = sw;
   h = sh;
#endif /* __APPLE__ && !TARGET_OS_IPHONE */

   if (m_width != w || m_height != h) {
      if (w > 0)
         m_width = w;
      if (h > 0)
         m_height = h;
      updateProjectionMatrix();
#ifdef MY_LUMINOUS
      updateBloomRendering();
#endif
   }
}

/* Render::getWidth: get width */
int Render::getWidth()
{
   return m_width;
}

/* Render::getHeight: get height */
int Render::getHeight()
{
   return m_height;
}

/* Render::setCameraView: set camera view parameters */
void Render::setCameraView(const float *trans, const float *angle, float distance, float fovy, PMDBone *baseBone, bool modelFollow)
{
   m_angle_set = btVector3(btScalar(angle[0]), btScalar(angle[1]), btScalar(angle[2]));
   m_trans_set = btVector3(btScalar(trans[0]), btScalar(trans[1]), btScalar(trans[2]));
   m_distance_set = distance;
   m_fovy_set = fovy;
   m_baseBone_set = baseBone;
   if (m_baseBoneName)
      free(m_baseBoneName);
   m_baseBoneName = baseBone ? MMDAgent_strdup(baseBone->getName()) : NULL;
   m_modelFollow = modelFollow;
   m_lastFollowPos = btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f));
   resetCamera();
}


/* Render::getCameraBoneName: get camera bone Name */
const char *Render::getCameraBoneName()
{
   return m_baseBoneName;
}

/* Render::updateCameraBone: update camera bone */
void Render::updateCameraBone(PMDBone *baseBone)
{
   m_baseBone_set = baseBone;
   if (m_baseBoneName)
      free(m_baseBoneName);
   m_baseBoneName = MMDAgent_strdup(baseBone->getName());
   m_baseBone = m_baseBone_set;
   updateRotationFromAngle();
}

/* Render::setCameraParam: set camera view parameter from camera controller */
void Render::setCameraFromController(CameraController *c)
{
   if (c != NULL) {
      c->getCurrentViewParam(&m_distance, &m_trans, &m_angle, &m_fovy);
      updateRotationFromAngle();
      m_viewControlledByMotion = true;
   } else
      m_viewControlledByMotion = false;
}

/* Render::setViewMoveTimer: set timer in sec for rotation, transition, and scale of view */
void Render::setViewMoveTimer(double sec)
{
   m_viewMoveTime = sec;
   if (m_viewMoveTime > 0.0) {
      m_viewMoveStartRot = m_currentRot;
      m_viewMoveStartTrans = m_currentTrans;
      m_viewMoveStartDistance = m_currentDistance;
      m_viewMoveStartFovy = m_currentFovy;
   }
}

/* Render::isViewMoving: return if view is moving by timer */
bool Render::isViewMoving()
{
   if (m_viewMoveTime > 0.0 && (m_currentRot != m_rot || m_currentTrans != m_trans || m_currentDistance != m_distance || m_currentFovy != m_fovy))
      return true;
   return false;
}

/* Render::resetCamera: reset camera view */
void Render::resetCamera()
{
   m_angle = m_angle_set;
   m_trans = m_trans_set;
   m_distance = m_distance_set;
   m_fovy = m_fovy_set;
   m_baseBone = m_baseBone_set;
   updateRotationFromAngle();
}

/* Render::translate: translate */
void Render::translate(float x, float y, float z)
{
   m_trans += btVector3(btScalar(x), btScalar(y), btScalar(z));
}

/* Render::translateWithView: translate with view */
void Render::translateWithView(float x, float y, float z)
{
   btVector3 v;
   btTransform tr;
   float f;

   f = m_distance * tanf(MMDFILES_RAD(m_currentFovy));
   v = btVector3(x * f, y * f, z * f);
   getCurrentViewTransform(&tr);
   tr.setOrigin(btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f)));
   v = tr.inverse() * v;
   m_trans -= v;
}

/* Render::rotate: rotate scene */
void Render::rotate(float x, float y, float z)
{
   m_angle.setX(btScalar(m_angle.x() + x));
   m_angle.setY(btScalar(m_angle.y() + y));
   m_angle.setZ(btScalar(m_angle.z() + z));
   updateRotationFromAngle();
}

/* Render::setDistance: set distance */
void Render::setDistance(float distance)
{
   m_distance = distance;
}

/* Render::getDistance: get distance */
float Render::getDistance()
{
   return m_distance;
}

/* Render::setFovy: set fovy */
void Render::setFovy(float fovy)
{
   m_fovy = fovy;
}

/* Render::getFovy: get fovy */
float Render::getFovy()
{
   return m_fovy;
}

/* Render::setShadow: switch shadow */
void Render::setShadow(bool useShadow)
{
   m_useShadow = useShadow;
}

/* Render::setShadowMapping: switch shadow mapping */
void Render::setShadowMapping(bool useShadowMapping, int textureSize)
{
#ifndef MMDAGENT_DONTUSESHADOWMAP
   if(useShadowMapping) {
      /* enabled */
      if (!m_shadowMapInitialized) {
         /* initialize now */
         initializeShadowMap(textureSize);
         m_shadowMapInitialized = true;
      }
      /* set how to set the comparison result value of R coordinates and texture (depth) value */
      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, m_depthTextureID);
      /* rendering order is light(full) - dark(shadow part), OpenGL should set the shadow part as true */
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
      glDisable(GL_TEXTURE_2D);
      glActiveTexture(GL_TEXTURE0);
   } else {
      /* disabled */
      if (m_shadowMapInitialized) {
         /* disable depth texture unit */
         glActiveTexture(GL_TEXTURE3);
         glDisable(GL_TEXTURE_2D);
         glActiveTexture(GL_TEXTURE0);
      }
   }
#endif /* !MMDAGENT_DONTUSESHADOWMAP */
}

/* Render::getRenderOrder: return rendering order */
void Render::getRenderOrder(int *order, PMDObject *objs, int num)
{
   int i, s;
   btVector3 pos, v;

   if (num == 0)
      return;

   s = 0;
   for (i = 0; i < num; i++) {
      if (objs[i].isEnable() == false || objs[i].allowMotionFileDrop() == false) continue;
      pos = objs[i].getPMDModel()->getCenterBone()->getTransform()->getOrigin();
      pos = m_transMatrix * pos;
      m_depth[s].dist = pos.z();
      m_depth[s].id = i;
      s++;
   }
   qsort(m_depth, s, sizeof(RenderDepthData), compareDepth);
   for (i = 0; i < s; i++)
      order[i] = m_depth[i].id;
   for (i = 0; i < num; i++)
      if (objs[i].isEnable() == false || objs[i].allowMotionFileDrop() == false)
         order[s++] = i;

   for (i = 0; i < num; i++)
      if (objs[i].isEnable() == true)
         objs[i].getPMDModel()->updateMaterialOrder(&m_transMatrix);
}

/* Render::render: render all */
void Render::render(PMDObject *objs, const int *order, int num, Stage *stage, bool useMMDLikeCartoon, bool useCartoonRendering, float lightIntensity, float *lightDirection, float *lightColor, bool useShadowMapping, int shadowMappingTextureSize, float shadowMappingSelfDensity, float shadowMappingFloorDensity, double ellapsedTimeForMove, float shadowDensity)
{
   bool updated;

   /* update camera view matrices */
   updated = updateDistance(ellapsedTimeForMove);
   updated |= updateTransRotMatrix(ellapsedTimeForMove);
   if (updated == true || m_baseBone != NULL)
      updateModelViewMatrix();
   if (updateFovy(ellapsedTimeForMove) == true)
      updateProjectionMatrix();

   if (isViewMoving() == false)
      m_viewMoveTime = -1.0;

#ifdef MY_LUMINOUS
   /* check model status for enable/disable bloom rendering */
   updateLuminous(objs, num, stage);
#endif

#ifndef MMDAGENT_DONTUSESHADOWMAP
   if (m_useShadow && useShadowMapping)
      renderSceneShadowMap(objs, order, num, stage, useMMDLikeCartoon, useCartoonRendering, lightIntensity, lightDirection, lightColor, shadowMappingTextureSize, shadowMappingSelfDensity);
   else
#endif /* !MMDAGENT_DONTUSESHADOWMAP */
      renderScene(objs, order, num, stage, useMMDLikeCartoon, useCartoonRendering, lightIntensity, lightDirection, lightColor, shadowDensity);

#ifdef MY_LUMINOUS
   /* render bloom texture and set onto the screen */
   renderBloomTexture(objs, order, num, stage, useMMDLikeCartoon, useCartoonRendering, lightIntensity, lightDirection, lightColor);
#endif
}

/* Render::pickModel: pick up a model at the screen position */
int Render::pickModel(PMDObject *objs, int num, int x, int y, int *allowDropPicked)
{
#ifdef MMDAGENT_DONTPICKMODEL
   return -1;
#else
   int i;

   GLuint selectionBuffer[512];
   GLint viewport[4];

   GLint hits;
   GLuint *data;
   GLuint minDepth = 0, minDepthAllowDrop = 0;
   int minID, minIDAllowDrop;
   GLuint depth;
   int id;

   /* get current viewport */
   glGetIntegerv(GL_VIEWPORT, viewport);
   /* set selection buffer */
   glSelectBuffer(512, selectionBuffer);
   /* begin selection mode */
   glRenderMode(GL_SELECT);
   /* save projection matrix */
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   /* set projection matrix for picking */
   glLoadIdentity();
   /* apply picking matrix */
   gluPickMatrix(x, viewport[3] - y, 15.0, 15.0, viewport);
   /* apply normal projection matrix */
   applyProjectionMatrix(m_viewPointFrustumNear, RENDER_VIEWPOINTFRUSTUMFAR);
   /* switch to model view mode */
   glMatrixMode(GL_MODELVIEW);
   /* initialize name buffer */
   glInitNames();
   glPushName(0);
   /* draw models with selection names */
   for (i = 0; i < num; i++) {
      if (objs[i].isEnable() == true) {
         glLoadName(i);
         objs[i].getPMDModel()->renderForPick();
      }
   }

   /* restore projection matrix */
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   /* switch to model view mode */
   glMatrixMode(GL_MODELVIEW);
   /* end selection mode and get number of hits */
   hits = glRenderMode(GL_RENDER);
   if (hits == 0) return -1;
   data = &(selectionBuffer[0]);
   minID = -1;
   minIDAllowDrop = -1;
   for (i = 0; i < hits; i++) {
      depth = *(data + 1);
      id = *(data + 3);
      if (minID == -1 || minDepth > depth) {
         minDepth = depth;
         minID = id;
      }
      if (allowDropPicked && objs[id].allowMotionFileDrop()) {
         if (minIDAllowDrop == -1 || minDepthAllowDrop > depth) {
            minDepthAllowDrop = depth;
            minIDAllowDrop = id;
         }
      }
      data += *data + 3;
   }
   if (allowDropPicked)
      *allowDropPicked = minIDAllowDrop;

   return minID;
#endif /* MMDAGENT_DONTPICKMODEL */
}

/* Render::updateLight: update light */
void Render::updateLight(bool useMMDLikeCartoon, bool useCartoonRendering, float lightIntensity, const float *lightDirection, const float *lightColor)
{
   float fLightDif[4];
   float fLightSpc[4];
   float fLightAmb[4];
   int i;
   float d, a, s;

   if (useMMDLikeCartoon == false) {
      /* MMDAgent original cartoon */
      d = 0.2f;
      a = lightIntensity * 1.5f;
      s = 0.4f;
   } else if (useCartoonRendering) {
      /* like MikuMikuDance */
      d = 0.0f;
      a = lightIntensity * 1.5f;
      s = lightIntensity;
   } else {
      /* no toon */
      d = lightIntensity;
      a = 1.0f;
      s = 1.0f; /* OpenGL default */
   }

   for (i = 0; i < 3; i++)
      fLightDif[i] = lightColor[i] * d;
   fLightDif[3] = 1.0f;
   for (i = 0; i < 3; i++)
      fLightAmb[i] = lightColor[i] * a;
   fLightAmb[3] = 1.0f;
   for (i = 0; i < 3; i++)
      fLightSpc[i] = lightColor[i] * s;
   fLightSpc[3] = 1.0f;

   glLightfv(GL_LIGHT0, GL_POSITION, lightDirection);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, fLightDif);
   glLightfv(GL_LIGHT0, GL_AMBIENT, fLightAmb);
   glLightfv(GL_LIGHT0, GL_SPECULAR, fLightSpc);

   /* update light direction vector */
   m_lightVec = btVector3(btScalar(lightDirection[0]), btScalar(lightDirection[1]), btScalar(lightDirection[2]));
   m_lightVec.normalize();
}

/* Render::updateDepthTextureViewParam: update center and radius information to get required range for shadow mapping */
void Render::updateDepthTextureViewParam(PMDObject *objList, int num)
{
   int i;
   float d, dmax;
   float *r = (float *) malloc(sizeof(float) * num);
   btVector3 *c = new btVector3[num];
   btVector3 cc = btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f));

   for (i = 0; i < num; i++) {
      if (objList[i].isEnable() == false)
         continue;
      r[i] = objList[i].getPMDModel()->calculateBoundingSphereRange(&(c[i]));
      cc += c[i];
   }
   cc /= (float) num;

   dmax = 0.0f;
   for (i = 0; i < num; i++) {
      if (objList[i].isEnable() == false)
         continue;
      d = cc.distance(c[i]) + r[i];
      if (dmax < d)
         dmax = d;
   }

   m_shadowMapAutoViewEyePoint = cc;
   m_shadowMapAutoViewRadius = dmax;

   free(r);
   delete [] c;
}

/* Render::getScreenPointPosition: convert screen position to object position */
void Render::getScreenPointPosition(btVector3 *dst, btVector3 *src)
{
   *dst = m_transMatrixInv * (*src);
}

/* Render::getCurrentViewCenterPos: get current view center position */
void Render::getCurrentViewCenterPos(btVector3 *pos)
{
   *pos = m_currentTrans;
}

/* Render::getCurrentViewTransform: get current view transform matrix */
void Render::getCurrentViewTransform(btTransform *tr)
{
   *tr = m_transMatrix;
}

/* Render::getInfoString: store current view parameters to buffer */
void Render::getInfoString(char *buf, int buflen)
{
   MMDAgent_snprintf(buf, buflen, "%.2f, %.2f, %.2f | %.2f, %.2f, %.2f | %.2f | %.2f", m_currentTrans.x(), m_currentTrans.y(), m_currentTrans.z(), m_angle.x(), m_angle.y(), m_angle.z(), m_currentDistance, m_currentFovy);
}

/* Render::clearScreen: clear screen */
void Render::clearScreen()
{
   /* clear all the buffers */
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

/* Render::setDoppelShadowParam: set doppel shadow parameters */
void Render::setDoppelShadowParam(const float *col, const float *offset)
{
   for (int i = 0; i < 3; i++) {
      m_doppelShadowColor[i] = col[i];
      m_doppelShadowOffset[i] = offset[i];
   }
}
/* Render::setDoppelShadowFlag: set doppel shadow flag */
void Render::setDoppelShadowFlag(bool flag)
{
   m_doppelShadowFlag = flag;
}

/* Render::setDefaultFrameBufferId: set default frame buffer id */
void Render::setDefaultFrameBufferId(GLuint id)
{
   m_defaultFrameBuffer = id;
   glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFrameBuffer);
}

/* Render::setBackgroundTransparency: set background transparency */
void Render::setBackgroundTransparency(bool flag, const float *bgcolor)
{
   m_backgroundForTransparent = flag;
   if (bgcolor) {
      for (int i = 0; i < 4; i++) {
         m_backgroundTransparentColor[i] = bgcolor[i];
      }
   }
}
