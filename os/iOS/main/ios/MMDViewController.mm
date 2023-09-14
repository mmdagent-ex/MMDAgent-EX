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

#import "MMDViewController.h"
#import "MMDAgent.h"
#import <OpenGLES/ES2/glext.h>
#import <AVFoundation/AVFoundation.h>

/* set below to download system dir */
#define SYSTEMDOWNLOADURI "https://mmdagent.lee-lab.org/dl/sys/v2"

#define TAPDURATION 0.3

extern MMDAgent *mmdagent;
extern bool enable;

char *openURLString = NULL;    /* open URL string given before launch by URL scheme */
float startCameraDistance;     /* camera distance at the beginning of pinch gesture */
int startLine;
int tapStatus = 0;             /* tap status, 1 to wait panning gesture for TAPDURATION sec after a tap  */
int panMode = 0;               /* panning mode, 0 = rotate, 1 = translate */

/* define this to enable high-resolution rendering on retina display */
#define RETINA_HIGHRES

@implementation MMDViewController

- (void)initView
{
   GLKView *view = (GLKView *)self.view;

   /* init view */
   view.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
   [EAGLContext setCurrentContext:view.context];
   view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
   view.drawableStencilFormat = GLKViewDrawableStencilFormat8;
   view.drawableMultisample = GLKViewDrawableMultisample4X;
#ifdef RETINA_HIGHRES
   view.contentScaleFactor = [UIScreen mainScreen].scale;
#endif /* RETINA_HIGHRES */

   self.preferredFramesPerSecond = 60;

   view.userInteractionEnabled = true;
   view.multipleTouchEnabled = true;
}

- (BOOL)prefersStatusBarHidden
{
   return true;
}

- (void)viewDidLoad
{
   [super viewDidLoad];

   [self initView];
   glfwInitForIOS();

   /* set up audio session of this application */
   NSError *error = nil;
   BOOL success;

   /* initialize audio session assigned to this application */
   AVAudioSession *session = [AVAudioSession sharedInstance];
   /* set up audio session category for PlayAndRecord with options */
   /*
      AVAudioSessionCategoryOptionMixWithOthers
         allow sound mixing whyen other sound application is active
         (invalid when background mode is not allowed)
      AVAudioSessionCategoryOptionDuckOthers;
         lower other volume (start/stop session between sound start)
   */
   /* success = [session setCategory: AVAudioSessionCategoryPlayAndRecord withOptions: AVAudioSessionCategoryOptionMixWithOthers error: &error]; */
   success = [session setCategory: AVAudioSessionCategoryPlayAndRecord error: &error];
   if (!success) {
     /* category setting error */
   }
   /* specialize the audio session category by specifying modes */
   /*
      AVAudioSessionModeVideoChat
        for PlayAndRecord, enable voice-specific processing (AGC etc.)
        audio routes are optimized for video chat: enable bluetooth and speaker
      AVAudioSessionModeMeasurement;
        disable sound signal processing
   */
   success = [session setMode: AVAudioSessionModeVideoChat error: &error];
   if (!success) {
     /* mode setting error */
   }

   /* activate audio session */
   success = [session setActive: YES error: &error];
   if (!success) {
     /* activation error */
   }

   /* set up gesture recognizer */
   /* tap */
   UITapGestureRecognizer *tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(tapped:)];
   tapRecognizer.numberOfTapsRequired = 1;
   [self.view addGestureRecognizer:tapRecognizer];
   /* pinch */
   UIPinchGestureRecognizer *pinchRecognizer = [[UIPinchGestureRecognizer alloc] initWithTarget:self action:@selector(pinched:)];
   [self.view addGestureRecognizer:pinchRecognizer];
   /* pan */
   UIPanGestureRecognizer *panRecognizer = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(panned:)];
   [self.view addGestureRecognizer:panRecognizer];
   /* long press */
   UILongPressGestureRecognizer *longPressRecognizer = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(longPressed:)];
   [self.view addGestureRecognizer:longPressRecognizer];
   /* edge pan */
   UIScreenEdgePanGestureRecognizer *edgePanRecognizer = [[UIScreenEdgePanGestureRecognizer alloc] initWithTarget:self action:@selector(rightEdgePanned:)];
   [edgePanRecognizer setEdges:UIRectEdgeRight];
   [self.view addGestureRecognizer:edgePanRecognizer];
   [panRecognizer requireGestureRecognizerToFail:edgePanRecognizer];
   UIScreenEdgePanGestureRecognizer *edgePanRecognizer2 = [[UIScreenEdgePanGestureRecognizer alloc] initWithTarget:self action:@selector(leftEdgePanned:)];
   [edgePanRecognizer2 setEdges:UIRectEdgeLeft];
   [self.view addGestureRecognizer:edgePanRecognizer2];
   [panRecognizer requireGestureRecognizerToFail:edgePanRecognizer2];
}

char *systemDirName;
char *systemConfigFileName;
char *pluginDirName;

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
   int i;
   size_t len;
   char buff[MMDAGENT_MAXBUFLEN];

   if (enable == true) {
      /* process main loop */
      mmdagent->updateAndRender();
      /* check if reset was ordered while setup process */
      if (mmdagent->getResetFlag() == true) {
         mmdagent->restart(systemDirName, pluginDirName, systemConfigFileName, SYSTEMDOWNLOADURI, "");
         mmdagent->createMenu();
      }
      /* check if URL loading is requested */
      if (mmdagent->getKeyValue() && !MMDAgent_strequal(mmdagent->getKeyValue()->getString("_RequestedURL", "NO"), "NO")) {
         [self openWebView:mmdagent->getKeyValue()->getString("_RequestedURL", "NO")];
         mmdagent->getKeyValue()->setString("_RequestedURL", "NO");
      }
   } else {
      /* for the first time, launch contents */
      NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
      const char *url = [[userDefaults stringForKey:@"sbURL"] UTF8String];
      /* get argc, argv, and append open url string if given at startup */
      NSArray *args = [[NSProcessInfo processInfo] arguments];
      char **argv;
      int argc = (int)[args count];
      if (openURLString != NULL)
         argc++;
      if (MMDAgent_strheadmatch(url, "http://"))
         argc++;
      argv = (char **)malloc(argc * sizeof(char*));
      for (i = 0; i < (int)[args count]; i++)
         argv[i] = MMDAgent_strdup([(NSString *)args[i] UTF8String]);
      if (MMDAgent_strheadmatch(url, "http://"))
         argv[i++] = MMDAgent_strdup(url);
      if (openURLString != NULL)
         argv[i++] = openURLString;

      // set paths from executable path in argv[0]
      strcpy(buff, argv[0]);
      if (MMDAgent_strtailmatch(buff, ".exe") == true || MMDAgent_strtailmatch(buff, ".EXE") == true) {
         len = MMDAgent_strlen(buff);
         buff[len - 4] = '.';
         buff[len - 3] = 'm';
         buff[len - 2] = 'd';
         buff[len - 1] = 'f';
      } else {
         strcat(buff, ".mdf");
      }
      systemConfigFileName = MMDAgent_strdup(buff);
      systemDirName = MMDAgent_dirname(argv[0]);
      sprintf(buff, "%s%c%s", systemDirName, MMDAGENT_DIRSEPARATOR, "Plugins");
      pluginDirName = MMDAgent_strdup(buff);

      char *sysDownloadURI = NULL;

      /* if AppData does not exist, download system dir under ContentDir/_sys */
      sprintf(buff, "%s%c%s", systemDirName, MMDAGENT_DIRSEPARATOR, "AppData");
      DIRECTORY *d = MMDAgent_opendir(buff);
      if (d != NULL) {
	     /* AppData exist on the same dir of executable: no download */
	      MMDAgent_closedir(d);
      } else {
        /* AppData not exist on the same dir of executable */
	     char *contentDownloadDir = MMDAgent_contentdirdup();
	     if (contentDownloadDir != NULL) {
          /* got content dir */
	       d = MMDAgent_opendir(contentDownloadDir);
	       if (d != NULL) {
            MMDAgent_closedir(d);
	       }
	       if (d != NULL || MMDAgent_mkdir(contentDownloadDir) == true) {
            /* content dir exists */
            sprintf(buff, "%s%c%s", contentDownloadDir, MMDAGENT_DIRSEPARATOR, "_sys");
            d = MMDAgent_opendir(buff);
            if (d != NULL) {
	           MMDAgent_closedir(d);
            }
            if (d != NULL || MMDAgent_mkdir(buff) == true) {
	           /* system cache dir under content dir exists */
	           free(systemDirName);
	           systemDirName = MMDAgent_strdup(buff);
	           sysDownloadURI = MMDAgent_strdup(SYSTEMDOWNLOADURI);
            }
	       }
	       free(contentDownloadDir);
	     }
      }

      // setup MMDAgent
      MMDAgent_enablepoco();
      MMDAgent_setTime(0.0);
      mmdagent = new MMDAgent();
      if(mmdagent->setupSystem(systemDirName, pluginDirName, systemConfigFileName, sysDownloadURI, "") == false) {
         delete mmdagent;
         return;
      }
      mmdagent->createMenu();

      /* set view size */
      CGRect rect = [[UIScreen mainScreen] bounds];
#ifdef RETINA_HIGHRES
      mmdagent->procWindowSizeMessage(rect.size.width * [UIScreen mainScreen].scale, rect.size.height * [UIScreen mainScreen].scale);
#else
      mmdagent->procWindowSizeMessage(rect.size.width, rect.size.height);
#endif /* RETINA_HIGHRES */

      /* setup content */
      if (mmdagent->setupContent(argc, argv) == false) {
         delete mmdagent;
         return;
      }

      /* check if reset was ordered while setup */
      if (mmdagent->getResetFlag() == true) {
         mmdagent->restart(systemDirName, pluginDirName, systemConfigFileName, SYSTEMDOWNLOADURI, "");
         mmdagent->createMenu();
      }

      /* free */
      for (int i = 0; i < argc; i++)
         free(argv[i]);
      free(argv);

      enable = true;
   }
}

- (void)tapped:(UITapGestureRecognizer *)sender
{
   CGRect rect = [[UIScreen mainScreen] bounds];
   CGPoint location = [sender locationInView:self.view];
   Menu *m;
   FileBrowser *f;
   Prompt *p;
   Button *b;
   InfoText *t;
   Slider *s;
   Tabbar *tb;

   if (mmdagent == NULL) {
      m = NULL;
      f = NULL;
      p = NULL;
      t = NULL;
      s = NULL;
      tb = NULL;
   } else {
      m = mmdagent->getMenu();
      f = mmdagent->getFileBrowser();
      p = mmdagent->getPrompt();
      t = mmdagent->getInfoText();
      s = mmdagent->getSlider();
      tb = mmdagent->getTabbar();
   }

   if (tb) {
      if (tb->isShowing()) {
         /* tab bar is active */
         if (tb->isPointed(location.x, location.y, rect.size.width, rect.size.height)) {
            tb->execByTap(location.x, location.y, rect.size.width, rect.size.height);
            return;
         }
      } else {
         /* not showing, show and proceed */
         tb->show();
      }
   }

   if (t && t->isShowing()) {
      t->execByTap(location.x, location.y, rect.size.width, rect.size.height);
      return;
   }
   if (s && s->isShowing()) {
      /* slider isactive: execute item if tapped, else hide */
      if (s->isPointed(location.x, location.y, rect.size.width, rect.size.height))
         s->execByTap(location.x, location.y, rect.size.width, rect.size.height);
      else {
         s->setExecuteFlag(false);
         s->hide();
      }
      return;
   }
   if (m && m->isShowing()) {
      /* menu is active: execute item if tapped, else hide */
      if (m->isPointed(location.x, location.y, rect.size.width, rect.size.height))
         m->execByTap(location.x, location.y, rect.size.width, rect.size.height);
      else if (m->isPopping())
         m->releasePopup();
      else
         m->hide();
      return;
   }
   if (f && f->isShowing()) {
      /* file browser is active: execute item if tapped, else hide */
      if (f->isPointed(location.x, location.y, rect.size.width, rect.size.height))
         f->execByTap(location.x, location.y, rect.size.width, rect.size.height);
      else
         f->hide();
      return;
   }
   if (p && p->isShowing()) {
      /* prompt is active: execute item if tapped, else hide */
      if (p->isPointed(location.x, location.y, rect.size.width, rect.size.height))
         p->execByTap(location.x, location.y, rect.size.width, rect.size.height);
      return;
   }
   if ((b = mmdagent->pointedButton(location.x, location.y, rect.size.width, rect.size.height))) {
      b->exec();
      return;
   }

   /* set flag and call tapExecute after TAPDURATION sec to distinguish single tap and tap-pan */
   tapStatus = 1;
   [self performSelector:@selector(tapExecute:) withObject:[NSValue valueWithCGPoint:location] afterDelay:TAPDURATION];
}

- (void)tapExecute:(NSValue *)locationValue
{
   CGPoint location;
   [locationValue getValue:&location];
   if (tapStatus == 1) {
      tapStatus = 0;
      /* no panning after a tap, so execute tap */
       mmdagent->procMouseLeftButtonDownMessage(location.x, location.y, false, false);
       mmdagent->procMouseLeftButtonUpMessage();
   }
}

CGPoint sp;       /* pan starting point */
int direction_x;  /* detected panning direction for X */
int direction_y;  /* detected panning direction for Y */
int scroll_pastY;
int scroll_Y;
int scroll_step = 20;

- (void) panned:(UIPanGestureRecognizer*)sender
{
   CGRect rect = [[UIScreen mainScreen] bounds];
   CGPoint location = [sender locationInView:self.view];
   CGPoint p;
   float r1, r2;
   float vx, vy;
   Menu *m;
   FileBrowser *f;
   InfoText *t;
   Slider *s;

   if (mmdagent == NULL) {
      m = NULL;
      f = NULL;
      t = NULL;
      s = NULL;
   } else {
      m = mmdagent->getMenu();
      f = mmdagent->getFileBrowser();
      t = mmdagent->getInfoText();
      s = mmdagent->getSlider();
   }

   p = [sender translationInView:self.view];
   vx = location.x - sp.x;
   vy = location.y - sp.y;

   switch(sender.state) {
      case UIGestureRecognizerStateBegan:
         if (t && t->isShowing()) {
            /* infotext is showing, set drag start point */
            t->setStartingPoint(location.x, location.y, rect.size.width, rect.size.height);
            break;
         }
         if (s && s->isShowing()) {
            /* slider is active, start sliding */
            if (s->isPointed(location.x, location.y, rect.size.width, rect.size.height)) {
               s->execByTap(location.x, location.y, rect.size.width, rect.size.height);
               s->setExecuteFlag(true);
               break;
            }
         }
         /* store pan starting point */
         sp = location;
         if (tapStatus == 1) {
            /* panning gesture just after a tap: translation mode */
            tapStatus = 0;
            panMode = 1;
         } else {
            /* panning gesture without tap: rotation mode */
            panMode = 0;
         }
         direction_x = 0;
         direction_y = 0;
         scroll_pastY = 0;
         break;
      case UIGestureRecognizerStateChanged:
         if (t && t->isShowing()) {
            /* infotext is showing, update current point */
            t->setCurrentPoint(location.x, location.y);
            break;
         }
         if (s && s->isShowing() && s->getExecuteFlag() == true) {
            /* slider is active, continue sliding */
            s->execByTap(location.x, location.y, rect.size.width, rect.size.height);
            break;
         }
         if (direction_x == 0 && direction_y == 0) {
            if (vx < -4.0f) {
               /* left pan trigger */
               direction_x = -1;
               /* if menu is active and pointed, forward menu */
               if (m && m->isPointed(sp.x, sp.y, rect.size.width, rect.size.height) && m->isShowing()) {
                  if (m->isPopping())
                     m->releasePopup();
                  m->forward();
               }
            } else if (vx > 4.0f) {
               /* right pan trigger */
               direction_x = 1;
               /* if menu is active and pointed, backward menu */
               if (m && m->isPointed(sp.x, sp.y, rect.size.width, rect.size.height) && m->isShowing()) {
                  if (m->isPopping())
                     m->releasePopup();
                  m->backward();
               }
            } else if (vy < -3.0f) {
               /* down pan trigger */
               direction_y = -1;
            } else if (vy > 3.0f) {
               /* up pan trigger */
               direction_y = 1;
            }
         }
         if (m && m->isPointed(sp.x, sp.y, rect.size.width, rect.size.height) && m->isShowing()) {
            /* if menu is active and pointed, update animation while panning */
            if (direction_x == 1){
               mmdagent->getMenu()->forceBackwardAnimationRate((location.x - sp.x) / rect.size.width);
            } else if (direction_x == -1) {
               mmdagent->getMenu()->forceForwardAnimationRate((sp.x - location.x)/ rect.size.width);
            } else {
               scroll_Y = scroll_step * vy / rect.size.height;
               if (scroll_Y != scroll_pastY)
                  mmdagent->getMenu()->scroll(scroll_pastY - scroll_Y);
               scroll_pastY = scroll_Y;
            }
         } else if (f && f->isPointed(sp.x, sp.y, rect.size.width, rect.size.height) && f->isShowing()) {
            /* if file browser is active and pointed, scroll for vertical pan and update animation for horizontal pan */
            if (fabs(vx) < fabs(vy)) {
               f->scroll(-direction_y);
            } else if (fabs(vx / rect.size.width) > 0.1f){
               f->setBackSlideAnimationRate(vx / rect.size.width);
            }
         } else {
            if (panMode == 1) {
               /* translation */
               CGRect rect = [[UIScreen mainScreen] bounds];
               r1 = p.x / rect.size.width;
               r2 = p.y / rect.size.height;
               if (mmdagent->getRenderer() != NULL && mmdagent->isViewMovable())
                  mmdagent->getRenderer()->translateWithView(r1 * 1.3f, -r2 * 1.3f, 0.0f);
            } else {
               /* rotation */
               r1 = p.x; r2 = p.y;
               if (r1 > 32767) r1 -= 65536;
               if (r1 < -32768) r1 += 65536;
               if (r2 > 32767) r2 -= 65536;
               if (r2 < -32768) r2 += 65536;
               if (mmdagent->getRenderer() != NULL && mmdagent->isViewMovable())
                  mmdagent->getRenderer()->rotate(r2 * 0.5f, r1 * 0.5f, 0.0f);
            }
         }
         break;
      case UIGestureRecognizerStateEnded:
      case UIGestureRecognizerStateCancelled:
         if (t && t->isShowing()) {
            t->releasePoint();
            break;
         }
         if (s && s->isShowing()) {
            s->setExecuteFlag(false);
         }
         if (m && m->isPointed(sp.x, sp.y, rect.size.width, rect.size.height) && m->isShowing()) {
            /* if menu is active and pointed, release animation and finish move forward/backward */
            if (direction_x == 1){
               mmdagent->getMenu()->forceBackwardAnimationRate(-1.0f);
            } else if (direction_x == -1) {
               mmdagent->getMenu()->forceForwardAnimationRate(-1.0f);
            }
         } else if (f && f->isPointed(sp.x, sp.y, rect.size.width, rect.size.height)) {
            /* if file browser is active and pointed, release animation and go back (right pan) or hide (left pan) */
            f->setBackSlideAnimationRate(0.0f);
            if (fabs(vx) > fabs(vy)) {
               if (vx / rect.size.width > 0.2f) {
                  f->back();
               } else if (vx / rect.size.width  < -0.2f) {
                  f->hide();
               }
            }
         }
         break;
      default:
         break;
   }
   [sender setTranslation:CGPointZero inView:self.view];
}

- (void) pinched:(UIPinchGestureRecognizer*)sender
{
   FileBrowser *f;
   float r;

   if (mmdagent == NULL)
      f = NULL;
   else
      f = mmdagent->getFileBrowser();

   if (f && f->isShowing()) {
      /* zoom in/out for file browser */
      if (sender.state == UIGestureRecognizerStateBegan)
         startLine = f->getLines();
      f->setLines((int)((float)startLine / sender.scale));
      return;
   }

   /* zoom in/out view */
   if (sender.state == UIGestureRecognizerStateBegan)
      startCameraDistance = mmdagent->getRenderer()->getDistance();

   if (fabs(sender.scale) < 0.001f) {
      r = 100.0f;
   } else {
      r = 1.0f / sender.scale;
   }
   if (mmdagent->getRenderer() != NULL && mmdagent->isViewMovable())
      mmdagent->getRenderer()->setDistance(startCameraDistance * r);
}

- (void) longPressed:(UILongPressGestureRecognizer*)sender
{
   CGRect rect = [[UIScreen mainScreen] bounds];
   CGPoint location = [sender locationInView:self.view];
   Menu *m;

   switch(sender.state) {
   case UIGestureRecognizerStateBegan:
      if (mmdagent == NULL)
         return;
      m = mmdagent->getMenu();
      if (m && m->isShowing() && m->isPointed(location.x, location.y, rect.size.width, rect.size.height)) {
         m->togglePopupByTap(location.x, location.y, rect.size.width, rect.size.height);
      } else {
         mmdagent->procMouseLeftButtonLongPressedMessage(location.x, location.y, rect.size.width, rect.size.height);
      }
      break;
   case UIGestureRecognizerStateChanged:
      break;
   case UIGestureRecognizerStateEnded:
   case UIGestureRecognizerStateCancelled:
      if (mmdagent == NULL)
         return;
      m = mmdagent->getMenu();
      if (m && m->isShowing() && m->isPointed(location.x, location.y, rect.size.width, rect.size.height)) {
      } else {
         mmdagent->procMouseLeftButtonLongReleasedMessage(location.x, location.y, rect.size.width, rect.size.height);
      }
      break;
   default:
      break;
   }


}

- (void) rightEdgePanned:(UIScreenEdgePanGestureRecognizer*)sender
{
   Menu *m;

   /* show menu or go forward menu */
   switch(sender.state) {
      case UIGestureRecognizerStateBegan:
         if (mmdagent == NULL)
            return;
         m = mmdagent->getMenu();
         if (m == NULL)
            return;

         if (m->isShowing() == false) {
            m->show();
         } else {
            if (m->isPopping())
               m->togglePopupCurrent();
            m->forward();
         }
         break;
      case UIGestureRecognizerStateChanged:
      case UIGestureRecognizerStateEnded:
      case UIGestureRecognizerStateCancelled:
         break;
      default:
         break;
   }
   [sender setTranslation:CGPointZero inView:self.view];
}

- (void)leftEdgePanned:(UIScreenEdgePanGestureRecognizer*)sender
{
   Slider *s;

   /* show slider */
   switch(sender.state) {
      case UIGestureRecognizerStateBegan:
         if (mmdagent == NULL)
            return;
         s = mmdagent->getSlider();
         if (s == NULL)
            return;

         if (s->isShowing() == false) {
            s->show();
         }
         break;
      case UIGestureRecognizerStateChanged:
      case UIGestureRecognizerStateEnded:
      case UIGestureRecognizerStateCancelled:
         break;
      default:
         break;
   }
   [sender setTranslation:CGPointZero inView:self.view];
}

- (void)willAnimateRotationToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration
{
   CGRect rect = [[UIScreen mainScreen] bounds];
#ifdef RETINA_HIGHRES
   mmdagent->procWindowSizeMessage(rect.size.width * [UIScreen mainScreen].scale, rect.size.height * [UIScreen mainScreen].scale);
#else
   mmdagent->procWindowSizeMessage(rect.size.width, rect.size.height);
#endif /* RETINA_HIGHRES */
}

- (void)openWebView:(const char *)url
{
   NSString *urlstr = [NSString stringWithCString:url encoding:NSUTF8StringEncoding];
   NSURL *nsurl = [NSURL URLWithString:urlstr];
   [[UIApplication sharedApplication] openURL:nsurl];
}

@end
