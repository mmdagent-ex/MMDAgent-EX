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
      FTGLTextDrawElements elem;     /* text drawing element */
      FTGLTextDrawElements elemOut;  /* text drawing element for edge 1 */
      FTGLTextDrawElements elemOut2; /* text drawing element for edge 2 */
      GLfloat vertices[12];          /* vertices */
      float drawWidth;               /* total drawing width */
      float drawHeight;              /* total drawing height */
      struct TimeCaptionList *next;  /* pointer to next caption item */
   };

   TimeCaptionList *m_list;   /* caption list */
   int m_listLen;             /* length of the list */

   double m_currentFrame;     /* current frame */
   int m_currentId;           /* current id */
   bool m_finished;           /* true when finished */

   /* initialize: initialize */
   void initialize();

   /* clear: free */
   void clear();

   /* clearElements: clear elements */
   void clearElements(TimeCaptionList *item);

   /* updateRenderingItem: update rendering Item */
   void updateRenderingItem(TimeCaptionList *item, CaptionElementConfig config, CaptionStyle *style);

public:

   /* TimeCaption: constructor */
   TimeCaption();

   /* ~TimeCaption: destructor */
   ~TimeCaption();

   /* load: load */
   bool load(const char *fileName);

   /* set: set */
   bool set(const char *string, double durationFrame);

   /* updateRendering: update rendering */
   void updateRendering(CaptionElementConfig config, CaptionStyle *style);

   /* render: render */
   void render(int id, CaptionElementConfig config, CaptionStyle *style, float width, float height);

   /* setFrame: set frame */
   int setFrame(double frame);

   /* proceedFrame: proceed frame */
   int proceedFrame(double ellapsedFrame);

   /* isFinished: return true when finished */
   bool isFinished();
};

/* CaptionElement: text caption element class */
class CaptionElement
{
private:

   char *m_name;                    /* name */
   CaptionElementConfig m_config;   /* configuration */
   TimeCaption *m_caption;          /* list of captions to be displayed */
   CaptionStyle *m_style;           /* style to be used */

   bool m_isShowing;                /* true when showing */
   bool m_endChecked;               /* flag for end detection */
   int m_timeCaptionId;             /* current time caption id */

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
