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

/* RenderOffScreen: off-screen render class with shader */
class RenderOffScreen
{
private:
   GLuint m_framebuffer;          /* frame buffer for off-screen rendering */
   GLuint m_renderbuffer;         /* rendering buffer for framebuffer */
   GLuint m_textureColorbuffer;   /* texture buffer for framebuffer (multisample) */
   GLuint m_resolveFramebuffer;   /* frame buffer for resolving multisample texture */
   GLuint m_resolvedTexture;      /* resolved texture */
   GLuint m_processFramebuffer;   /* second frame buffer for second pass rendering */
   GLuint m_processTextureColorbuffer;  /* second texture buffer for second pass rendering */
   GLuint m_quadVBO;              /* VBO for drawing resolve texture on screen */
   GLuint m_quadVAO;              /* VAO for drawing resolve texture on screen */
   bool m_buffersActive;          /* true when buffers are assigned */
   bool m_buffersRequireInit;     /* true when buffers are just assigned and needs initialization */

   GLuint m_shaderProgram;   /* shader program id */
   bool m_shaderLoaded;      /* true when shader has been loaded */
   GLchar m_infoLog[512];    /* shader info log string */

   int m_width;            /* screen width */
   int m_height;           /* screen height */

   bool m_enabled;         /* true when off-screen rendering is enabled now */
   float m_intensity;      /* intensity of effect */
   float m_scalingFactor;  /* scale of effect */

   /* initialize: initialzie Render */
   void initialize();

   /* clear: free Render */
   void clear();

   /* loadShader: load shader programs */
   bool loadShader();

   /* initBuffers: initialize buffers */
   bool initBuffers();

   /* clearBuffers: clear buffers */
   void clearBuffers();

public:

   /* RenderOffScreen: constructor */
   RenderOffScreen();

   /* ~RenderOffScreen: destructor */
   ~RenderOffScreen();

   /* getInfoLog: get info log */
   const char *getInfoLog();

   /* setup: initialize and setup */
   bool setup(int width, int height);

   /* changeScreenSize: change screen size */
   void changeScreenSize(int width, int height);

   /* start: start off-screen rendering */
   void start();

   /* finish: finish off-screen rendering */
   void finish();

   /* getFrameBufferId: get frame buffer id */
   GLuint getFrameBufferId();

   /* getFrameBufferRequireInit: return flag if the current frame buffer requires surface initialization */
   bool getFrameBufferRequireInit();

   /* setFrameBufferRequireInit: set flag if the current frame buffer requires surface initialization */
   void setFrameBufferRequireInit(bool flag);

   /* getIntensity: get intensity */
   float getIntensity();

   /* setIntensity: set intensity */
   void setIntensity(float f);

   /* toggleIntensity: toggle between a set of intensity values */
   void toggleIntensity();

   /* getScaling: get scaling */
   float getScaling();

   /* setScaling: set scaling */
   void setScaling(float f);

   /* toggleScaling: toggle between a set of scaling values */
   void toggleScaling();

};
