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

/* headers */

#include "MMDAgent.h"

/* shader programs */

/* basic vertex shader */
const char *vertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aTexCoords;

    out vec2 TexCoords;

    void main()
    {
        gl_Position = vec4(aPos, 0.0, 1.0);
        TexCoords = aTexCoords;
    }
)glsl";

/* basic fragment shader, just copy texture to screen */
const char *fragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;

    in vec2 TexCoords;

    uniform sampler2D screenTexture;
    uniform vec2 resolution;

    void main()
    {
        FragColor = texture(screenTexture, TexCoords);
    }
)glsl";

/* testing diffusion effect */
const char *vertexShaderSourceDiffusion = R"glsl(
#version 330 core

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec2 aTeXcoords;

out vec2 TexCoords;

uniform vec2 resolution;

void main() {
    gl_Position = in_position;
    TexCoords = aTeXcoords + (vec2(0.5, 0.5) / resolution);
}

)glsl";

const char *fragmentShaderSourceDiffusion = R"glsl(
#version 330 core

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D screenTexture;
uniform sampler2D processedTexture;

uniform vec2 resolution;

uniform int horizontal;

uniform float Strength;
//const float Strength = 1.0 / 3.0;

//uniform float Extent;
//uniform vec3 ColorFilter;
//uniform float alpha1;

const float Extent = 0.002;
const vec3 ColorFilter = vec3(1.0, 1.0, 1.0);
const float alpha1 = 1.0;
//const float scaling = 1.0;
uniform float scaling;
const float ObjXYZ = 1.0;

const int SAMP_NUM = 7;
const float B_COLOR = 0.5;

const bool NON_TRANSPARENT = true;
const vec4 ClearColor = NON_TRANSPARENT ? vec4(1.0, 1.0, 1.0, 1.0) : vec4(B_COLOR, B_COLOR, B_COLOR, 0.0);
const float ClearDepth = 1.0;

vec2 SampStep = vec2(Extent * resolution.y / resolution.x, Extent) * scaling;

vec4 passX(vec2 TexCoord) {
    vec4 sum = vec4(0);
    float e, f, n = 0;

    for (int i = -SAMP_NUM; i <= SAMP_NUM; i++) {
        f = float(i);
        e = exp(-pow(f / (float(SAMP_NUM) / 2.0), 2.0) / 2.0);
        sum += pow(texture(screenTexture, vec2(TexCoord.x + SampStep.x * f, TexCoord.y)), vec4(2.0,2.0,2.0,2.0)) * e;
        n += e;
    }

    return sum / n;
}

vec4 passY(vec2 TexCoord) {
    vec4 Color, sum = vec4(0);
    float e, f, n = 0;

    for (int i = -SAMP_NUM; i <= SAMP_NUM; i++) {
        f = float(i);
        e = exp(-pow(f / (float(SAMP_NUM) / 2.0), 2.0) / 2.0);
        sum += texture(processedTexture, vec2(TexCoord.x, TexCoord.y + SampStep.y * f)) * e;
        n += e;
    }

    Color = sum / n;

    vec4 ColorOrg = texture(screenTexture, TexCoord);
    vec4 ColorSrc = vec4(pow(ColorOrg.rgb, vec3(2.0,2.0,2.0)), ColorOrg.a);

    Color = ColorSrc + Color - ColorSrc * Color;
    Color.rgb = mix(Color.rgb * ObjXYZ * ColorFilter, Color.rgb, Color.rgb);
    Color = max(Color, ColorOrg);
    Color = mix(ColorOrg, Color, Strength * alpha1);

    if (NON_TRANSPARENT == false) {
        Color.a = ColorOrg.a;
    }

    return Color;
}

void main() {
    vec4 Color;
    if (horizontal == 1) {
        Color = passX(TexCoords);
    } else {
        Color = passY(TexCoords);
    }
    FragColor = Color;
}

)glsl";

static float quadVertices[] = {
   // positions   // texCoords
   -1.0f,  1.0f,  0.0f, 1.0f,
   -1.0f, -1.0f,  0.0f, 0.0f,
    1.0f, -1.0f,  1.0f, 0.0f,

   -1.0f,  1.0f,  0.0f, 1.0f,
    1.0f, -1.0f,  1.0f, 0.0f,
    1.0f,  1.0f,  1.0f, 1.0f
};

/* RenderOffScreen::initialize: initialize */
void RenderOffScreen::initialize()
{
   m_shaderProgram = 0;
   m_shaderLoaded = false;
   m_framebuffer = 0;
   m_renderbuffer = 0;
   m_textureColorbuffer = 0;
   m_resolveFramebuffer = 0;
   m_resolvedTexture = 0;
   m_processFramebuffer = 0;
   m_processTextureColorbuffer = 0;
   m_quadVBO = 0;
   m_quadVAO = 0;
   m_buffersActive = false;
   m_buffersRequireInit = false;
   m_width = 0;
   m_height = 0;
   m_enabled = false;
   m_intensity = 0.0f;
   m_scalingFactor = 1.0f;
}

/* RenderOffScreen::clear: free */
void RenderOffScreen::clear()
{
   clearBuffers();
   initialize();
}

/* RenderOffScreen::initBuffers: initialize buffers */
bool RenderOffScreen::initBuffers()
{
   /* generate frame buffer */
   glGenFramebuffers(1, &m_framebuffer);
   glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
   /* generate texture to use for the output of the frame buffer */
   /* use texture unit #3 for texture rendering */
   glActiveTexture(GL_TEXTURE0 + 3);
   glGenTextures(1, &m_textureColorbuffer);
   glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_textureColorbuffer);
   glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, m_width, m_height, GL_TRUE);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_textureColorbuffer, 0);
   /* generate render buffer to render with depth test and stencil test */
   glGenRenderbuffers(1, &m_renderbuffer);
   glBindRenderbuffer(GL_RENDERBUFFER, m_renderbuffer);
   glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, m_width, m_height);
   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderbuffer);

   /* check of the frame buffer was correctly generated */
   if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      /* error */
      MMDAgent_snprintf(m_infoLog, 512, "could not generate frame buffer for off-screen rendering");
      glDeleteRenderbuffers(1, &m_renderbuffer);
      glDeleteTextures(1, &m_textureColorbuffer);
      glDeleteFramebuffers(1, &m_framebuffer);
      return false;
   }
   /* revert to default frame buffer (screen) */
   glBindFramebuffer(GL_FRAMEBUFFER, 0);

   /* prepare multisample resolving frame buffer and texture */
   glGenFramebuffers(1, &m_resolveFramebuffer);
   glBindFramebuffer(GL_FRAMEBUFFER, m_resolveFramebuffer);
   glActiveTexture(GL_TEXTURE0 + 3);
   glGenTextures(1, &m_resolvedTexture);
   glBindTexture(GL_TEXTURE_2D, m_resolvedTexture);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_resolvedTexture, 0);
   if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      /* error */
      MMDAgent_snprintf(m_infoLog, 512, "could not generate frame buffer for resolving");
      glDeleteTextures(1, &m_resolvedTexture);
      glDeleteFramebuffers(1, &m_resolveFramebuffer);
      glDeleteRenderbuffers(1, &m_renderbuffer);
      glDeleteTextures(1, &m_textureColorbuffer);
      glDeleteFramebuffers(1, &m_framebuffer);
      return false;
   }
   glBindFramebuffer(GL_FRAMEBUFFER, 0);

   /* prepare another frame buffer with texture for multi-pass shader */
   glGenFramebuffers(1, &m_processFramebuffer);
   glBindFramebuffer(GL_FRAMEBUFFER, m_processFramebuffer);
   glActiveTexture(GL_TEXTURE0 + 3);
   glGenTextures(1, &m_processTextureColorbuffer);
   glBindTexture(GL_TEXTURE_2D, m_processTextureColorbuffer);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_processTextureColorbuffer, 0);
   if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      /* error */
      MMDAgent_snprintf(m_infoLog, 512, "could not generate frame buffer for resolving");
      glDeleteTextures(1, &m_processTextureColorbuffer);
      glDeleteFramebuffers(1, &m_processFramebuffer);
      glDeleteTextures(1, &m_resolvedTexture);
      glDeleteFramebuffers(1, &m_resolveFramebuffer);
      glDeleteRenderbuffers(1, &m_renderbuffer);
      glDeleteTextures(1, &m_textureColorbuffer);
      glDeleteFramebuffers(1, &m_framebuffer);
      return false;
   }
   glBindFramebuffer(GL_FRAMEBUFFER, 0);

   /* generate VBO and VAO for texture-to-screen post rendering */
   glGenBuffers(1, &m_quadVBO);
   glGenVertexArrays(1, &m_quadVAO);
   glBindVertexArray(m_quadVAO);
   glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
   glEnableVertexAttribArray(1);
   glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
   /* revert to default */
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);

   m_buffersActive = true;
   m_buffersRequireInit = true;

   /* load shader */
   if (loadShader() == false) {
      clear();
      return false;
   }

   /* revert texture unit to default */
   glActiveTexture(GL_TEXTURE0);

   return true;
}

/* RenderOffScreen::clearBuffers: clear buffers */
void RenderOffScreen::clearBuffers()
{
   if (m_shaderLoaded) {
      glDeleteProgram(m_shaderProgram);
   }
   if (m_buffersActive) {
      glDeleteVertexArrays(1, &m_quadVAO);
      glDeleteBuffers(1, &m_quadVBO);
      glDeleteTextures(1, &m_processTextureColorbuffer);
      glDeleteFramebuffers(1, &m_processFramebuffer);
      glDeleteTextures(1, &m_resolvedTexture);
      glDeleteFramebuffers(1, &m_resolveFramebuffer);
      glDeleteRenderbuffers(1, &m_renderbuffer);
      glDeleteTextures(1, &m_textureColorbuffer);
      glDeleteFramebuffers(1, &m_framebuffer);
   }
   m_shaderProgram = 0;
   m_shaderLoaded = false;
   m_framebuffer = 0;
   m_renderbuffer = 0;
   m_textureColorbuffer = 0;
   m_resolveFramebuffer = 0;
   m_resolvedTexture = 0;
   m_processFramebuffer = 0;
   m_processTextureColorbuffer = 0;
   m_quadVBO = 0;
   m_quadVAO = 0;
   m_buffersActive = false;
   m_buffersRequireInit = false;
}

/* RenderOffScreen::setup: set up */
bool RenderOffScreen::setup(int width, int height)
{
   m_width = width;
   m_height = height;

   return true;
}

/* RenderOffScreen::loadShader: load shader programs */
bool RenderOffScreen::loadShader()
{
   GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
   GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
   GLint result = GL_FALSE;

   // compile vertex shader
   glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
   glCompileShader(vertexShader);
   glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
   if (!result) {
      /* error */
      glGetShaderInfoLog(vertexShader, 512, nullptr, m_infoLog);
      return false;
   }

   // compile fragment shader
   glShaderSource(fragmentShader, 1, &fragmentShaderSourceDiffusion, nullptr);
   glCompileShader(fragmentShader);
   glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
   if (!result) {
      /* error */
      glGetShaderInfoLog(fragmentShader, 512, nullptr, m_infoLog);
      return false;
   }

   // link the compiled shader programs
   GLuint programID = glCreateProgram();
   glAttachShader(programID, vertexShader);
   glAttachShader(programID, fragmentShader);
   glLinkProgram(programID);
   glGetProgramiv(programID, GL_LINK_STATUS, &result);
   if (!result) {
      /* error */
      MMDAgent_snprintf(m_infoLog, 512, "failed to link shader programs");
      return false;
   }

   // done loading to GPU, detach and delete the loaded programs
   glDetachShader(programID, vertexShader);
   glDetachShader(programID, fragmentShader);
   glDeleteShader(vertexShader);
   glDeleteShader(fragmentShader);

   m_shaderProgram = programID;
   m_shaderLoaded = true;

   return true;
}

/* RenderOffScreen::RenderOffScreen: constructor */
RenderOffScreen::RenderOffScreen()
{
   initialize();
}

/* RenderOffScreen::~RenderOffScreen: destructor */
RenderOffScreen::~RenderOffScreen()
{
   clear();
}

/* RenderOffScreen::getInfoLog: get info log */
const char *RenderOffScreen::getInfoLog()
{
   return m_infoLog;
}

/* RenderOffScreen::start: start off-screen rendering */
void RenderOffScreen::start()
{
   if (m_enabled == false)
      return;
   if (m_buffersActive == false || m_shaderLoaded == false)
      return;

   glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
}

/* RenderOffScreen::finish: finish off-screen rendering */
void RenderOffScreen::finish()
{
   if (m_enabled == false)
      return;
   if (m_buffersActive == false || m_shaderLoaded == false)
      return;

   /* draw rendered texture back to screen */
   /* multisample resolving */
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_resolveFramebuffer);
   glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffer);
   glBlitFramebuffer(0, 0,m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

   /* tell GPU to activate shader for the following drawings */
   glUseProgram(m_shaderProgram);
   GLint textureLocation = glGetUniformLocation(m_shaderProgram, "screenTexture");
   GLint processedTextureLocation = glGetUniformLocation(m_shaderProgram, "processedTexture");
   GLint resolutionLocation = glGetUniformLocation(m_shaderProgram, "resolution");
   GLint strengthLocation = glGetUniformLocation(m_shaderProgram, "Strength");
   GLint scalingLocation = glGetUniformLocation(m_shaderProgram, "scaling");
   GLint horizontalLocation = glGetUniformLocation(m_shaderProgram, "horizontal");
   glUniform1i(textureLocation, 3);
   glUniform1i(processedTextureLocation, 2);
   glUniform2f(resolutionLocation, static_cast<float>(m_width), static_cast<float>(m_height));
   glUniform1f(strengthLocation, m_intensity);
   glUniform1f(scalingLocation, m_scalingFactor);

   /* process resolved texture to generate processed texture */
   glUniform1i(horizontalLocation, 1);
   glBindFramebuffer(GL_FRAMEBUFFER, m_processFramebuffer);
   glClear(GL_COLOR_BUFFER_BIT);
   glActiveTexture(GL_TEXTURE0 + 3);
   glBindTexture(GL_TEXTURE_2D, m_resolvedTexture);
   glBindVertexArray(m_quadVAO);
   glDrawArrays(GL_TRIANGLES, 0, 6);

   /* revert to screen buffer */
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   /* clear screen */
   glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   /* disable unused tests for performance */
   glDisable(GL_DEPTH_TEST);
   /* render texture to screen width resolved texture and processed texture */
   glUniform1i(horizontalLocation, 0);
   glActiveTexture(GL_TEXTURE0 + 3);
   glBindTexture(GL_TEXTURE_2D, m_resolvedTexture);
   glActiveTexture(GL_TEXTURE0 + 2);
   glBindTexture(GL_TEXTURE_2D, m_processTextureColorbuffer);
   glBindVertexArray(m_quadVAO);
   glDrawArrays(GL_TRIANGLES, 0, 6);
   /* revert to default settings */
   glBindTexture(GL_TEXTURE_2D, 0);
   glActiveTexture(GL_TEXTURE0);
   glUseProgram(0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);
   glEnable(GL_DEPTH_TEST);
}

/* RenderOffScreen::getFrameBufferId: get frame buffer id */
GLuint RenderOffScreen::getFrameBufferId()
{
   return m_framebuffer;
}

/* RenderOffScreen::getFrameBufferRequireInit: return flag if the current frame buffer requires surface initialization */
bool RenderOffScreen::getFrameBufferRequireInit()
{
   return m_buffersRequireInit;
}

/* RenderOffScreen::setFrameBufferRequireInit: set flag if the current frame buffer requires surface initialization */
void RenderOffScreen::setFrameBufferRequireInit(bool flag)
{
   m_buffersRequireInit = flag;
}

/* RenderOffScreen::changeScreenSize: change screen size */
void RenderOffScreen::changeScreenSize(int width, int height)
{
   m_width = width;
   m_height = height;

   if (m_buffersActive == false || m_shaderLoaded == false)
      return;
   if (m_enabled == false)
      return;

   glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
   glActiveTexture(GL_TEXTURE0 + 3);
   glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_textureColorbuffer);
   glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, m_width, m_height, GL_TRUE);

   /* generate render buffer to render with depth test and stencil test */
   glBindRenderbuffer(GL_RENDERBUFFER, m_renderbuffer);
   glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, m_width, m_height);
   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderbuffer);

   glBindFramebuffer(GL_FRAMEBUFFER, m_resolveFramebuffer);
   glActiveTexture(GL_TEXTURE0 + 3);
   glBindTexture(GL_TEXTURE_2D, m_resolvedTexture);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_resolvedTexture, 0);

   glBindFramebuffer(GL_FRAMEBUFFER, m_processFramebuffer);
   glActiveTexture(GL_TEXTURE0 + 3);
   glBindTexture(GL_TEXTURE_2D, m_processTextureColorbuffer);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_processTextureColorbuffer, 0);

   /* revert to default frame buffer (screen) */
   glBindFramebuffer(GL_FRAMEBUFFER, 0);

   /* revert texture unit to default */
   glActiveTexture(GL_TEXTURE0);

}

/* RenderOffScreen::getIntensity: get intensity */
float RenderOffScreen::getIntensity()
{
   return m_intensity;
}

/* RenderOffScreen::setIntensity: set intensity */
void RenderOffScreen::setIntensity(float f)
{
   m_intensity = f;
   if (m_enabled == false && m_intensity > 0.0f) {
      m_enabled = true;
      initBuffers();
   } else if (m_enabled == true && m_intensity == 0.0f) {
      clearBuffers();
      m_enabled = false;
   }
}

/* RenderOffScreen::toggleIntensity: toggle between a set of intensity values */
void RenderOffScreen::toggleIntensity()
{
   if (m_intensity == 0.0f) {
      m_enabled = true;
      initBuffers();
   }
   m_intensity += 0.2f;
   if (m_intensity > 1.0f) {
      clearBuffers();
      m_enabled = false;
      m_intensity = 0.0f;
   }
}

/* RenderOffScreen::getScaling: get scaling */
float RenderOffScreen::getScaling()
{
   return m_scalingFactor;
}

/* RenderOffScreen::setScaling: set scaling */
void RenderOffScreen::setScaling(float f)
{
   m_scalingFactor = f;
}

/* RenderOffScreen::toggleScaling: toggle between a set of scaling values */
void RenderOffScreen::toggleScaling()
{
   m_scalingFactor *= 1.4f;
   if (m_scalingFactor > 5.0f) {
      m_scalingFactor = 1.0f;
   }
}
