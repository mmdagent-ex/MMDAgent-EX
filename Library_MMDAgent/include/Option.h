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

/* definitions */

#define OPTION_USECARTOONRENDERING_STR "use_cartoon_rendering"
#define OPTION_USECARTOONRENDERING_DEF true

#define OPTION_USEMMDLIKECARTOON_STR "use_mmd_like_cartoon"
#define OPTION_USEMMDLIKECARTOON_DEF true

#define OPTION_CARTOONEDGEWIDTH_STR "cartoon_edge_width"
#define OPTION_CARTOONEDGEWIDTH_DEF 0.7f
#define OPTION_CARTOONEDGEWIDTH_MAX 1000.0f
#define OPTION_CARTOONEDGEWIDTH_MIN 0.001f

#define OPTION_CARTOONEDGESTEP_STR "cartoon_edge_step"
#define OPTION_CARTOONEDGESTEP_DEF 1.2f
#define OPTION_CARTOONEDGESTEP_MAX 10.0f
#define OPTION_CARTOONEDGESTEP_MIN 1.0f

#define OPTION_CARTOONEDGESELECTEDCOLOR_STR  "cartoon_edge_selected_color"
#define OPTION_CARTOONEDGESELECTEDCOLORR_DEF 1.0f
#define OPTION_CARTOONEDGESELECTEDCOLORG_DEF 0.0f
#define OPTION_CARTOONEDGESELECTEDCOLORB_DEF 0.0f
#define OPTION_CARTOONEDGESELECTEDCOLORA_DEF 1.0f
#define OPTION_CARTOONEDGESELECTEDCOLOR_MAX  1.0f
#define OPTION_CARTOONEDGESELECTEDCOLOR_MIN  0.0f

#define OPTION_CAMERAROTATION_STR  "camera_rotation"
#define OPTION_CAMERAROTATIONX_DEF 0.0f
#define OPTION_CAMERAROTATIONY_DEF 0.0f
#define OPTION_CAMERAROTATIONZ_DEF 0.0f
#define OPTION_CAMERAROTATION_MAX  1000.0f
#define OPTION_CAMERAROTATION_MIN  0.001f

#define OPTION_CAMERATRANSITION_STR  "camera_transition"
#define OPTION_CAMERATRANSITIONX_DEF 0.0f
#define OPTION_CAMERATRANSITIONY_DEF 13.0f
#define OPTION_CAMERATRANSITIONZ_DEF 0.0f
#define OPTION_CAMERATRANSITION_MAX  10000.0f
#define OPTION_CAMERATRANSITION_MIN  -10000.0f

#define OPTION_CAMERADISTANCE_STR "camera_distance"
#define OPTION_CAMERADISTANCE_DEF 100.0f
#define OPTION_CAMERADISTANCE_MAX 100000.0f
#define OPTION_CAMERADISTANCE_MIN 0.0f

#define OPTION_CAMERAFOVY_STR "camera_fovy"
#define OPTION_CAMERAFOVY_DEF 16.0f
#define OPTION_CAMERAFOVY_MAX 180.0f
#define OPTION_CAMERAFOVY_MIN 0.0f

#define OPTION_STAGESIZE_STR  "stage_size"
#define OPTION_STAGESIZEW_DEF 25.0f
#define OPTION_STAGESIZED_DEF 25.0f
#define OPTION_STAGESIZEH_DEF 40.0f
#define OPTION_STAGESIZE_MAX  1000.0f
#define OPTION_STAGESIZE_MIN  0.001f

#define OPTION_SHOWFPS_STR "show_fps"
#define OPTION_SHOWFPS_DEF true

#define OPTION_WINDOWSIZE_STR "window_size"
#define OPTION_WINDOWSIZEW_DEF 1024
#define OPTION_WINDOWSIZEH_DEF 900
#define OPTION_WINDOWSIZE_MAX  16384
#define OPTION_WINDOWSIZE_MIN  1

#define OPTION_FULLSCREEN_STR "full_screen"
#define OPTION_FULLSCREEN_DEF false

#define OPTION_LOGSIZE_STR  "log_size"
#define OPTION_LOGSIZEW_DEF 80
#define OPTION_LOGSIZEH_DEF 30
#define OPTION_LOGSIZE_MAX  4096
#define OPTION_LOGSIZE_MIN  1

#define OPTION_LOGPOSITION_STR  "log_position"
#define OPTION_LOGPOSITIONX_DEF -17.5f
#define OPTION_LOGPOSITIONY_DEF 3.0f
#define OPTION_LOGPOSITIONZ_DEF -15.0f

#define OPTION_LOGSCALE_STR "log_scale"
#define OPTION_LOGSCALE_DEF 1.0f
#define OPTION_LOGSCALE_MAX 1000.0f
#define OPTION_LOGSCALE_MIN 0.001f

#define OPTION_LIGHTDIRECTION_STR  "light_direction"
#define OPTION_LIGHTDIRECTIONX_DEF 0.5f
#define OPTION_LIGHTDIRECTIONY_DEF 1.0f
#define OPTION_LIGHTDIRECTIONZ_DEF 0.5f
#define OPTION_LIGHTDIRECTIONI_DEF 0.0f

#define OPTION_LIGHTINTENSITY_STR "light_intensity"
#define OPTION_LIGHTINTENSITY_DEF 0.6f
#define OPTION_LIGHTINTENSITY_MAX 1.0f
#define OPTION_LIGHTINTENSITY_MIN 0.0f

#define OPTION_LIGHTCOLOR_STR  "light_color"
#define OPTION_LIGHTCOLORR_DEF 1.0f
#define OPTION_LIGHTCOLORG_DEF 1.0f
#define OPTION_LIGHTCOLORB_DEF 1.0f
#define OPTION_LIGHTCOLOR_MAX  1.0f
#define OPTION_LIGHTCOLOR_MIN  0.0f

#define OPTION_CAMPUSCOLOR_STR  "campus_color"
#define OPTION_CAMPUSCOLORR_DEF 0.0f
#define OPTION_CAMPUSCOLORG_DEF 0.0f
#define OPTION_CAMPUSCOLORB_DEF 0.2f
#define OPTION_CAMPUSCOLOR_MAX  1.0f
#define OPTION_CAMPUSCOLOR_MIN  0.0f

#define OPTION_MAXMULTISAMPLING_STR "max_multi_sampling"
#define OPTION_MAXMULTISAMPLING_DEF 4
#define OPTION_MAXMULTISAMPLING_MAX 32
#define OPTION_MAXMULTISAMPLING_MIN 0

#define OPTION_MOTIONADJUSTTIME_STR "motion_adjust_time"
#define OPTION_MOTIONADJUSTTIME_DEF 0.0f
#define OPTION_MOTIONADJUSTTIME_MAX 10.0f
#define OPTION_MOTIONADJUSTTIME_MIN -10.0f

#define OPTION_LIPSYNCPRIORITY_STR "lipsync_priority"
#define OPTION_LIPSYNCPRIORITY_DEF 100.0f
#define OPTION_LIPSYNCPRIORITY_MAX 1000.0f
#define OPTION_LIPSYNCPRIORITY_MIN -1000.0f

#define OPTION_BULLETFPS_STR "bullet_fps"
#if defined(__ANDROID__) || TARGET_OS_IPHONE
#define OPTION_BULLETFPS_DEF 60
#else
#define OPTION_BULLETFPS_DEF 120
#endif
#define OPTION_BULLETFPS_MAX 120
#define OPTION_BULLETFPS_MIN 1

#define OPTION_GRAVITYFACTOR_STR "gravity_factor"
#define OPTION_GRAVITYFACTOR_DEF 10.0f
#define OPTION_GRAVITYFACTOR_MAX 1024.0f
#define OPTION_GRAVITYFACTOR_MIN 0.0f

#define OPTION_ROTATESTEP_STR "rotate_step"
#define OPTION_ROTATESTEP_DEF 4.5f
#define OPTION_ROTATESTEP_MAX 180.0f
#define OPTION_ROTATESTEP_MIN 0.001f

#define OPTION_TRANSLATESTEP_STR "translate_step"
#define OPTION_TRANSLATESTEP_DEF 0.5f
#define OPTION_TRANSLATESTEP_MAX 1000.0f
#define OPTION_TRANSLATESTEP_MIN 0.001f

#define OPTION_DISTANCESTEP_STR "distance_step"
#define OPTION_DISTANCESTEP_DEF 4.0f
#define OPTION_DISTANCESTEP_MAX 1000.0f
#define OPTION_DISTANCESTEP_MIN 0.001f

#define OPTION_FOVYSTEP_STR "fovy_step"
#define OPTION_FOVYSTEP_DEF 1.0f
#define OPTION_FOVYSTEP_MAX 1000.0f
#define OPTION_FOVYSTEP_MIN 0.001f

#define OPTION_USESHADOW_STR "use_shadow"
#define OPTION_USESHADOW_DEF true

#define OPTION_SHADOWDENSITY_STR "shadow_density"
#define OPTION_SHADOWDENSITY_DEF 0.5f
#define OPTION_SHADOWDENSITY_MAX 1.0f
#define OPTION_SHADOWDENSITY_MIN 0.0f

#define OPTION_USESHADOWMAPPING_STR "use_shadow_mapping"
#define OPTION_USESHADOWMAPPING_DEF false

#define OPTION_SHADOWMAPPINGTEXTURESIZE_STR "shadow_mapping_texture_size"
#define OPTION_SHADOWMAPPINGTEXTURESIZE_DEF 1024
#define OPTION_SHADOWMAPPINGTEXTURESIZE_MAX 8192
#define OPTION_SHADOWMAPPINGTEXTURESIZE_MIN 1

#define OPTION_SHADOWMAPPINGSELFDENSITY_STR "shadow_mapping_self_density"
#define OPTION_SHADOWMAPPINGSELFDENSITY_DEF 1.0f
#define OPTION_SHADOWMAPPINGSELFDENSITY_MAX 1.0f
#define OPTION_SHADOWMAPPINGSELFDENSITY_MIN 0.0f

#define OPTION_SHADOWMAPPINGFLOORDENSITY_STR "shadow_mapping_floor_density"
#define OPTION_SHADOWMAPPINGFLOORDENSITY_DEF 0.5f
#define OPTION_SHADOWMAPPINGFLOORDENSITY_MAX 1.0f
#define OPTION_SHADOWMAPPINGFLOORDENSITY_MIN 0.0f

#define OPTION_SHADOWMAPPINGLIGHTFIRST_STR "shadow_mapping_light_first" /* bogus */

#define OPTION_DISPLAYCOMMENTTIME_STR "display_comment_time"
#define OPTION_DISPLAYCOMMENTTIME_DEF 0.0f
#define OPTION_DISPLAYCOMMENTTIME_MAX 30.0f
#define OPTION_DISPLAYCOMMENTTIME_MIN 0.0f

#define OPTION_MAXNUMMODEL_STR "max_num_model"
#define OPTION_MAXNUMMODEL_DEF 10
#define OPTION_MAXNUMMODEL_MAX 1024
#define OPTION_MAXNUMMODEL_MIN 1

#define OPTION_LOGFILE_STR "log_file"
#define OPTION_LOGFILE_DEF ""

#define OPTION_BULLETFLOOR_STR "bullet_floor"
#define OPTION_BULLETFLOOR_DEF true

#define OPTION_SHOWBUTTON_STR "show_button"
#define OPTION_SHOWBUTTON_DEF true

#define OPTION_DOPPELSHADOW_STR "doppel_shadow"
#define OPTION_DOPPELSHADOW_DEF false

#define OPTION_DOPPELSHADOWCOLOR_STR  "doppel_shadow_color"
#define OPTION_DOPPELSHADOWCOLORR_DEF 0.5f
#define OPTION_DOPPELSHADOWCOLORG_DEF 0.5f
#define OPTION_DOPPELSHADOWCOLORB_DEF 0.5f
#define OPTION_DOPPELSHADOWCOLOR_MAX  1.0f
#define OPTION_DOPPELSHADOWCOLOR_MIN  0.0f

#define OPTION_LDOPPELSHADOWOFFSET_STR  "doppel_shadow_offset"
#define OPTION_LDOPPELSHADOWOFFSET_X_DEF -0.5f
#define OPTION_LDOPPELSHADOWOFFSET_Y_DEF -0.3f
#define OPTION_LDOPPELSHADOWOFFSET_Z_DEF 0.0f

#define OPTION_DIFFUSIONFILTER_STR "diffusion_postfilter"
#define OPTION_DIFFUSIONFILTER_DEF false

#define OPTION_DIFFUSIONFILTERINTENSITY_STR "diffusion_postfilter_intensity"
#define OPTION_DIFFUSIONFILTERINTENSITY_DEF 0.6f
#define OPTION_DIFFUSIONFILTERINTENSITY_MAX 1.0f
#define OPTION_DIFFUSIONFILTERINTENSITY_MIN 0.0f

#define OPTION_DIFFUSIONFILTERSCALE_STR "diffusion_postfilter_scale"
#define OPTION_DIFFUSIONFILTERSCALE_DEF 1.0f
#define OPTION_DIFFUSIONFILTERSCALE_MAX 5.0f
#define OPTION_DIFFUSIONFILTERSCALE_MIN 0.0f

#define OPTION_PARALLELSKINNING_STR "parallel_skinning_numthreads"
#define OPTION_PARALLELSKINNING_DEF 1
#define OPTION_PARALLELSKINNING_MAX 4
#define OPTION_PARALLELSKINNING_MIN 1

#define OPTION_HTTPSERVER_STR "http_server"
#define OPTION_HTTPSERVER_DEF true

#define OPTION_HTTPSERVERPORT_STR "http_server_port"
#define OPTION_HTTPSERVERPORT_DEF 50000
#define OPTION_HTTPSERVERPORT_MAX 65535
#define OPTION_HTTPSERVERPORT_MIN 0


/* Option: user options */
class Option
{
private:

   char errmsg[MMDAGENT_MAXBUFLEN];

   /* cartoon rendering */
   bool m_useCartoonRendering;
   bool m_useMMDLikeCartoon;
   float m_cartoonEdgeWidth;
   float m_cartoonEdgeStep;
   float m_cartoonEdgeSelectedColor[4];

   /* camera viewpoint parameters */
   float m_cameraRotation[3];
   float m_cameraTransition[3];
   float m_cameraDistance;
   float m_cameraFovy;

   /* stage */
   float m_stageSize[3];

   /* fps */
   bool m_showFps;
   float m_fpsPosition[3];

   /* window */
   int m_windowSize[2];
   bool m_fullScreen;

   /* log */
   int m_logSize[2];
   float m_logPosition[3];
   float m_logScale;

   /* light */
   float m_lightDirection[4];
   float m_lightIntensity;
   float m_lightColor[3];

   /* campus */
   float m_campusColor[3];

   /* OpenGL */
   int m_maxMultiSampling;

   /* motion */
   float m_motionAdjustTime;
   float m_lipsyncPriority;

   /* bullet physics */
   int m_bulletFps;
   bool m_bulletFloor;

   /* gravity scale */
   float m_gravityFactor;

   /* move */
   float m_rotateStep;
   float m_translateStep;
   float m_distanceStep;
   float m_fovyStep;

   /* shadow */
   bool m_useShadow;
   float m_shadowDensity;

   /* shadow mapping */
   bool m_useShadowMapping;
   int m_shadowMapTextureSize;
   float m_shadowMapSelfDensity;
   float m_shadowMapFloorDensity;

   /* comment */
   float m_displayCommentTime;

   /* model */
   int m_maxNumModel;

   /* log */
   char *m_logFile;

   /* button */
   bool m_showButton;

   /* doppel shadow effect */
   bool m_useDoppelShadow;
   float m_doppelShadowColor[3];
   float m_doppelShadowOffset[3];

   /* diffusion postfilter effect */
   bool m_useDiffusionFilter;
   float m_diffusionFilterIntensity;
   float m_diffusionFilterScale;

   /* parallel skinning */
   int m_parallelSkinningNumThreads;

   /* http server */
   bool m_useHttpServer;
   int m_httpServerPortNumber;

   /* initialize: initialize options */
   void initialize();

public:

   /* Option: constructor */
   Option();

   /* ~Option: destructor */
   ~Option();

   /* load: load options */
   bool load(const char *file, ZFileKey *key, char **errstr);

   /* getUseCartoonRendering: get cartoon rendering flag */
   bool getUseCartoonRendering();

   /* setUseCartoonRendering: set cartoon rendering flag */
   void setUseCartoonRendering(bool b);

   /* getUseMMDLikeCartoon: get MikuMikuDance like cartoon flag */
   bool getUseMMDLikeCartoon();

   /* setUseMMDLikeCartoon: set MikuMikuDance like cartoon flag */
   void setUseMMDLikeCartoon(bool b);

   /* getCartoonEdgeWidth: get edge width for catoon */
   float getCartoonEdgeWidth();

   /* setCartoonEdgeWidth: set edge width for catoon */
   void setCartoonEdgeWidth(float f);

   /* getCartoonEdgeStep: get cartoon edge step */
   float getCartoonEdgeStep();

   /* setCartoonEdgeStep: set cartoon edge step */
   void setCartoonEdgeStep(float f);

   /* getCartoonEdgeSelectedColor: get cartoon edge seleceted color */
   float *getCartoonEdgeSelectedColor();

   /* setCartoonEdgeSelectedColor: set cartoon edge seleceted color */
   void setCartoonEdgeSelectedColor(const float *f);

   /* getCameraRotation: get camera rotation */
   float *getCameraRotation();

   /* setCameraRotation: set camera rotation */
   void setCameraRotation(const float *f);

   /* getCameraTransition: get camera transition */
   float *getCameraTransition();

   /* setCameraTransition: set camera transition */
   void setCameraTransition(const float *f);

   /* getCameraDistance: get camera distance */
   float getCameraDistance();

   /* setCameraDistance: set camera distance */
   void setCameraDistance(float f);

   /* getCameraFovy: get camera fovy */
   float getCameraFovy();

   /* setCameraFovy: set camera fovy */
   void setCameraFovy(float f);

   /* getStageSize: get stage size */
   float *getStageSize();

   /* setStageSize: set stage size */
   void setStageSize(const float *f);

   /* getShowFps: get fps flag */
   bool getShowFps();

   /* setShowFps: set fps flag */
   void setShowFps(bool b);

   /* getWindowSize: get window size */
   int *getWindowSize();

   /* setWindowSize: set window size */
   void setWindowSize(const int *i);

   /* getFullScreen: get full screen flag */
   bool getFullScreen();

   /* setFullScreen: set full screen flag */
   void setFullScreen(bool b);

   /* getLogSize: get log window size */
   int* getLogSize();

   /* setLogSize: set log window size */
   void setLogSize(const int *i);

   /* getLogPosition: get log window position */
   float *getLogPosition();

   /* setLogPosition: set log window position */
   void setLogPosition(const float *f);

   /* getLogScale: get log window scale */
   float getLogScale();

   /* setLogScale: set log window scale */
   void setLogScale(float f);

   /* getLogDirection: get light direction */
   float *getLightDirection();

   /* setLogDirection: set light direction */
   void setLightDirection(const float *f);

   /* getLogIntensity: get light intensity */
   float getLightIntensity();

   /* setLogIntensity: set light intensity */
   void setLightIntensity(float f);

   /* getLightColor: get light color */
   float *getLightColor();

   /* setLightColor: set light color */
   void setLightColor(const float *f);

   /* getCampusColor: get campus color */
   float *getCampusColor();

   /* setCampusColor: set campus color */
   void setCampusColor(const float *f);

   /* getMaxMultiSampling: get max number of multi sampling */
   int getMaxMultiSampling();

   /* setMaxMultiSampling: set max number of multi sampling */
   void setMaxMultiSampling(int i);

   /* getMotionAdjustTime: get motion adjust time in sec */
   float getMotionAdjustTime();

   /* setMotionAdjustTime: set motion adjust time in sec */
   void setMotionAdjustTime(float f);

   /* getLipsyncPriority: get lipsync motion priority */
   float getLipsyncPriority();

   /* setLipsyncPriority: set lipsync motion priority */
   void setLipsyncPriority(float f);

   /* getBulletFps: get bullet fps */
   int getBulletFps();

   /* setBulletFps: set bullet fps */
   void setBulletFps(int i);

   /* setGravityFactor: set gravity factor */
   void setGravityFactor(float f);

   /* getGravityFactor: get gravity factor */
   float getGravityFactor();

   /* getRotateStep: get rotate step */
   float getRotateStep();

   /* setRotateStep: set rotate step */
   void setRotateStep(float f);

   /* getTranslateStep: get translate step */
   float getTranslateStep();

   /* setTranslateStep: set translate step */
   void setTranslateStep(float f);

   /* getDistanceStep: get distance step */
   float getDistanceStep();

   /* setDistanceStep: set distance step */
   void setDistanceStep(float f);

   /* getFovyStep: get fovy step */
   float getFovyStep();

   /* setFovyStep: set fovy step */
   void setFovyStep(float f);

   /* getUseShadow: get shadow flag */
   bool getUseShadow();

   /* setUseShadow: set shadow flag */
   void setUseShadow(bool b);

   /* getShadowDensity: get shadow dentisy */
   float getShadowDensity();

   /* setShadowDensity: set shadow dentisy */
   void setShadowDensity(float f);

   /* getUseShadowMapping: get shadow mapping flag */
   bool getUseShadowMapping();

   /* setUseShadowMapping: set shadow mapping flag */
   void setUseShadowMapping(bool b);

   /* getShadowMappingTextureSize: get texture size of shadow mapping */
   int getShadowMappingTextureSize();

   /* setShadowMappingTextureSize: set texture size of shadow mapping */
   void setShadowMappingTextureSize(int i);

   /* getShadowMappingSelfDensity: get self density of shadow mapping */
   float getShadowMappingSelfDensity();

   /* setShadowMappingSelfDensity: set self density of shadow mapping */
   void setShadowMappingSelfDensity(float f);

   /* getShadowMappingFloorDensity: get floor density of shadow mapping */
   float getShadowMappingFloorDensity();

   /* setShadowMappingFloorDensity: set floor density of shadow mapping */
   void setShadowMappingFloorDensity(float f);

   /* getDisplayCommentTime: get display comment time in sec */
   float getDisplayCommentTime();

   /* setDisplayCommentTime: set display comment time in sec */
   void setDisplayCommentTime(float f);

   /* getMaxNumModel: get maximum number of models */
   int getMaxNumModel();

   /* setMaxNumModel: set maximum number of models */
   void setMaxNumModel(int i);

   /* getLogFile: get log file name */
   char *getLogFile();

   /* setLogFile: set log file name */
   void setLogFile(char *filepath);

   /* getBulletFloor: get bullet floor flag */
   bool getBulletFloor();

   /* setBulletFloor: set bullet floor flag */
   void setBulletFloor(bool b);

   /* getShowButton: get show button flag */
   bool getShowButton();

   /* setShowButton: set show button flag */
   void setShowButton(bool b);

   /* getUseDoppelShadow: get use of doppel shadow */
   bool getUseDoppelShadow();

   /* setUseDoppelShadow: set use of doppel shadow */
   void setUseDoppelShadow(bool b);

   /* getDoppelShadowColor: get doppel shadow color */
   const float *getDoppelShadowColor();

   /* setDoppelShadowColor: set doppel shadow color */
   void setDoppelShadowColor(const float *f);

   /* getDoppelShadowOffset: get doppel shadow offset */
   const float *getDoppelShadowOffset();

   /* setDoppelShadowOffset: set doppel shadow offset */
   void setDoppelShadowOffset(const float *f);

   /* getUseDiffusionFilter: get use of diffusion filter */
   bool getUseDiffusionFilter();

   /* setUseDiffusionFilter: set use of diffusion filter */
   void setUseDiffusionFilter(bool b);

   /* getDiffusionFilterIntensity: get intensity of diffusion filter */
   float getDiffusionFilterIntensity();

   /* setDiffusionFilterIntensity: set intensity of diffusion filter */
   void setDiffusionFilterIntensity(float f);

   /* getDiffusionFilterScale: get scale of diffusion filter */
   float getDiffusionFilterScale();

   /* setDiffusionFilterScale: set scale of diffusion filter */
   void setDiffusionFilterScale(float f);

   /* getParallelSkinningNumthreads: get parallel skinning numthreads */
   int getParallelSkinningNumthreads();

   /* setParallelSkinningNumthreads: set parallel skinning numthreads */
   void setParallelSkinningNumthreads(int i);

   /* getUseHttpServer: get use of http server */
   bool getUseHttpServer();

   /* setUseHttpServer: set use of http server */
   void setUseHttpServer(bool b);

   /* getHttpServerPortNumber: get http server port number */
   int getHttpServerPortNumber();

   /* setHttpServerPortNumber: set http server port number */
   void setHttpServerPortNumber(int i);
};
