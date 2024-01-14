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

/* maximum number of captions at a time */
#define MMDAGENT_CAPTION_MAXNUM 30
/* maximum number of caption styles */
#define MMDAGENT_CAPTION_STYLE_MAXNUM 10

/* enum for caption positions */
enum {
   CAPTION_POSITION_CENTER,
   CAPTION_POSITION_SLIDELEFT,
   CAPTION_POSITION_SLIDERIGHT,
   CAPTION_POSITION_NUM
};

/* caption style structure */
struct CaptionStyle {
   char name[128];
   float color[4];           /* foreground color */
   float edgecolor1[4];      /* color of edge 1 */
   float edgethickness1;     /* thickness of edge 1 */
   float edgecolor2[4];      /* color of edge 2 */
   float edgethickness2;     /* thickness of edge 2 */
   float bgcolor[4];         /* background color */
   FTGLTextureFont *font;
   FTGLTextureFont *allocatedFont;
};

/* configuration of an caption */
struct CaptionElementConfig {
   short position;           /* caption style: center, slideLeft, slideRight */
   float size;               /* text size */
   float height;             /* location height ratio */
   double duration;          /* duration in frames */
};

/* time-varying caption */
class TimeCaption
{
private:

   struct TimeCaptionList
   {
      int id;
      char *string;         /* caption string */
      double frame;         /* time in frame to be shown */
      struct TimeCaptionList *next;  /* pointer to next caption item */
   };

   char *m_fileName;          /* source file */
   TimeCaptionList *m_list;   /* caption list */
   int m_listLen;             /* length of the list */

   double m_currentFrame;     /* current frame */
   int m_currentId;           /* current id */
   bool m_finished;           /* true when finished */

   /* initialize: initialize */
   void initialize();

   /* clear: free */
   void clear();

public:

   /* TimeCaption: constructor */
   TimeCaption();

   /* ~TimeCaption: destructor */
   ~TimeCaption();

   /* setup: setup */
   bool setup(const char *fileName);

   /* setFrame: set frame */
   int setFrame(double frame);

   /* proceedFrame: proceed frame */
   int proceedFrame(double ellapsedFrame);

   /* getCaption: get caption */
   const char *getCaption(int id);

   /* isFinished: return true when finished */
   bool isFinished();

   /* getFileName: get file name */
   const char *getFileName();
};

/* CaptionElement: text caption element class */
class CaptionElement
{
private:

   char *m_name;                    /* name */
   CaptionElementConfig m_config;   /* configuration */
   TimeCaption *m_timeCaption;      /* displaying file */
   CaptionStyle *m_style;           /* style to be used */

   char m_captionString[MMDAGENT_MAXBUFLEN]; /* current caption string */
   float m_drawWidth;               /* total drawing width */
   float m_drawHeight;              /* total drawing height */
   FTGLTextDrawElements m_elem;     /* text drawing element */
   FTGLTextDrawElements m_elemOut;  /* text drawing element for edge 1 */
   FTGLTextDrawElements m_elemOut2; /* text drawing element for edge 2 */
   GLfloat m_vertices[12];          /* vertices */
   double m_frameLeft;              /* duration timer */
   bool m_isShowing;                /* true when showing */
   bool m_endChecked;               /* flag for end detection */
   int m_timeCaptionId;             /* current time caption id */

   /* clearElements: clear elements */
   void clearElements();

   /* setCaption: set caption */
   void setCaption(const char *string);

   /* updateRenderingElement: update rendering element */
   void updateRenderingElement();

public:

   /* CaptionElement: constructor */
   CaptionElement();

   /* ~CaptionElement: constructor */
   ~CaptionElement();

   /* setup: setup */
   bool setup(const char *name, const char *str, CaptionElementConfig config, CaptionStyle *style);

   /* update: update */
   void update(double ellapsedFrame);

   /* render2D: render in 2D screen */
   void render2D(float width, float height);

   /* isShowing: return true when showing */
   bool isShowing();

   /* getName: get name */
   const char *getName();

   /* getChecked: get checked flag */
   bool getChecked();

   /* setChecked: set checked flag */
   void setChecked(bool flag);
};

/* Caption: text caption class */
class Caption
{
private:

   MMDAgent *m_mmdagent;
   int m_id;
   FTGLTextureAtlas *m_atlas;   /* texture atlas for caption drawing */
   bool m_hasAtlasError;
   CaptionStyle *m_styles[MMDAGENT_CAPTION_STYLE_MAXNUM];
   int m_numStyles;
   CaptionElement *m_captions[MMDAGENT_CAPTION_MAXNUM];
   int m_numCaptions;

   /* initialize: initialize */
   void initialize();

   /* clear: free */
   void clear();

public:

   /* Caption: constructor */
   Caption();

   /* ~Caption: destructor */
   ~Caption();

   /* setup: set up */
   void setup(MMDAgent *mmdagent, int mid);

   /* setStyle: set style */
   bool setStyle(const char *name, const char *fontPath, float *col, float *edge1, float *edge2, float *bscol);

   /* start: start a caption*/
   bool start(const char *name, const char *string, const char *styleName, CaptionElementConfig config);

   /* stop: stop a caption*/
   bool stop(const char *name);

   /* update: update */
   void update(double ellapsedFrame);

   /* render2D: render in 2D screen */
   void render2D(float width, float height);

   /* isShowing: return true when any caption is showing */
   bool isShowing();
};
