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

#define LOGTEXT_COLOR          1.0f,0.7f,0.3f,0.7f /* default text color */
#define LOGTEXT_COLOR_ERROR    1.0f,0.0f,0.0f,0.8f /* text color for error text */
#define LOGTEXT_COLOR_WARNING  1.0f,0.6f,0.0f,0.8f /* text color for warning text */
#define LOGTEXT_COLOR_STATUS   0.0f,0.7f,0.4f,0.7f /* text color for status text*/
#define LOGTEXT_COLOR_SENT     1.0f,0.9f,0.0f,0.7f /* text color for sent text */
#define LOGTEXT_COLOR_CAPTURED 0.3f,0.2f,0.0f,0.7f /* text color for captured text */
#define LOGTEXT_COLOR_NARROW   0.7f,1.0f,0.2f,0.7f /* text color for narrowing */
#define LOGTEXT_BGCOLOR        0.0f,0.0f,0.0f,0.8f /* background color */
#define LOGTEXT_BGCOLOR_TYPING 0.3f,0.6f,0.1f,0.8f /* background color while typing narrowing */
#define LOGTEXT_MAXLINELEN     256
#define LOGTEXT_MAXNLINES      512
#define LOGTEXT_SCROLLBARWIDTH 0.4f                /* scroll bar width */

#define LOGTEXT_TYPINGDURATIONFRAME 90.0f
#define LOGTEXT_STATETRANSITIONFRAME 6.0f
#define LOGTEXT_TEXTTRANSITIONFRAME 10.0f

/* LogText: log text area behind character */
class LogText
{
private:

   FTGLTextureFont *m_font;   /* text font */
   int m_textHeight;          /* text height */
   int m_textWidth;           /* text width */
   float m_textX;             /* text position in x */
   float m_textY;             /* text position in y */
   float m_textZ;             /* text position in z */
   float m_textScale;         /* text scale */

   char **m_textList;                    /* text list */
   unsigned int *m_flagList;             /* log flag list */
   float *m_transRateList;
   FTGLTextDrawElements *m_drawElements; /* drawing elements for rendering */
   bool *m_elementErrorFlag;             /* element error flag */
   int m_textIndex;                      /* current position of text list */
   int m_viewIndex;                      /* relative position for rendering */

   double m_typingFrame;                  /* narrowing typing mode rest frame */
   char m_narrowString[MMDAGENT_MAXBUFLEN];   /* narrowing string */
   bool m_narrowStringHasCase;
   FTGLTextDrawElements *m_drawElementNarrow; /* drawing element for rendering narrowing string */
   bool *m_noShowFlag;                   /* no show flag, used for narrowing */

   bool m_status;                        /* tatus, true when enabled, false when disabled */
   float m_transRate;                    /* transition rate */

   float m_heightBase;
   bool m_2d;

   /* LogText: initialize logger */
   void initialize();

   /* LogText: free logger */
   void clear();

   /* updateNarrow: update narrowing */
   void updateNarrow();

   /* matchNarrow: check if string matches narrowing string */
   bool matchNarrow(const char *str);

   /* renderMain: rendering main function */
   void renderMain(float width, float height, float fullScale, float textScale, float x, float y, float z);

public:

   /* LogText: constructor */
   LogText();

   /* ~LogText: destructor */
   ~LogText();

   /* setup: initialize and setup logger with args */
   bool setup(FTGLTextureFont *font, const int *size, const float *position, float scale);

   /* set2dflag: set 2d flag */
   void set2dflag(bool flag);

   /* get2dflag: get 2d flag */
   bool get2dflag();

   /* log: store log text */
   void log(unsigned int flag, const char *format, ...);

   /* scroll: scroll text area */
   void scroll(int shift);

   /* updateTypingActiveTime: update typing active time */
   void updateTypingActiveTime(double frame);

   /* startTyping: start typing */
   void startTyping();

   /* isTypingActive: return true when typing is active */
   bool isTypingActive();

   /* endTyping: end typing */
   void endTyping();

   /* resetNarrowString: reset narrow string */
   void resetNarrowString();

   /* addCharToNarrowString: add char to narrow string */
   void addCharToNarrowString(char c);

   /* backwardCharToNarrowString: backward char to narrow string */
   void backwardCharToNarrowString();

   /* render: render text area */
   void render();

   /* render2d: render text area for 2d screen */
   void render2d(float screenWidth, float screenHeight, int idx);

   /* setStatus: set open / close status */
   void setStatus(bool sw);

   /* update Status: set open / close transition status */
   void updateStatus(double ellapsedFrame);

   /* getHeightBase: get height base */
   float getHeightBase();
};
