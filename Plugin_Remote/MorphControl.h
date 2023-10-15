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
