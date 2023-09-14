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
#define TEXTAREA_MAXLINELEN 256            /* maximum text length of a line */
#define TEXTAREA_MAXLINENUM 100            /* maximum number of lines */
#define TEXTAREA_DEFAULT_WIDTH 8.0f        /* image default width */
#define TEXTAREA_TRANSITION_FRAME_LEN 10.0 /* transition effect frame length */
#define TEXTAREA_TRANSITION_OFFSET 0.6f    /* transition effect offsetting range */

/* TextArea: textarea manager */
class TextArea
{
private:

   MMDAgent *m_mmdagent;
   int m_id;
   char  *m_name;          /* alias name */
   float m_width_set;      /* width of text area */
   float m_height_set;     /* height of text area */
   float m_textsize_set;   /* text font scale */
   float m_width;          /* actual width of text area */
   float m_height;         /* actual height of text area */
   float m_textsize;       /* actual text font scale */
   float m_margin;         /* margin around texts */
   float m_linespace;      /* space adjuster between lines */
   bool m_outline;         /* true when outline mode is enabled */
   float m_bgcolor[4];     /* background color */
   float m_textcolor[4];   /* text color */
   btVector3 m_offset;     /* position at the center of the text area */
   btQuaternion m_rot;     /* rotation of the text area */
   btScalar m_matrix[16];  /* transformation matrix */
   PMDBone *m_basebone;    /* base bone on which this text area is on */

   FTGLTextureFont *m_font; /* text font */
   double m_transFrame;     /* transition frame */
   bool m_active;           /* active flag */

   char  *m_text[2];               /* current text to be displayed */
   FTGLTextDrawElements m_elem[2]; /* text element */
   FTGLTextDrawElements m_elemOut[2]; /* text outline element */
   PMDTexture *m_imageTexture[2];  /* image texture to be displayed */
   int m_bid;                      /* current id of the double-buffer */
   bool m_enableTrans;             /* enable Transition when true */

   CameraImage *m_cameraImage[2];   /* camera image handler */


   /* initialize: initialize variables */
   void initialize();

   /* clear: free variables */
   void clear();

   /* updateVertices: update vertices */
   void updateVertices();

public:

   /* TextArea: constructor */
   TextArea();

   /* ~TextArea: destructor */
   ~TextArea();

   /* setup: setup variables */
   bool setup(MMDAgent *mmdagent, int id, const char *str);

   /* setActiveFlag: set active flag */
   void setActiveFlag(bool flag);

   /* setText: set text */
   void setText(const char *text);

   /* getName: get name */
   char *getName();

   /* matchName: check if name matches */
   bool matchName(const char *str);

   /* update: update */
   void update(double frame);

   /* render: render */
   void render();
};
