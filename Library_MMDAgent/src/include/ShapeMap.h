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
