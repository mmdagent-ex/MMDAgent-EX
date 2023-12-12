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

#define PMDTEXTURE_UNINITIALIZEDID 0xFFFFFFFF

/* PMDTexture: texture of PMD */
class PMDTexture
{
private:

   GLuint m_id;                  /* OpenGL texture id */
   bool m_isTransparent;         /* true if this texture contains transparency */
   bool m_isSphereMap;           /* true if this texture is sphere map (.sph or .spa) */
   bool m_isSphereMapAdd;        /* true if this is sphere map to add (.spa) */
   int m_width;                  /* texture image width */
   int m_height;                 /* texture image height */
   unsigned char m_components;   /* number of components (3 for RGB, 4 for RGBA) */
   unsigned char *m_textureData; /* texel data */
   unsigned int m_textureDataLen;
   bool m_isAnimated;                /* true when this is animated texture */
   int m_numFrames;                  /* number of defined frames in animated texture */
   unsigned char **m_animationData;  /* buffer to hold frame images */
   int *m_animOffsetX;               /* x offset of updated part at each frame */
   int *m_animOffsetY;               /* y offset of updated part at each frame */
   int *m_animWidth;                 /* width of updated part at each frame */
   int *m_animHeight;                /* height of updated part at each frame */
   unsigned char *m_disposeOp;       /* dispose operation flag at each frame */
   unsigned char *m_blendOp;         /* blend operation flag at each frame */
   double *m_endFrame;               /* calculated end of frame at each frame */
   double m_totalDuration;           /* total frames */
   double m_restFrame;               /* work area to hold rest frame at each progress */
   int m_currentFrame;               /* work area to hold current frame at each progress */
   int m_format;                     /* RGB or RGBA format */
   double m_animSpeedRate;           /* animation target speed rate */
   double m_animCurrentRate;         /* animtaion current speed rate */

private:

   /* loadBMP: load BMP texture */
   bool loadBMP(const char *fileName);

   /* loadTGA: load TGA texture */
   bool loadTGA(const char *fileName);

   /* loadJPG: load JPG texture */
   bool loadJPG(const char *fileName);

   /* initialize: initialize texture */
   void initialize();

   /* clear: free texture */
   void clear();

public:

   /* PMDTexture: constructor */
   PMDTexture();

   /* ~PMDTexture: destructor */
   ~PMDTexture();

   /* load: load image from file name as texture */
   bool load(const char *fileName, bool sphereFlag = false, bool sphereAddFlag = false);

   /* loadPNG: load PNG texture */
   bool loadPNG(const char *fileName);

   /* loadImage: load image from file name as an image */
   bool loadImage(const char *fileName);

   /* getID: get OpenGL texture ID */
   GLuint getID();

   /* getWidth: get width */
   int getWidth();

   /* getHeight: get height */
   int getHeight();

   /* isTransparent: return true if this texture contains transparency */
   bool isTransparent();

   /* isSphereMap: return true if this texture is sphere map */
   bool isSphereMap();

   /* isSphereMapAdd: return true if this is sphere map to add */
   bool isSphereMapAdd();

   /* release: free texture */
   void release();

   /* savePNG: save image as PNG */
   bool savePNG(GLubyte *bytes, int width, int height, const char *filename);

   /* setNextFrame: set next frame */
   void setNextFrame(double ellapsedFrame);

   /* setAnimationSpeedRate: set animatin speed rate */
   void setAnimationSpeedRate(double rate);

   /* getData: get data */
   unsigned char *getData();

   /* getDataLength: get data length */
   unsigned int getDataLength();

   /* getComponentNum: get component num */
   int getComponentNum();

};
