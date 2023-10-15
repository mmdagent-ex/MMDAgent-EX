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

#define BULLETPHYSICS_PI 3.1415926535897932384626433832795

#define BULLETPHYSICS_RIGIDBODYFLAGB 0x0010
#define BULLETPHYSICS_RIGIDBODYFLAGP 0x0020
#define BULLETPHYSICS_RIGIDBODYFLAGA 0x0040
#define BULLETPHYSICS_RIGIDBODYFLAGGROUPMASK 0x0F00

/* collision group of static floor */
#define BULLETPHYSICS_FLOOR_COLLISION_GROUP 15
/* latitude division for debug sphere rendering */
#define BULLETPHYSICS_SPHERELATS 10
/* longitude division for debug sphere rendering */
#define BULLETPHYSICS_SPHERELONGS 10
/* number of vertices for debug sphere rendering */
#define BULLETPHYSICS_SPHEREVERTEXNUM ((BULLETPHYSICS_SPHERELATS - 1) * BULLETPHYSICS_SPHERELONGS + 2) // 92
/* number of indices for debug sphere rendering */
#define BULLETPHYSICS_SPHEREINDEXNUM ((BULLETPHYSICS_SPHERELATS - 1) * BULLETPHYSICS_SPHERELONGS + (BULLETPHYSICS_SPHERELATS * BULLETPHYSICS_SPHERELONGS) + 1) // 191
/* debug texture size */
#define BULLETPHYSICS_DEBUG_TEXTURE_SIZE 512

/* BulletPhysics: Bullet Physics engine */
class BulletPhysics
{
private:

   btDefaultCollisionConfiguration *m_collisionConfig; /* collision configuration */
   btCollisionDispatcher *m_dispatcher;                /* collision dispatcher */
   btAxisSweep3 *m_overlappingPairCache;
   btConstraintSolver *m_solver;                       /* constraint solver */
   btDiscreteDynamicsWorld *m_world;                   /* the simulation world */

   int m_fps;          /* simulation frame rate (Hz) */
   btScalar m_subStep; /* sub step to process simulation */
   int m_numObjects;   /* number of objects in the simulation world */
   bool m_worldModified; /* true when some object has been changed in the simulation world */

   btVector3 *m_debugVertices;                 /* Vertices for debug drawing */
   unsigned char *m_debugColors;               /* Colors for debug drawing, by type */
   unsigned char *m_debugColorsGroup;          /* Colors for debug drawing, by collision group */
   INDICES *m_debugIndices;             /* Indices for debug drawing */
   int m_debugNumVertices;                     /* Number of vertices for debug drawing */
   int m_debugNumIndices;                      /* Number of indices for debug drawing */
   INDICES *m_debugNumVertivesPerHull;  /* Numbers of vertices per object */
   btVector3 *m_debugTransformedVertices;      /* buffer to store transformed vertices */

   GLuint m_fboID;          /* frame buffer object id for debug drawing */
   GLuint m_textureID;      /* texture id for debug drawing */
   GLuint m_depthRenderID;  /* depth render buffer id for debug drawing */
   bool m_fboInitialized;   /* true when the FBO was initialize */
   bool m_fboDisabled;      /* true when failed to initalize FBO */

   /* initialize: initialize BulletPhysics */
   void initialize();

   /* clear: free BulletPhysics */
   void clear();

public:

   /* BulletPhysics: constructor */
   BulletPhysics();

   /* ~BulletPhysics: destructor */
   ~BulletPhysics();

   /* setup: initialize and setup BulletPhysics */
   void setup(int simulationFps, float gravityFactor);

   /* addFloor: add floor */
   void addFloor();

   /* update: step the simulation world forward */
   void update(float deltaFrame);

   /* getWorld: get simulation world */
   btDiscreteDynamicsWorld *getWorld();

   /* setModifiedFlag: set modified flag */
   void setModifiedFlag();

   /* debugDisplay: render rigid bodies */
   void debugDisplay();
};
