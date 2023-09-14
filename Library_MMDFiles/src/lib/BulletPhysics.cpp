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
#include "BulletCollision/CollisionShapes/btShapeHull.h"

/* BulletPhysics::initialize: initialize BulletPhysics */
void BulletPhysics::initialize()
{
   m_collisionConfig = NULL;
   m_dispatcher = NULL;
   m_overlappingPairCache = NULL;
   m_solver = NULL;
   m_world = NULL;

   m_fps = 0;
   m_numObjects = 0;
   m_worldModified = false;

   m_debugVertices = NULL;
   m_debugColors = NULL;
   m_debugColorsGroup = NULL;
   m_debugIndices = NULL;
   m_debugNumVertices = 0;
   m_debugNumIndices = 0;
   m_debugTransformedVertices = NULL;
   m_debugNumVertivesPerHull = NULL;
   m_fboID = 0;
   m_textureID = 0;
   m_depthRenderID = 0;
   m_fboInitialized = false;
   m_fboDisabled = false;
}

/* BulletPhysics::clear: free BulletPhysics */
void BulletPhysics::clear()
{
   int i, numObject;
   btCollisionObject *obj;
   btRigidBody *body;

   if (m_world) {
      /* release remaining objects within the world */
      numObject = m_world->getNumCollisionObjects();
      for (i = 0; i < numObject; i++) {
         obj = m_world->getCollisionObjectArray()[i];
         body = btRigidBody::upcast(obj);
         if (body && body->getMotionState())
            delete body->getMotionState();
         m_world->removeCollisionObject(obj);
         delete obj;
      }
   }
   if (m_world)
      delete m_world;
   if (m_solver)
      delete m_solver;
   if (m_overlappingPairCache)
      delete m_overlappingPairCache;
   if (m_dispatcher)
      delete m_dispatcher;
   if (m_collisionConfig)
      delete m_collisionConfig;

   if (m_debugVertices != NULL)
      free(m_debugVertices);
   if (m_debugColors != NULL)
      free(m_debugColors);
   if (m_debugColorsGroup != NULL)
      free(m_debugColorsGroup);
   if (m_debugIndices != NULL)
      free(m_debugIndices);
   if (m_debugTransformedVertices != NULL)
      free(m_debugTransformedVertices);
   if (m_debugNumVertivesPerHull != NULL)
      free(m_debugNumVertivesPerHull);

   if (m_fboID != 0)
      glDeleteFramebuffers(1, &m_fboID);
   if (m_textureID != 0)
      glDeleteTextures(1, &m_textureID);
   if (m_depthRenderID != 0)
      glDeleteRenderbuffers(1, &m_depthRenderID);

   initialize();
}

/* BulletPhysics::BulletPhysics: constructor */
BulletPhysics::BulletPhysics()
{
   initialize();
}

/* BulletPhysics::~BulletPhysics: destructor */
BulletPhysics::~BulletPhysics()
{
   clear();
}

/* BulletPhysics::setup: initialize and setup BulletPhysics */
void BulletPhysics::setup(int simulationFps, float gravityFactor)
{
   float dist = 400.0f;

   clear();

   /* store values */
   m_fps = simulationFps;
   m_subStep = btScalar(1.0f / m_fps);

   /* make a collision configuration */
   m_collisionConfig = new btDefaultCollisionConfiguration();

   /* make a collision dispatcher from the configuration for sequenciall processing */
   m_dispatcher = new btCollisionDispatcher(m_collisionConfig);

   /* set broadphase */
   m_overlappingPairCache = new btAxisSweep3(btVector3(btScalar(-dist), btScalar(-dist), btScalar(-dist)), btVector3(btScalar(dist), btScalar(dist), btScalar(dist)), 1024);

   /* make a sequencial constraint solver */
   m_solver = new btSequentialImpulseConstraintSolver();

   /* create simulation world */
   m_world = new btDiscreteDynamicsWorld(m_dispatcher, m_overlappingPairCache, m_solver, m_collisionConfig);

   /* set default gravity */
   /* some tweak for the simulation to match that of MikuMikuDance */
   m_world->setGravity(btVector3(btScalar(0.0f), btScalar(-9.8f * gravityFactor), btScalar(0.0f)));

   /* a weird configuration to use 120Hz simulation */
   /* change the number of constraint solving iteration to be inversely propotional to simulation rate */
   /* is this a bug of bulletphysics? */
   m_world->getSolverInfo().m_numIterations = (int) (10 * 60 / m_fps);
}

/* BulletPhysics::addFloor: add floor */
void BulletPhysics::addFloor()
{
   btCollisionShape *groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
   btDefaultMotionState *groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
   btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
   btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
   m_world->addRigidBody(groundRigidBody, short(0x0001 << BULLETPHYSICS_FLOOR_COLLISION_GROUP), short(btBroadphaseProxy::AllFilter));
   setModifiedFlag();
}

/* BulletPhysics::update: step the simulation world forward */
void BulletPhysics::update(float deltaFrame)
{
   btScalar sec = deltaFrame / 30.0f; /* convert frame to second */

   if (sec > 1.0) {
      /* long pause, just move ahead at one step */
      m_world->stepSimulation(sec, 1, sec);
   } else {
      /* progress by (1.0/fps) sub step */
      m_world->stepSimulation(sec, m_fps, m_subStep);
   }
}

/* BulletPhysics::getWorld: get simulation world */
btDiscreteDynamicsWorld *BulletPhysics::getWorld()
{
   return m_world;
}

/* BulletPhysics::setModifiedFlag: set modified flag */
void BulletPhysics::setModifiedFlag()
{
   m_worldModified = true;
}

/* debugDisplay: render rigid bodies */
void BulletPhysics::debugDisplay()
{
#ifndef MMDFILES_DONTRENDERDEBUG
   unsigned char color[3];
   GLubyte groupcolor[16][3] =
   {
      { 255, 212, 107 },
      { 199, 250, 113 },
      { 142, 255, 147 },
      { 110, 255, 215 },
      { 126, 224, 255 },
      { 139, 133, 221 },
      { 221, 136, 253 },
      { 234, 84, 8 },
      { 255, 249, 3 },
      { 0, 255, 0 },
      { 0, 255, 255 },
      { 0, 103, 231 },
      { 82, 10, 244 },
      { 255, 0, 255 },
      { 230, 21, 60 },
      { 124, 134, 0 }
   };
   int i;
   GLint polygonMode[2];
   btRigidBody* body;
   btCollisionShape* shape;
   int numObjects = m_world->getNumCollisionObjects();
   int vid;
   int iid;
   int group;

   if (m_numObjects != numObjects || m_worldModified == true) {
      /* world changed, reconstruct vertice attributes for debug drawing */
      m_numObjects = numObjects;
      m_worldModified = false;

      /* free memories */
      if (m_debugVertices != NULL) {
         free(m_debugVertices);
         m_debugVertices = NULL;
      }
      if (m_debugIndices != NULL) {
         free(m_debugVertices);
         m_debugIndices = NULL;
      }
      if (m_debugColors != NULL) {
         free(m_debugColors);
         m_debugColors = NULL;
      }
      if (m_debugTransformedVertices != NULL) {
         free(m_debugTransformedVertices);
         m_debugTransformedVertices = NULL;
      }
      if (m_debugNumVertivesPerHull != NULL) {
         free(m_debugNumVertivesPerHull);
         m_debugNumVertivesPerHull = NULL;
      }

      /* count required number of vertices and indices */
      m_debugNumVertices = 0;
      m_debugNumIndices = 0;
      for (i = 0; i < numObjects; i++) {
         body = btRigidBody::upcast(m_world->getCollisionObjectArray()[i]);
         shape = body->getCollisionShape();
         if (shape->isConvex()) {
            btShapeHull *hull = new btShapeHull((btConvexShape*)shape);
            hull->buildHull(shape->getMargin());
            if (hull->numVertices() > 0) {
               m_debugNumVertices += hull->numVertices();
               m_debugNumIndices += hull->numIndices();
            }
            delete hull;
         }
      }
      if (m_debugNumVertices == 0 || m_debugNumIndices == 0)
         return;

      /* allocate memories */
      m_debugVertices = (btVector3 *)malloc(sizeof(btVector3) * m_debugNumVertices);
      m_debugColors = (unsigned char *)malloc(sizeof(unsigned char) * 4 * m_debugNumVertices);
      m_debugColorsGroup = (unsigned char *)malloc(sizeof(unsigned char) * 4 * m_debugNumVertices);
      m_debugIndices = (INDICES *)malloc(sizeof(INDICES) * m_debugNumIndices);
      m_debugNumVertivesPerHull = (INDICES *)malloc(sizeof(INDICES) * m_numObjects);

      /* put vertices, colors and indices */
      vid = 0;
      iid = 0;
      for (i = 0; i < numObjects; i++) {
         body = btRigidBody::upcast(m_world->getCollisionObjectArray()[i]);
         shape = body->getCollisionShape();
         if (body->getFlags() & BULLETPHYSICS_RIGIDBODYFLAGB) {
            color[0] = 76;
            color[1] = 179;
            color[2] = 0;
         } else if (body->getFlags() & BULLETPHYSICS_RIGIDBODYFLAGP) {
            color[0] = 200;
            color[1] = 100;
            color[2] = 26;
         } else if (body->getFlags() & BULLETPHYSICS_RIGIDBODYFLAGA) {
            color[0] = 200;
            color[1] = 200;
            color[2] = 0;
         } else {
            color[0] = 76;
            color[1] = 200;
            color[2] = 0;
         }
         group = (body->getFlags() & 0x0F00) >> 8;
         if (shape->isConvex()) {
            btShapeHull *hull = new btShapeHull((btConvexShape*)shape);
            hull->buildHull(shape->getMargin());
            m_debugNumVertivesPerHull[i] = hull->numVertices();
            if (hull->numVertices() > 0) {
               for (int j = 0; j < hull->numVertices(); j++) {
                  m_debugVertices[vid + j] = hull->getVertexPointer()[j];
                  m_debugColors[(vid + j) * 4]     = color[0];
                  m_debugColors[(vid + j) * 4 + 1] = color[1];
                  m_debugColors[(vid + j) * 4 + 2] = color[2];
                  m_debugColors[(vid + j) * 4 + 3] = 200;
                  m_debugColorsGroup[(vid + j) * 4] = groupcolor[group][0];
                  m_debugColorsGroup[(vid + j) * 4 + 1] = groupcolor[group][1];
                  m_debugColorsGroup[(vid + j) * 4 + 2] = groupcolor[group][2];
                  m_debugColorsGroup[(vid + j) * 4 + 3] = 200;
               }
               for (int j = 0; j < hull->numIndices(); j++)
                  m_debugIndices[iid + j] = hull->getIndexPointer()[j] + vid;
               vid += hull->numVertices();
               iid += hull->numIndices();
            }
            delete hull;
         }
      }
      /* allocate memory for storing vertices after transform */
      m_debugTransformedVertices = (btVector3 *)malloc(sizeof(btVector3) * m_debugNumVertices);
   }

   if (m_debugNumVertices == 0 || m_debugNumIndices == 0)
      return;

   /* update vertices according to the world transforms */
   vid = 0;
   for (i = 0; i < numObjects; i++) {
      body = btRigidBody::upcast(m_world->getCollisionObjectArray()[i]);
      shape = body->getCollisionShape();
      btTransform tr = body->getWorldTransform();
      if (shape->isConvex()) {
         for (INDICES j = 0; j < m_debugNumVertivesPerHull[i]; j++) {
            m_debugTransformedVertices[vid + j] = tr * m_debugVertices[vid + j];
         }
         vid += m_debugNumVertivesPerHull[i];
      }
   }

   /* prepare fbo if not yet */
   if (m_fboInitialized == false && m_fboDisabled == false) {
      glGenTextures(1, &m_textureID);
      glBindTexture(GL_TEXTURE_2D, m_textureID);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, BULLETPHYSICS_DEBUG_TEXTURE_SIZE, BULLETPHYSICS_DEBUG_TEXTURE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glBindTexture(GL_TEXTURE_2D, 0);

      glGenRenderbuffers(1, &m_depthRenderID);
      glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderID);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, BULLETPHYSICS_DEBUG_TEXTURE_SIZE, BULLETPHYSICS_DEBUG_TEXTURE_SIZE);
      glBindRenderbuffer(GL_RENDERBUFFER, 0);

      glGenFramebuffers(1, &m_fboID);
      glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureID, 0);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderID);

      GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
      if(status != GL_FRAMEBUFFER_COMPLETE)
         m_fboDisabled = true;

      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      m_fboInitialized = true;
   }

   if (m_fboDisabled == true) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glDisable(GL_LIGHTING);
      glDisable(GL_TEXTURE_2D);
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(3, GL_FLOAT, sizeof(btVector3), m_debugTransformedVertices);
      glEnableClientState(GL_COLOR_ARRAY);
      glColorPointer(4, GL_UNSIGNED_BYTE, 0, m_debugColors);
      glDrawElements(GL_TRIANGLES, m_debugNumIndices, GL_INDICES, m_debugIndices);
      glDisableClientState(GL_COLOR_ARRAY);
      glDisableClientState(GL_VERTEX_ARRAY);
      glEnable(GL_LIGHTING);
      if (polygonMode[1] != GL_LINE)
         glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      return;
   }

   /* save viewport */
   GLint viewport[4];
   glGetIntegerv(GL_VIEWPORT, viewport);

   /* get status about wire mode */
   glGetIntegerv(GL_POLYGON_MODE, polygonMode);

   /* switch to fill on wire mode */
   if (polygonMode[1] == GL_LINE)
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

   /* draw bodies to texture */
   glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
   glViewport(0, 0, BULLETPHYSICS_DEBUG_TEXTURE_SIZE, BULLETPHYSICS_DEBUG_TEXTURE_SIZE);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glDisable(GL_LIGHTING);
   glDisable(GL_TEXTURE_2D);
   glDisable(GL_DEPTH_TEST);
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, sizeof(btVector3), m_debugTransformedVertices);
   glEnableClientState(GL_COLOR_ARRAY);
   glColorPointer(4, GL_UNSIGNED_BYTE, 0, m_debugColorsGroup);
   glDrawElements(GL_TRIANGLES, m_debugNumIndices, GL_INDICES, m_debugIndices);
   glDisableClientState(GL_COLOR_ARRAY);
   glDisableClientState(GL_VERTEX_ARRAY);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
   glBindFramebuffer(GL_FRAMEBUFFER, 0);

   /* restore viewport */
   glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

   /* switch to wire mode */
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

   /* draw lines to default FBO */
   glDisable(GL_LIGHTING);
   glDisable(GL_TEXTURE_2D);
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, sizeof(btVector3), m_debugTransformedVertices);
   glEnableClientState(GL_COLOR_ARRAY);
   glColorPointer(4, GL_UNSIGNED_BYTE, 0, m_debugColors);
   glDrawElements(GL_TRIANGLES, m_debugNumIndices, GL_INDICES, m_debugIndices);
   glDisableClientState(GL_COLOR_ARRAY);
   glDisableClientState(GL_VERTEX_ARRAY);
   glEnable(GL_LIGHTING);

   /* switch to fill mode */
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

   /* put body texture on screen */
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
   glDisable(GL_DEPTH_TEST);
   glBindTexture(GL_TEXTURE_2D, m_textureID);
   glEnable(GL_TEXTURE_2D);

   GLfloat coords[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };
   GLfloat vers[] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0, 1.0f, 1.0f, 1.0f };

   glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(2, GL_FLOAT, 0, vers);
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glTexCoordPointer(2, GL_FLOAT, 0, coords);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisableClientState(GL_VERTEX_ARRAY);

   glDisable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 0);
   glEnable(GL_DEPTH_TEST);
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glEnable(GL_LIGHTING);

   /* switch to line on wire mode */
   if (polygonMode[1] == GL_LINE)
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

#endif /* !MMDFILES_DONTRENDERDEBUG */
}
