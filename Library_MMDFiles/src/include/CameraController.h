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

/* CameraController: camera controller class, to handle a camera motion */
class CameraController
{
private:

   CameraMotion *m_motion;  /* camera motion to be played */
   float m_distance;        /* calculated distance */
   btVector3 m_pos;         /* calculated position */
   btVector3 m_angle;       /* calculated angle */
   float m_fovy;            /* calculated view angle */
   unsigned long m_lastKey; /* last key frame number */

   /* internal work area */
   double m_currentFrame;  /* current frame */
   double m_previousFrame; /* current frame at last call to advance() */

   /* control: set camera parameters according to the motion at the specified frame */
   void control(float frameNow);

   /* initialize: initialize controller */
   void initialize();

   /* clear: free controller */
   void clear();

public:

   /* CameraController: constructor */
   CameraController();

   /* ~CameraController: destructor */
   ~CameraController();

   /* setup: initialize and set up controller */
   void setup(VMD *motion);

   /* reset: reset values */
   void reset();

   /* advance: advance motion controller by the given frame, return true when reached end */
   bool advance(double deltaFrame);

   /* getCurrentViewParam: get current view parameters */
   void getCurrentViewParam(float *distance, btVector3 *pos, btVector3 *angle, float *fovy);

   /* getCurrentFrame: get current frame */
   double getCurrentFrame();

   /* setCurrentFrame: set current frame */
   void setCurrentFrame(double frame);

   /* getPreviousFrame: get previous frame */
   double getPreviousFrame();

   /* setPreviousFrame: set previous frame */
   void setPreviousFrame(double frame);
};
