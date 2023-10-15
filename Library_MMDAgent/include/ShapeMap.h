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

// maximum number of entries in a AUPRIORITY definition
#define SHAPEMAP_AUPRIORITY_MAXNUM 100

// maximum number of morph tune
#define SHAPEMAP_MORPH_TUNE_MAXNUM 30
#define SHAPEMAP_MORPH_TUNE_MAXLEN 10

/* ShapeMap: bone/face shape mapper */
class ShapeMap
{
private:

   float m_eyeRotationCoef;
   PTree *m_lipMorph;
   PTree *m_trackBone;
   PTree *m_auMorph;
   PTree *m_arKitMorph;
   PTree *m_actionMotion;
   PTree *m_exBone;
   PTree *m_exMorph;
   PMDFaceInterface *m_morphTuneSet[SHAPEMAP_MORPH_TUNE_MAXNUM][SHAPEMAP_MORPH_TUNE_MAXLEN];
   int m_morphTuneLen[SHAPEMAP_MORPH_TUNE_MAXNUM];
   int m_morphTuneNum;
   char *m_lipIgnoreList;

   /* initialize: initialize */
   void initialize();

   /* freeMorphInstance: free morph instance in a tree */
   void freeMorphInstance(PTree *tree);

   /* clear: free */
   void clear();

public:

   /* ShapeMap: constructor */
   ShapeMap();

   /* ~ShapeMap: destructor */
   ~ShapeMap();

   /* load: load shape mapping information from file */
   bool load(const char *mapFileName, PMDModel *pmd, const char *dirName);

   /* getEyeRotationCoef: get eye rotation coef */
   float getEyeRotationCoef();

   /* getLipMorph: get lip morph */
   PMDFaceInterface *getLipMorph(const char *lipEntryName);

   /* getTrackBone: get bone for head tracking */
   PMDBone *getTrackBone(const char *trackingBoneEntryName);

   /* getAUMorph: get morph for Action Unit based face tracking */
   PMDFaceInterface *getAUMorph(const char *auName);

   /* getARKitMorph: get morph for ARKit based face tracking */
   PMDFaceInterface *getARKitMorph(const char *shapeName);

   /* getActionMotionFileName: get file name for the specified dialogue action */
   const char *getActionMotionFileName(const char *actionName);

   /* getExBone: get bone for extra bone control */
   PMDBone *getExBone(const char *boneEntryName);

   /* getExMorph: get morph for extra morph control */
   PMDFaceInterface *getExMorph(const char *morphEntryName);

   /* doMorphTune: apply morph weight post-tune */
   void doMorphTune();

   /* getLipIgnoreList: get lip ignore list */
   const char *getLipIgnoreList();

};
