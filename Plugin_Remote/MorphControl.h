/* ----------------------------------------------------------------- */
/*           The Toolkit for Building Voice Interaction Systems      */
/*           "MMDAgent" developed by MMDAgent Project Team           */
/*           http://www.mmdagent.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2015  Nagoya Institute of Technology          */
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

// Morph control information
class MorphControl
{
private:
   PMDFaceInterface *m_targetFace;
   float m_rateTarget;
   float m_rateCurrent;

   // initialize: initialize instance
   void initialize();

public:
   // constructor
   MorphControl();

   // constructor
   MorphControl(PMDFaceInterface *f, float rateTarget);

   // destructor
   ~MorphControl();

   // clear: clear instance
   void clear();

   // set: set
   void set(PMDFaceInterface *f, float rateTarget);

   // resetTarget: reset target weight
   void resetTarget();

   // setTarget: set target weight
   void setTarget(float value);

   // resetWeight: reset face weights before updates
   void resetWeight();

   // addWeight: add face weights before updates
   void addWeight(float smearingCoef);

   // applyWeight: apply the added rate to faces
   void apply();
};

// Morph controller set
class MorphControlSet
{
private:
   PTree *m_index;

   // initialize: initialize instance
   void initialize();

public:
   // constructor
   MorphControlSet();

   // destructor
   ~MorphControlSet();

   // clear: clear instance
   void clear();

   // setup: setup
   void setup();

   // set: set control for the shape name on the model
   bool set(const char *shapeName, PMDFaceInterface *interface, float value);

   /* find: find morphinfo in the index */
   MorphControl *find(const char *shapeName);

   // resetTargets: reset all targets
   void resetTargets();

   // resetWeights: reset all weights
   void resetWeights();

   // addWeights: add face weights before updates
   void addWeights(float smearingCoef);

   // apply: apply the added rates to faces
   void apply();

};
