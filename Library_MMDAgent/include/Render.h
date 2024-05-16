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

#define RENDER_SHADOWPCF                 /* use hardware PCF for shadow mapping */
#define RENDER_SHADOWAUTOVIEW            /* automatically define depth frustum */
#define RENDER_SHADOWAUTOVIEWANGLE 15.0f /* view angle for automatic depth frustum */

#define RENDER_MINMOVEDIFF       0.000001f
#define RENDER_MOVESPEEDRATE     0.9f
#define RENDER_MINSPINDIFF       0.000001f
#define RENDER_SPINSPEEDRATE     0.9f
#define RENDER_MINDISTANCEDIFF   0.1f
#define RENDER_DISTANCESPEEDRATE 0.9f
#define RENDER_MINFOVYDIFF       0.01f
#define RENDER_FOVYSPEEDRATE     0.9f
#ifdef __ANDROID__
#define RENDER_VIEWPOINTFRUSTUMNEAR 5.0f
#else
#define RENDER_VIEWPOINTFRUSTUMNEAR 0.5f
#endif /* __ANDROID__ */
#define RENDER_VIEWPOINTFRUSTUMFAR  500.0f

#ifdef MY_LUMINOUS
#define LUMINOUS_FBO_UNINITIALIZEDID 0xFFFFFFFF
#endif

/* RenderDepthData: depth data for model ordering */
typedef struct {
   float dist;
   short   id;
} RenderDepthData;

/* Render: render */
class Render
{
private:

   int m_width;             /* window width */
   int m_height;            /* winodw height */

   btVector3 m_trans_set;       /* view trans vector given last */
   btVector3 m_angle_set;       /* view angles given last */
   float m_distance_set;        /* view distance given last */
   float m_fovy_set;            /* view fovy given last */
   PMDBone *m_baseBone_set;     /* camera base bone given last */
   char *m_baseBoneName;        /* camera base bone name */

   btVector3 m_trans;       /* view trans vector */
   btVector3 m_angle;       /* view angles */
   btQuaternion m_rot;      /* view rotation */
   float m_distance;        /* view distance */
   float m_fovy;            /* view fovy */
   PMDBone *m_baseBone;     /* camera base bone */
   bool m_modelFollow;      /* model follow mode */
   btVector3 m_lastFollowPos;

   btVector3 m_currentTrans;     /* current view trans vector */
   btQuaternion m_currentRot;    /* current view rotation */
   float m_currentDistance;      /* current view distance */
   float m_currentFovy;          /* current view fovy */
   btTransform m_transMatrix;    /* current trans vector + rotation matrix */
   btTransform m_transMatrixInv; /* current trans vector + inverse of rotation matrix */
   btScalar m_rotMatrix[16];     /* current rotation + OpenGL rotation matrix */
   btScalar m_rotMatrixInv[16];  /* current rotation + inverse of OpenGL rotation matrix */

   double m_viewMoveTime;           /* view length in sec */
   bool m_viewControlledByMotion;   /* true when view is controlled by motion */
   btVector3 m_viewMoveStartTrans;  /* transition at start of view move */
   btQuaternion m_viewMoveStartRot; /* rotation at start of view move */
   float m_viewMoveStartDistance;   /* distance at start of view move */
   float m_viewMoveStartFovy;       /* distance at start of view move */

   float m_backgroundColor[3]; /* background color */

   bool m_useShadow;                      /* true if use shadow */
   bool m_shadowMapInitialized;           /* true if initialized */
   GLuint m_depthTextureID;               /* depth texture for FBO */
   GLuint m_fboID;                        /* frame buffer object name */
   btVector3 m_lightVec;                  /* light vector for shadow maapping */
   btVector3 m_shadowMapAutoViewEyePoint; /* view point of shadow mapping */
   float m_shadowMapAutoViewRadius;       /* radius from view point */
   float m_viewPointFrustumNear;          /* view point frustum near */

#ifdef MY_LUMINOUS
   bool m_existLuminousModel;      /* true if a model as at least one auto-luminous material */
   GLuint m_bloomFboID;            /* FBO id for light bloom rendering */
   GLuint m_bloomTextureID;        /* Texture id used in light bloom rendering */
   GLuint m_bloomRboID;            /* RBO id for light bloom rendering */
   int m_bloomTextureWidth;        /* texture width for bloom rendering */
   int m_bloomTextureHeight;       /* texture height for bloom rendering */
   float m_bloomIntensity;         /* bloom intensity (default = 1.0) */
#endif

   RenderDepthData *m_depth;              /* depth data of each model for reordering */

   float m_doppelShadowColor[3];   /* doppel shadow color */
   float m_doppelShadowOffset[3];  /* doppel shadow offset */
   bool m_doppelShadowFlag;        /* doppel shadow flag */

   GLuint m_defaultFrameBuffer;    /* default frame buffer to render */

   bool m_backgroundTransparency;


   /* applyProjectionMatirx: update projection matrix */
   void applyProjectionMatrix(double nearVal, double farVal);

   /* updateModelViewMatrix: update model view matrix */
   void updateModelViewMatrix();

   /* updateTransRotMatrix:  update trans and rotation matrix */
   bool updateTransRotMatrix(double ellapsedTimeForMove);

   /* updateRotationFromAngle: update rotation quaternion from angle */
   void updateRotationFromAngle();

   /* updateDistance: update distance */
   bool updateDistance(double ellapsedTimeForMove);

   /* updateFovy: update fovy */
   bool updateFovy(double ellapsedTimeForMove);

   /* initializeShadowMap: initialize OpenGL for shadow mapping */
   void initializeShadowMap(int textureSize);

   /* renderSceneShadowMap: shadow mapping */
   void renderSceneShadowMap(PMDObject *objs, const int *order, int num, Stage *stage, bool useMMDLikeCartoon, bool useCartoonRendering, float lightIntensity, const float *lightDirection, const float *lightColor, int shadowMappingTextureSize, float shadowMappingSelfDensity);

   /* renderScene: render scene */
   void renderScene(PMDObject *objs, const int *order, int num, Stage *stage, bool useMMDLikeCartoon, bool useCartoonRendering, float lightIntensity, const float *lightDirection, const float *lightColor, float shadowMappingFloorDensity);

#ifdef MY_LUMINOUS
   /* initBloomRendering: initialize for bloom effect rendering */
   void initBloomRendering();

   /* clearBloomRendering: clear bloom effect rendering */
   void clearBloomRendering();

   /* updateBloomRendering: update texture for bloom effect rendering */
   void updateBloomRendering();

   /* renderBloomTexture: render bloom texture and apply to screen */
   void renderBloomTexture(PMDObject *objs, const int *order, int num, Stage *stage, bool useMMDLikeCartoon, bool useCartoonRendering, float lightIntensity, const float *lightDirection, const float *lightColor);

   /* updateLuminous: update scene status for luminous rendering */
   void updateLuminous(PMDObject *objs, int num, Stage *stage);
#endif

   /* initialize: initialzie Render */
   void initialize();

   /* clear: free Render */
   void clear();

public:

   /* Render: constructor */
   Render();

   /* ~Render: destructor */
   ~Render();

	/* initSurface: initialize rendering surface */
   bool initSurface();

   /* setup: initialize and setup Render */
   bool setup(const int *size, const float *color, const float *trans, const float *rot, float distance, float fovy, bool useShadow, bool useShadowMapping, int shadowMappingTextureSize, int maxNumModel);

   /* setSize: set size */
   void setSize(int w, int h);

   /* getWidth: get width */
   int getWidth();

   /* getHeight: get height */
   int getHeight();

   /* setCameraView: set camera view parameters */
   void setCameraView(const float *trans, const float *angle, float distance, float fovy, PMDBone *baseBone, bool modelFollow);

   /* getCameraBoneName: get camera bone Name */
   const char *getCameraBoneName();

   /* updateCameraBone: update camera bone */
   void updateCameraBone(PMDBone *baseBone);

   /* setCameraParam: set camera view parameter from camera controller */
   void setCameraFromController(CameraController *c);

   /* setViewMoveTimer: reset timer in sec for rotation, transition, and scale of view */
   void setViewMoveTimer(double sec);

   /* isViewMoving: return if view is moving by timer */
   bool isViewMoving();

   /* resetCamera: reset camera view */
   void resetCamera();

   /* translate: translate */
   void translate(float x, float y, float z);

   /* translateWithView: translate with view */
   void translateWithView(float x, float y, float z);

   /* rotate: rotate scene */
   void rotate(float x, float y, float z);

   /* setDistance: set distance */
   void setDistance(float distance);

   /* getDistance: get distance */
   float getDistance();

   /* setFovy: set fovy */
   void setFovy(float distance);

   /* getFovy: get fovy */
   float getFovy();

   /* setShadow: switch shadow */
   void setShadow(bool useShadow);

   /* setShadowMapping: switch shadow mapping */
   void setShadowMapping(bool useShadowMapping, int textureSize);

   /* getRenderOrder: return rendering order */
   void getRenderOrder(int *order, PMDObject *objs, int num);

   /* render: render all */
   void render(PMDObject *objs, const int *order, int num, Stage *stage, bool useMMDLikeCartoon, bool useCartoonRendering, float lightIntensity, float *lightDirection, float *lightColor, bool useShadowMapping, int shadowMappingTextureSize, float shadowMappingSelfDensity, float shadowMappingFloorDensity, double ellapsedTimeForMove, float shadowDensity);

   /* pickModel: pick up a model at the screen position */
   int pickModel(PMDObject *objs, int num, int x, int y, int *allowDropPicked);

   /* updateLigit: update light */
   void updateLight(bool useMMDLikeCartoon, bool useCartoonRendering, float lightIntensity, const float *lightDirection, const float *lightColor);

   /* updateDepthTextureViewParam: update center and radius information to get required range for shadow mapping */
   void updateDepthTextureViewParam(PMDObject *objList, int num);

   /* updateProjectionMatrix: update view information */
   void updateProjectionMatrix();

#ifdef MY_LUMINOUS
   /* toggleLuminous: toggle luminous effect */
   void toggleLuminous();

   /* getLuminousIntensity: get luminous intensity */
   float getLuminousIntensity();
#endif

   /* getScreenPointPosition: convert screen position to object position */
   void getScreenPointPosition(btVector3 *dst, btVector3 *src);

   /* getCurrentViewCenterPos: get current view center position */
   void getCurrentViewCenterPos(btVector3 *pos);

   /* getCurrentViewTransform: get current view transform matrix */
   void getCurrentViewTransform(btTransform *tr);

   /* getInfoString: store current view parameters to buffer */
   void getInfoString(char *buf, int buflen);

   /* clearScreen: clear screen */
   void clearScreen();

   /* setDoppelShadowParam: set doppel shadow parameters */
   void setDoppelShadowParam(const float *col, const float *offset);

   /* setDoppelShadowFlag: set doppel shadow flag */
   void setDoppelShadowFlag(bool flag);

   /* setDefaultFrameBufferId: set default frame buffer id */
   void setDefaultFrameBufferId(GLuint id);

   /* setBackgroundTransparency: set background transparency */
   void setBackgroundTransparency(bool flag);
};
