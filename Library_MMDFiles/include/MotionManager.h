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

#define MOTIONMANAGER_DEFAULTPRIORITY       0.0f  /* motion priority */
#define MOTIONMANAGER_DEFAULTLOOPATFRAME    0.0f  /* when specified with loop, motion will rewind at this frame when reached end */
#define MOTIONCONTROLLER_BONEENDMARGINFRAME 20.0f /* frame lengths for bone motion smoothing at loop tail */
#define MOTIONCONTROLLER_FACEENDMARGINFRAME 6.0f  /* frame lengths for face motion smoothing at loop tail */

/* motions's status at last call */
enum {
   MOTION_STATUS_RUNNING, /* running */
   MOTION_STATUS_LOOPED,  /* just looped */
   MOTION_STATUS_DELETED, /* just reached to the end and deleted itself */
};

/* motion's speed status at last call */
enum {
   ACCELERATION_STATUS_CONSTANT, /* constantly running */
   ACCELERATION_STATUS_WAITING,  /* waiting acceleration beginning frame (constant) */
   ACCELERATION_STATUS_CHANGING, /* speed is changing */
   ACCELERATION_STATUS_ENDED     /* just finished acceleration */
};

/* Motion player: hold a bag of data to perform a motion, holding status, manage loop and end of motion */
typedef struct _MotionPlayer {
   char *name; /* name */

   MotionController mc; /* motion controller */
   VMD *vmd;            /* source motion data */

   /* switches which should be set before startMotion() */
   unsigned char onEnd;         /* switch for end-of-motion: 0=keep last pose forever, 1=loop(rewind), 2=disapper with blending */
   float priority;              /* priority of this motion, larger value will supercede others default is 0 */
   bool ignoreStatic;           /* if true, bones / faces which has only the first frame will be ignored */
   float loopAt;                /* rewind to the frame, used when onEnd==1 */
   bool enableSmooth;           /* enable "magic" smooth motion transition at start, end, and loop of motion */
   bool enableRePos;            /* enable moving root bone offset of the model to the current center bone position at motion start */
   float endingBoneBlendFrames; /* length of ending motion blend */
   float endingFaceBlendFrames; /* length of ending motion blend */
   float motionBlendRate;       /* motion blend rate */

   /* work area */
   bool active;           /* become false when this motion was finished, used for motion-end detection */
   float endingBoneBlend; /* at blending down at motion end, this value keeps the rest frame */
   float endingFaceBlend; /* at blending down at motion end, this value keeps the rest frame */
   int statusFlag;        /* variable to hold status */

   float targetSpeedRate;                       /* target speed rate */
   float currentSpeedRate;                      /* current speed rate */
   float remainingFramesForStartOfAcceleration; /* remaining frames for start of changing speed rate */
   float remainingFramesForEndOfAcceleration;   /* remaining frames for end of changing speed rate */
   int accelerationStatusFlag;                  /* variable to hold status for motion speed change */
   bool wantDeleteFlag;   /* true when should delete at next call */

   struct _MotionPlayer *next;
} MotionPlayer;

/* MotionPlayer_initialize: initialize MotionPlayer */
void MotionPlayer_initialize(MotionPlayer *m);

/* Motion manager: control all the motion players for multi-track motion handling */
class MotionManager
{
private:

   PMDModel *m_pmd;                     /* assigned model */
   MotionPlayer *m_playerList;          /* list of motion players running */
   float m_beginningNonControlledBlend; /* at motion start, bones/faces not controlled in base motion will be reset within this frame */

   /* terminateEndingMotion: terminate ending active motion */
   void terminateEndingMotion(const char *name);

   /* purgeMotion: purge inactive motions */
   void purgeMotion();

   /* setup: initialize and setup motion manager */
   void setup(PMDModel *pmd);

   /* startMotionSub: initialize a motion */
   void startMotionSub(VMD *vmd, MotionPlayer *m);

   /* initialize: initialize motion manager */
   void initialize();

   /* clear: free motion manager */
   void clear();

public:

   /* MotionManager: constructor */
   MotionManager(PMDModel *pmd);

   /* ~MotionManager: destructor */
   ~MotionManager();

   /* startMotion: start a motion */
   bool startMotion(VMD *vmd, const char *name, bool full, bool once, bool enableSmooth, bool enableRePos, float priority);

   /* swapMotion: swap a motion, keeping parameters */
   bool swapMotion(VMD *vmd, const char *name);

   /* setMotionSpeedRate: set motion speed */
   bool setMotionSpeedRate(const char *name, float speedRate, float changeLength, float targetFrameIndex);

   /* deleteMotion: delete a motion */
   bool deleteMotion(const char *name);

   /* configureMotion: configure a motion */
   bool configureMotion(const char *name, const char *key, const char *value);

   /* update: apply all motion players */
   bool update(double frame);

   /* updateMotionSpeedRate: update motion speed rate */
   bool updateMotionSpeedRate(double frame);
   
   /* getMotionPlayerList: get list of motion players */
   MotionPlayer *getMotionPlayerList();

   /* getRunning: get the running motion player */
   MotionPlayer *getRunning(const char *name);

   /* updateModel: update model */
   void updateModel(PMDModel *pmd);
};
