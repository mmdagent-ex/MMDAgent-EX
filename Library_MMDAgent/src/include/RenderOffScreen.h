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
   GLuint m_transFramebuffer;
   GLuint m_transTexture;
   bool m_requireSurfaceInit;     /* true when buffers are just assigned and needs initialization */

   GLuint m_shaderProgram;   /* shader program id */
   bool m_shaderLoaded;      /* true when shader has been loaded */
   GLchar m_infoLog[512];    /* shader info log string */

   int m_width;            /* screen width */
   int m_height;           /* screen height */

   float m_intensity;      /* intensity of effect */
   float m_scalingFactor;  /* scale of effect */

   Render *m_render;       /* render buffer */
   int m_pauseCount;       /* pause call counter */
   bool m_skip;

   bool m_tpUsePixmap;      /* ttue when use pixmap, else use color-based */
   void *m_tpPixels; /* pixel work area */
   bool m_tpWindow;

   bool m_base;         /* true when base off-screen rendering is enabled */
   bool m_effect;       /* true when effect rendering is enabled */
   bool m_transparent;  /* true when current window is (being) transparent */
   bool m_baseCurrent;
   bool m_effectCurrent;
   bool m_transparentCurrent;
   bool m_baseActive;          /* true when base buffers are assigned */
   bool m_effectActive;        /* true when effect buffers are assigned */
   bool m_transparentActive;   /* true when transparent buffers are assigned */

   /* initialize: initialzie Render */
   void initialize();

   /* clear: free Render */
   void clear();

   /* loadShader: load shader programs */
   bool loadShader();

   /* initBaseBuffers: initialize base buffers */
   bool initBaseBuffers();

   /* initEffectBuffers: initialize effect buffers */
   bool initEffectBuffers();

   /* initTransparentBuffers: initialize transparent buffers */
   bool initTransparentBuffers();

   /* clearBaseBuffers: clear base buffers */
   void clearBaseBuffers();

   /* clearEffectBuffers: clear effect buffers */
   void clearEffectBuffers();

   /* clearTransparentBuffers: clear transparent buffers */
   void clearTransparentBuffers();

public:

   /* RenderOffScreen: constructor */
   RenderOffScreen();

   /* ~RenderOffScreen: destructor */
   ~RenderOffScreen();

   /* getInfoLog: get info log */
   const char *getInfoLog();

   /* updateRender: update render status */
   void updateRender();

   /* setup: initialize and setup */
   bool setup(Render *render, int width, int height);

   /* changeScreenSize: change screen size */
   void changeScreenSize(int width, int height);

   /* start: start off-screen rendering */
   void start();

   /* finish: finish off-screen rendering */
   void finish();

   /* pause: pause */
   void pause();

   /* resume: resume */
   void resume();

   /* setParam: set intensity and scaling parameters */
   void setParam(float intensity, float scaling);

   /* getIntensity: get intensity */
   float getIntensity();

   /* getScaling: get scaling */
   float getScaling();

   /* setRelativeIntensity: set relative intensity */
   void setRelativeIntensity(float addval);

   /* setRelativeScaling: set relative scaling */
   void setRelativeScaling(float mulval);

   /* isTransparentWindow: true when window is transparent */
   bool isTransparentWindow();

   /* enableTransparentWindow: enable transparent window */
   bool enableTransparentWindow(const float *transparentColor, bool usePixmap);

   /* disableTransparentWindow: disable transparent window */
   bool disableTransparentWindow();
};
