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

/* Stage: stage */
class Stage
{
private:

   TileTexture m_floor;      /* floor texture */
   TileTexture m_background; /* background texture */

   PMDModel m_pmd;           /* PMD for background */
   bool m_hasPMD;            /* true if m_pmd is used */
   float m_range;            /* stage range */

   /* work area */
   GLfloat m_floorShadow[4][4]; /* matrix for shadow of floor */

   PMDTexture *m_frameTexture; /* window frame texture */
   GLfloat m_frameVertices[12];
   GLindices m_frameIndices[6];
   GLfloat m_frameTexcoords[8];
   float m_width;
   float m_height;

   /* initialize: initialize stage */
   void initialize();

   /* clear: free stage */
   void clear();

public:

   /* Stage: constructor */
   Stage();

   /* ~Stage: destructor */
   ~Stage();

   /* setSize: set size of floor and background */
   void setSize(const float *size, float numx, float numy);

   /* loadFloor: load floor image */
   bool loadFloor(const char *file);

   /* loadBackground: load background image */
   bool loadBackground(const char *file);

   /* loadStagePMD: load stage pmd */
   bool loadStagePMD(const char *file, BulletPhysics *bullet, SystemTexture *systex);

   /* renderFloor: render the floor */
   void renderFloor();

   /* renderBackground: render the background */
   void renderBackground();

   /* renderPMD: render the stage pmd */
   void renderPMD();

   /* getPMD: get pmd */
   PMDModel *getPMD();

   /* updateShadowMatrix: update shadow projection matrix */
   void updateShadowMatrix(const float *lightDirection);

   /* getShadowMatrix: get shadow projection matrix */
   GLfloat *getShadowMatrix();

   /* getRange: return maximum stage range from origin */
   float getRange();

   /* update: update */
   void update(double ellapsedFrame);

   /* loadFrameTexture: load frame texture */
   bool loadFrameTexture(const char *file);

   /* hasFrameTexture: return TRUE if has frame texture */
   bool hasFrameTexture();

   /* renderFrameTexture2D: render frame texture */
   void renderFrameTexture2D(float screenWidth, float screenHeight);
};
