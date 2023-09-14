/* ----------------------------------------------------------------- */
/*           Toolkit for Building Voice Interaction Systems          */
/*           MMDAgent developed by MMDAgent Project Team             */
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

#ifndef __mmdagent_h__
#define __mmdagent_h__

/* definitions */

#ifdef __APPLE__
#if TARGET_OS_IPHONE
#define MOBILE_SCREEN
#endif /* TARGET_OS_IPHONE */
#else /* __APPLE__ */
#if defined(__ANDROID__)
#define MOBILE_SCREEN
#endif /* __ANDROID__ */
#endif /* __APPLE__ */

#define MMDAGENT_CONTENTINFOFILE ".mmdagent-save"
#define MMDAGENT_CONTENTUSERINFOFILE ".mmdagent-save-user"

#define MMDAGENT_MAXBUFLEN    MMDFILES_MAXBUFLEN
#define MMDAGENT_DIRSEPARATOR MMDFILES_DIRSEPARATOR
#define MMDAGENT_MAXNCOMMAND  10
#define MMDAGENT_SCREENUNITLENGTH 30.0f
#define MMDAGENT_STARTANIMATIONFRAME 30.0f
#define MMDAGENT_STARTANIMATIONPATTERNNUM 3
#define MMDAGENT_MAXCONTENTNUM 6
#define MMDAGENT_MAXPAUSESEC 5
#define MMDAGENT_CONTENTUPDATECHECKDELAYFRAME 60.0f
#define MMDAGENT_BUTTONEDGEDISTANCERATIO 0.3f
#define MMDAGENT_LOGFILEDIRNAME  "log"
#define MMDAGENT_UUIDFILENAME "_uuid"
#define MMDAGENT_MAXNUMCONTENTBUTTONS 10
#define MMDAGENT_SHOWUSAGEFRAME 210.0

#if defined(__ANDROID__) || TARGET_OS_IPHONE
// more offset for rounded corner of recent devices...
#define MMDAGENT_INDICATOR_OFFSET 0.4f
#else
#define MMDAGENT_INDICATOR_OFFSET 0.2f
#endif

#define MMDAGENT_COMMAND_MODELADD         "MODEL_ADD"
#define MMDAGENT_COMMAND_MODELCHANGE      "MODEL_CHANGE"
#define MMDAGENT_COMMAND_MODELCHANGEASYNC "MODEL_CHANGE_ASYNC"
#define MMDAGENT_COMMAND_MODELDELETE      "MODEL_DELETE"
#define MMDAGENT_COMMAND_MODELBINDBONE    "MODEL_BINDBONE"
#define MMDAGENT_COMMAND_MODELBINDFACE    "MODEL_BINDFACE"
#define MMDAGENT_COMMAND_MODELUNBINDBONE  "MODEL_UNBINDBONE"
#define MMDAGENT_COMMAND_MODELUNBINDFACE  "MODEL_UNBINDFACE"
#define MMDAGENT_COMMAND_MOTIONADD        "MOTION_ADD"
#define MMDAGENT_COMMAND_MOTIONCHANGE     "MOTION_CHANGE"
#define MMDAGENT_COMMAND_MOTIONACCELERATE "MOTION_ACCELERATE"
#define MMDAGENT_COMMAND_MOTIONRESET      "MOTION_RESET"
#define MMDAGENT_COMMAND_MOTIONDELETE     "MOTION_DELETE"
#define MMDAGENT_COMMAND_MOTIONCONFIGURE  "MOTION_CONFIGURE"
#define MMDAGENT_COMMAND_MOVESTART        "MOVE_START"
#define MMDAGENT_COMMAND_MOVESTOP         "MOVE_STOP"
#define MMDAGENT_COMMAND_TURNSTART        "TURN_START"
#define MMDAGENT_COMMAND_TURNSTOP         "TURN_STOP"
#define MMDAGENT_COMMAND_ROTATESTART      "ROTATE_START"
#define MMDAGENT_COMMAND_ROTATESTOP       "ROTATE_STOP"
#define MMDAGENT_COMMAND_STAGE            "STAGE"
#define MMDAGENT_COMMAND_WINDOWFRAME      "WINDOWFRAME"
#define MMDAGENT_COMMAND_LIGHTCOLOR       "LIGHTCOLOR"
#define MMDAGENT_COMMAND_LIGHTDIRECTION   "LIGHTDIRECTION"
#define MMDAGENT_COMMAND_LIPSYNCSTART     "LIPSYNC_START"
#define MMDAGENT_COMMAND_LIPSYNCSTOP      "LIPSYNC_STOP"
#define MMDAGENT_COMMAND_CAMERA           "CAMERA"
#define MMDAGENT_COMMAND_PLUGINENABLE     "PLUGIN_ENABLE"
#define MMDAGENT_COMMAND_PLUGINDISABLE    "PLUGIN_DISABLE"
#define MMDAGENT_COMMAND_KEYVALUESET      "KEYVALUE_SET"
#define MMDAGENT_COMMAND_LOG_START        "LOG_START"
#define MMDAGENT_COMMAND_LOG_FINISH       "LOG_FINISH"
#define MMDAGENT_COMMAND_LOG_UPLOAD       "LOG_UPLOAD"
#define MMDAGENT_COMMAND_RECOGRECORDSTART "RECOG_RECORD_START"
#define MMDAGENT_COMMAND_RECOGRECORDSTOP  "RECOG_RECORD_STOP"
#define MMDAGENT_COMMAND_BUTTONADD        "BUTTON_ADD"
#define MMDAGENT_COMMAND_BUTTONDELETE     "BUTTON_DELETE"
#define MMDAGENT_COMMAND_MOTIONCAPTURESTART "MOTIONCAPTURE_START"
#define MMDAGENT_COMMAND_MOTIONCAPTURESTOP  "MOTIONCAPTURE_STOP"
#define MMDAGENT_COMMAND_SETANIMATIONSPEEDRATE "TEXTURE_SETANIMATIONRATE"
#define MMDAGENT_COMMAND_OPENCONTENT     "OPEN_CONTENT"
#define MMDAGENT_COMMAND_SETPARALLELSKINNINGTHREADS  "CONFIG_PARALLELSKINNING_THREADS"
#define MMDAGENT_COMMAND_REOMTEKEYCHAR  "REMOTEKEY_CHAR"
#define MMDAGENT_COMMAND_REOMTEKEYDOWN  "REMOTEKEY_DOWN"
#define MMDAGENT_COMMAND_REOMTEKEYUP    "REMOTEKEY_UP"
#define MMDAGENT_COMMAND_CAPTION_SETSTYLE "CAPTION_SETSTYLE"
#define MMDAGENT_COMMAND_CAPTION_START    "CAPTION_START"
#define MMDAGENT_COMMAND_CAPTION_STOP     "CAPTION_STOP"
#define MMDAGENT_COMMAND_HOME_SET       "HOME_SET"
#define MMDAGENT_COMMAND_HOME_CLEAR     "HOME_CLEAR"

#define MMDAGENT_EVENT_MODELADD         "MODEL_EVENT_ADD"
#define MMDAGENT_EVENT_MODELCHANGE      "MODEL_EVENT_CHANGE"
#define MMDAGENT_EVENT_MODELDELETE      "MODEL_EVENT_DELETE"
#define MMDAGENT_EVENT_MODELSELECT      "MODEL_EVENT_SELECT"
#define MMDAGENT_EVENT_MODELBINDBONE    "MODEL_EVENT_BINDBONE"
#define MMDAGENT_EVENT_MODELBINDFACE    "MODEL_EVENT_BINDFACE"
#define MMDAGENT_EVENT_MODELUNBINDBONE  "MODEL_EVENT_UNBINDBONE"
#define MMDAGENT_EVENT_MODELUNBINDFACE  "MODEL_EVENT_UNBINDFACE"
#define MMDAGENT_EVENT_MOTIONADD        "MOTION_EVENT_ADD"
#define MMDAGENT_EVENT_MOTIONCHANGE     "MOTION_EVENT_CHANGE"
#define MMDAGENT_EVENT_MOTIONACCELERATE "MOTION_EVENT_ACCELERATE"
#define MMDAGENT_EVENT_MOTIONRESET      "MOTION_EVENT_RESET"
#define MMDAGENT_EVENT_MOTIONDELETE     "MOTION_EVENT_DELETE"
#define MMDAGENT_EVENT_MOTIONCONFIGURE  "MOTION_EVENT_CONFIGURE"
#define MMDAGENT_EVENT_MOVESTART        "MOVE_EVENT_START"
#define MMDAGENT_EVENT_MOVESTOP         "MOVE_EVENT_STOP"
#define MMDAGENT_EVENT_TURNSTART        "TURN_EVENT_START"
#define MMDAGENT_EVENT_TURNSTOP         "TURN_EVENT_STOP"
#define MMDAGENT_EVENT_ROTATESTART      "ROTATE_EVENT_START"
#define MMDAGENT_EVENT_ROTATESTOP       "ROTATE_EVENT_STOP"
#define MMDAGENT_EVENT_LIPSYNCSTART     "LIPSYNC_EVENT_START"
#define MMDAGENT_EVENT_LIPSYNCSTOP      "LIPSYNC_EVENT_STOP"
#define MMDAGENT_EVENT_PLUGINENABLE     "PLUGIN_EVENT_ENABLE"
#define MMDAGENT_EVENT_PLUGINDISABLE    "PLUGIN_EVENT_DISABLE"
#define MMDAGENT_EVENT_DRAGANDDROP      "DRAGANDDROP"
#define MMDAGENT_EVENT_KEY              "KEY"
#define MMDAGENT_EVENT_LONGPRESSED      "SCREEN_EVENT_LONGPRESSED"
#define MMDAGENT_EVENT_LONGRELEASED     "SCREEN_EVENT_LONGRELEASED"
#define MMDAGENT_EVENT_BUTTONADD        "BUTTON_EVENT_ADD"
#define MMDAGENT_EVENT_BUTTONEXEC       "BUTTON_EVENT_EXEC"
#define MMDAGENT_EVENT_BUTTONDELETE     "BUTTON_EVENT_DELETE"
#define MMDAGENT_EVENT_CAPTION_SETSTYLE "CAPTION_EVENT_SETSTYLE"
#define MMDAGENT_EVENT_CAPTION_START    "CAPTION_EVENT_START"
#define MMDAGENT_EVENT_CAPTION_STOP     "CAPTION_EVENT_STOP"

#define MMDAGENT_CURRENTTIME
#define MMDAGENT_TAPPED

#define MMDAGENT_MOONSHOT

/* headers */

#ifdef _WIN32
#include <windows.h>
#endif

#include "MMDFiles.h"

#include "GL/glfw.h"

class MMDAgent;
class SplashScreen;
class LogToFile;

#include "MMDAgent_utils.h"

#include "FreeTypeGL.h"
#include "LogText.h"
#include "LipSync.h"
#include "KeyValue.h"
#include "BoneFaceControl.h"
#include "PMDFaceInterface.h"
#include "ShapeMap.h"
#include "PMDObject.h"

#include "Option.h"
#include "ScreenWindow.h"
#include "Message.h"
#include "TileTexture.h"
#include "Stage.h"
#include "Render.h"
#include "Timer.h"
#include "Plugin.h"
#include "MotionStocker.h"
#include "Menu.h"
#include "Button.h"
#include "FileBrowser.h"
#include "Prompt.h"
#include "InfoText.h"
#include "Slider.h"
#include "Tabbar.h"
#include "ContentManagerThreadZip.h"
#include "ContentManagerThreadWeb.h"
#include "ContentManager.h"
#include "ContentUpload.h"
#include "ThreadedLoading.h"
#include "RenderOffScreen.h"
#include "KeyHandler.h"
#include "Caption.h"

/* MMDAgent: MMDAgent class */
class MMDAgent
{
private:
   bool m_enable;          /* enable flag */
   int m_moduleId;
   char *m_title;          /* window title */
   char *m_sysDownloadURL; /* system download URL */
   char *m_pluginDirName;  /* directory name of plugin */
   char *m_systemConfigFileName; /* system configuration .mdf file name */
   unsigned long m_tickCount;
   char *m_userDirName;    /* directory name before chdir() */
   char *m_tempDirName;    /* temporary directory name */
   char *m_configFileName; /* config file name */
   char *m_configDirName;  /* directory name of config file */
   char *m_systemDirName;  /* directory name of system */
   char *m_appDirName;     /* directory name of application data */
   char **m_argv;          /* command line arguments */
   int m_argc;             /* number of m_argv */
   bool m_loadHome;        /* load home content when no content specified at start up */
   int m_screenSize[2];
   char m_optionalStatusString[MMDAGENT_MAXBUFLEN];

   Option *m_option;        /* user options */
   ScreenWindow *m_screen;  /* screen window */
   Message *m_message;      /* message queue */
   BulletPhysics *m_bullet; /* Bullet Physics */
   Plugin *m_plugin;        /* plugins */
   Stage *m_stage;          /* stage */
   SystemTexture *m_systex; /* system texture */
   LipSync *m_lipSync;      /* system default lipsync */
   Render *m_render;        /* render */
   Timer *m_timer;          /* timer */
   KeyValue *m_keyvalue;    /* generic key-value pairs */

   FTGLTextureAtlas *m_atlas;   /* texture atlas for text drawing */
   FTGLTextureFont *m_font;     /* font information for text drawing */
   FTGLTextureFont *m_fontAwesome; /* font information for font awesome*/
   FTGLTextDrawElements m_elem; /* text drawing element holder for debug display */
   FTGLTextDrawElements m_elemAwesome; /* text drawing element holder for debug display with awesomefont */
   LogText *m_loggerMessage;    /* logger */
   LogText *m_loggerLog;        /* logger */
   Menu *m_menu;                /* menu */
   Button *m_button;            /* button */
   Button *m_buttonTop;         /* top button */
   bool m_buttonShowing;        /* true when buttons are showing */
   FileBrowser *m_filebrowser;  /* file browser */
   Prompt *m_prompt;            /* prompt handler */
   InfoText *m_infotext;        /* information text */
   Slider *m_slider;            /* slider */
   Tabbar *m_tabbar;            /* tabbar */

   PMDObject *m_model;      /* models */
   int *m_renderOrder;      /* model rendering order */
   int m_numModel;          /* number of models */
   MotionStocker *m_motion; /* motions */
   bool m_hasExtModel;      /* true if displaying any ext model */

   CameraController m_camera; /* camera controller */
   bool m_cameraControlled;   /* true when camera is controlled by motion */

   bool m_keyCtrl;           /* true if Ctrl-key is on */
   bool m_keyShift;          /* true if Shift-key is on */
   int m_selectedModel;      /* model ID selected by mouse */
   int m_highLightingModel;  /* ID of highlighted model */
   bool m_doubleClicked;     /* true if double clicked */
   int m_mousePosX;          /* mouse position in x*/
   int m_mousePosY;          /* mouse position in y*/
   bool m_leftButtonPressed; /* true if left button is on */
   double m_restFrame;       /* remaining frames */
   double m_stepFrame;        /* minimum step frame for an update */
   int m_maxStep;          /* maximim step frame count for an update */

   bool m_enablePhysicsSimulation; /* true if physics simulation is on */
   bool m_dispLog;                 /* true if log window is shown */
   bool m_dispLogConsole;          /* true if log console is shown */
   bool m_dispBulletBodyFlag;      /* true if bullet body is shown */
   bool m_dispModelDebug;          /* true if model debugger is on */
   bool m_holdMotion;              /* true if holding motion */

   ContentManager *m_content;      /* content manager */

   Button *m_contentButtons[MMDAGENT_MAXNUMCONTENTBUTTONS];       /* content buttons */

   FILE *m_fpLog;                  /* file pointer for log output */
   bool m_resetFlag;               /* true if soft reset was ordered */
   bool m_hardResetFlag;           /* true if hard reset was ordered */
   double m_startingFrame;         /* starting animation frame */
   int m_startingFrameSkipCount;   /* skipped count of starting animation frame */
   int m_startingFramePattern;     /* current starting animation pattern */
   bool m_contentInErrorPrompt;    /* true when content has error in loading and waiting for prompt */
   bool m_pluginStarted;           /* true when plugins are started */
   bool m_contentUpdateStarted;    /* true when content update check was started */
   bool m_contentUpdateChecked;    /* true when content update check was done */
   int m_contentUpdateWait;        /* 0: not waiting, 1: system update is waiting 2: content update is waiting */
   SplashScreen *m_splash;         /* splash screen renderer */
   bool m_inihibitSplash;          /* true when skip slpash screen at start up */
   bool m_contentLaunched;         /* true when content was fully launched */
   bool m_contentDocViewing;       /* true when viewing content documents */
   LogToFile *m_logToFile;         /* logging class */
   ContentUpload *m_logUploader;   /* logged file uploader */

   char *m_autoUpdateFiles;        /* auto-update files */
   double m_autoUpdatePeriod;      /* auto-update interval period in sec */
   TinyDownload *m_tinyDownload;   /* tiny downloader */

   bool m_threadsPauseFlag;        /* true when paused, false when resumed */
   GLFWmutex m_threadsPauseMutex;  /* mutex for pausing threads*/
   GLFWcond m_threadsPauseCond;    /* condition for pausing threads */
#ifdef MMDAGENT_CURRENTTIME
   double m_timeMessageFrame;
#endif
   bool m_cameraCanMove;           /* true if camera can be moved */
   double m_showUsageFrame;        /* start up message duration time */

   ThreadedLoading *m_threadedLoading;  /* threaded loading class */

   RenderOffScreen *m_offscreen;   /* off-screen rendering class */
   KeyHandler m_keyHandler;        /* key input handler */
   Caption *m_caption;

   char m_errorMessages[MMDAGENT_MAXBUFLEN];
   FTGLTextDrawElements m_elemErrorMessage;

   /* getNewModelId: return new model ID */
   int getNewModelId();

   /* removeRelatedModels: delete a model */
   void removeRelatedModels(int modelId);

   /* updateLight: update light */
   void updateLight();

   /* setHighLight: set high-light of selected model */
   void setHighLight(int modelId);

   /* updateScene: update the whole scene */
   bool updateScene();

   /* renderScene: render the whole scene */
   bool renderScene();

   /* renderAppearingAnimation: render appearing animation */
   void renderAppearingAnimation();

   /* addModel: add model */
   bool addModel(const char *modelAlias, const char *fileName, btVector3 *pos, btQuaternion *rot, bool useCartoonRendering, const char *baseModelAlias, const char *baseBoneName);

public:
   /* changeModel: change model */
   bool changeModel(const char *modelAlias, const char *fileName, PMDModel *pmd = NULL);

private:
   /* deleteModel: delete model */
   bool deleteModel(const char *modelAlias);

   /* addMotion: add motion */
   bool addMotion(const char *modelAlias, const char *motionAlias, const char *fileName, bool full, bool once, bool enableSmooth, bool enableRePos, float priority);

   /* changeMotion: change motion */
   bool changeMotion(const char *modelAlias, const char *motionAlias, const char *fileName);

   /* resetMotion: reset motion */
   bool resetMotion(const char *modelAlias, const char *motionAlias);

   /* accelerateMotion: accelerate motion */
   bool accelerateMotion(const char *modelAlias, const char *motionAlias, float speed, float durationTime, float targetTime);

   /* deleteMotion: delete motion */
   bool deleteMotion(const char *modelAlias, const char *motionAlias);

   /* configureMotion: configure motion */
   bool configureMotion(const char *modelAlias, const char *motionAlias, const char *key, const char *value);

   /* startMove: start moving */
   bool startMove(const char *modelAlias, btVector3 *pos, bool local, float speed);

   /* stopMove: stop moving */
   bool stopMove(const char *modelAlias);

   /* startTurn: start turn */
   bool startTurn(const char *modelAlias, btVector3 *pos, bool local, float speed);

   /* stopTurn: stop turn */
   bool stopTurn(const char *modelAlias);

   /* startRotation: start rotation */
   bool startRotation(const char *modelAlias, btQuaternion *rot, bool local, float spped);

   /* stopRotation: stop rotation */
   bool stopRotation(const char *modelAlias);

   /* setFloor: set floor image */
   bool setFloor(const char *fileName);

   /* setBackground: set background image */
   bool setBackground(const char *fileName);

   /* setStage: set stage */
   bool setStage(const char *fileName);

   /* setWindowFrame: set window frame */
   bool setWindowFrame(const char *fileName);

   /* changeCamera: change camera setting */
   bool changeCamera(const char *pos, const char *rot, const char *distance, const char *fovy, const char *time, const char *modelalias, const char *bonename);

   /* changeLightColor: change light color */
   bool changeLightColor(float r, float g, float b);

   /* changeLightDirection: change light direction */
   bool changeLightDirection(float x, float y, float z);

   /* startLipSync: start lip sync */
   bool startLipSync(const char *modelAlias, const char *seq);

   /* stopLipSync: stop lip sync */
   bool stopLipSync(const char *modelAlias);

   /* addBoneControl: add bone control */
   bool addBoneControl(const char *valueName, float valueMin, float valueMax, const char *modelName, const char *boneName, btVector3 *pos1, btQuaternion *rot1, btVector3 *pos2, btQuaternion *rot2);

   /* addMorphControl: add morph control */
   bool addMorphControl(const char *valueName, float valueMin, float valueMax, const char *modelAlias, const char *morphName, float fmin, float fmax);

   /* removeBoneControl: remove bone control */
   bool removeBoneControl(const char *modelAlias, const char *boneName);

   /* removeMorphControl: remove morph control */
   bool removeMorphControl(const char *modelAlias, const char *morphName);

   /* startMotionCapture: start motion capture */
   bool startMotionCapture(const char *modelAlias, const char *fileName);

   /* stopMotionCapture: stop motion capture */
   bool stopMotionCapture(const char *modelAlias);

   /* setTexureAnimationSpeed: set texture animation speed rate */
   bool setTexureAnimationSpeedRate(const char *modelAlias, const char *textureFileName, double rate);

   /* procReceivedMessage: process received message */
   void procReceivedMessage(const char *type, const char *value);

   /* procReceivedLogString: process received log string */
   void procReceivedLogString(int id, unsigned int flag, const char *log, const char *timestamp);

   /* initialize: initialize MMDAgent */
   void initialize();

   /* clear: free MMDAgent */
   void clear();

public:

   /* MMDAgent: constructor */
   MMDAgent();

   /* ~MMDAgent: destructor */
   ~MMDAgent();

   /* restart: restart MMDAgent */
   bool restart(const char *systemDirName, const char *pluginDirName, const char *systemConfigFileName, const char *sysDownloadURL, const char *title);

   /* setupSystem: setup system */
   bool setupSystem(const char *systemDirName, const char *pluginDirName, const char *systemConfigFileName, const char *sysDownloadURL, const char *title);

   /* setupContent: setup content */
   bool setupContent(int argc, char **argv);

   /* setupWorld: load world */
   bool setupWorld();

   /* updateAndRender: update and render the whole scene */
   bool updateAndRender();

   /* renderInterContentTransition: render inter-content transition */
   void renderInterContentTransition(float width, float height, float currentFrame, float maxFrame, int patternId, bool closing);

   /* restoreSurface: restore rendering surface */
   void restoreSurface();

   /* resetAdjustmentTimer: reset adjustment timer */
   void resetAdjustmentTimer();

   /* getModuleId: get module id */
   int getModuleId(const char *ident);

   /* sendMessage: send message to global message queue */
   void sendMessage(int id, const char *type, const char *format, ...);

   /* sendLogString: send log string */
   void sendLogString(int id, unsigned int flag, const char *format, ...);

   /* stopLogString: stop log string */
   void stopLogString();

   /* resumeLogString: resume log string */
   void resumeLogString();

   /* findModelAlias: find a model with the specified alias */
   int findModelAlias(const char *alias);

   /* getMoelList: get model list */
   PMDObject *getModelList();

   /* getNumModel: get number of models */
   short getNumModel();

   /* getMousePosition:: get mouse position */
   void getMousePosition(int *x, int *y);

   /* getScreenPointPosition: convert screen position to object position */
   void getScreenPointPosition(btVector3 *dst, btVector3 *src);

   /* MMDAgent::getWindowSize: get window size */
   void getWindowSize(int *w, int *h);

   /* getConfigFileName: get config file name for plugin */
   char *getConfigFileName();

   /* getConfigDirName: get directory of config file for plugin */
   char *getConfigDirName();

   /* getSystemDirName: get system directory name for plugin */
   char *getSystemDirName();

   /* getAppDirName: get application directory name for plugin */
   char *getAppDirName();

   /* getTextureFont: get texture font for plugin */
   FTGLTextureFont *getTextureFont();

   /* getKeyValue: get key-value instance for plugin */
   KeyValue *getKeyValue();

   /* getModuleName: get module name from module id for plugin */
   const char *getModuleName(int module_id);

   /* getRenderer: return renderer */
   Render *getRenderer();

   /* getMenu: return menu */
   Menu *getMenu();

   /* getFileBrowser: return file browser */
   FileBrowser *getFileBrowser();

   /* getPrompt: return prompt */
   Prompt *getPrompt();

   /* getInfoText: return imfotext */
   InfoText *getInfoText();

   /* getSlider: return slider */
   Slider *getSlider();

   /* getTabbar: return tabbar */
   Tabbar *getTabbar();

   /* setButtonsInDir: set buttons in dir */
   void setButtonsInDir(const char *dir);

   /* pointedButton: return pointed button */
   Button *pointedButton(int x, int y, int screenWidth, int screenHeight);

   /* toggleButtons: show/hide toggle buttons */
   void toggleButtons();

   /* requireSystemUpdate: check if system update is required */
   bool requireSystemUpdate();

   /* procMenu: process menu */
   void procMenu(int id, int item);

   /* createMenu: create menu */
   void createMenu();

   /* updateCurrentSystem: update current system */
   void updateCurrentSystem();

   /* deleteContents: delete contents in cache */
   void deleteContents();

   /* resetHome: reset home */
   void resetHome();

   /* deleteFavorite: delete favorites */
   void deleteFavorites();

   /* hasReadme: return if current content has readme */
   bool hasReadme();

   /* showReadme: show readme */
   void showReadme();

   /* deleteAllContentsInCache delete all data in cache */
   void deleteAllContentsInCache();

   /* setResetFlag: set reset flag */
   void setResetFlag(const char *argv);

   /* getResetFlag: return reset flag */
   bool getResetFlag();

   /* getHardResetFlag: return hard reset flag */
   bool getHardResetFlag();

   /* reload: reload current content */
   void reload();

   /* isViewMovable: return true if view can be moved */
   bool isViewMovable();

   /* pauseThreads: pause all threads */
   void pauseThreads();

   /* resumeThreads: resume paused threads */
   void resumeThreads();

   /* waitWhenPaused: wait when paused, till resume */
   void waitWhenPaused();

   /* getArguments: return arguments given at start up */
   char **getArguments(int *num_ret);

   /* inhibitSplash: skip showing splash screen at start up  */
   void inhibitSplash();

   /* procWindowDestroyMessage: process window destroy message */
   void procWindowDestroyMessage();

   /* procMouseLeftButtonDoubleClickMessage: process mouse left button double click message */
   void procMouseLeftButtonDoubleClickMessage(int x, int y);

   /* procMouseLeftButtonDownMessage: process mouse left button down message */
   void procMouseLeftButtonDownMessage(int x, int y, bool withCtrl, bool withShift);

   /* procMouseLeftButtonUpMessage: process mouse left button up message */
   void procMouseLeftButtonUpMessage();

   /* procMouseLeftButtonLongPressedMessage: process mouse left button long pressed message */
   void procMouseLeftButtonLongPressedMessage(int x, int y, int screenWidth, int screenHeight);

   /* procMouseLeftButtonLongReleasedMessage: process mouse left button long released message */
   void procMouseLeftButtonLongReleasedMessage(int x, int y, int screenWidth, int screenHeight);

   /* procMouseWheel: process mouse wheel message */
   void procMouseWheelMessage(bool zoomup, bool withCtrl, bool withShift);

   /* procMousePosMessage: process mouse position message */
   void procMousePosMessage(int x, int y, bool withCtrl, bool withShift);

   /* procMouseStatusMessage: process mouse status message */
   void procMouseStatusMessage(int x, int y, bool withCtrl, bool withShift);

   /* procFullScreenMessage: process full screen message */
   void procFullScreenMessage();

   /* procInfoStringMessage: process information string message */
   void procInfoStringMessage();

   /* procVSyncMessage: process vsync message */
   void procVSyncMessage();

   /* procShadowMessage: process shadow message */
   void procShadowMessage();

   /* procShadowMappingMessage: process shadow mapping message */
   void procShadowMappingMessage();

   /* procDisplayRigidBodyMessage: process display rigid body message */
   void procDisplayRigidBodyMessage();

   /* procDisplayWireMessage: process display wire message */
   void procDisplayWireMessage();

   /* procDisplayBoneMessage: process display bone message */
   void procDisplayBoneMessage();

   /* procCartoonEdgeMessage: process cartoon edge message */
   void procCartoonEdgeMessage(bool plus);

   /* procTimeAdjustMessage: process time adjust message */
   void procTimeAdjustMessage(bool plus);

   /* procHorizontalRotateMessage: process horizontal rotate message */
   void procHorizontalRotateMessage(bool right);

   /* procVerticalRotateMessage: process vertical rotate message */
   void procVerticalRotateMessage(bool up);

   /* procHorizontalMoveMessage: process horizontal move message */
   void procHorizontalMoveMessage(bool right);

   /* procVerticalMoveMessage: process vertical move message */
   void procVerticalMoveMessage(bool up);

   /* procCameraMoveMessage: process camera move message */
   void procCameraMoveMessage(bool zoomup, float stepFactor);

   /* procCameraFovyMessage: process camera fovy change message */
   void procCameraFovyMessage(bool zoomup);

   /* procMoveResetMessage: process move reset message */
   void procMoveResetMessage();

   /* procToggleCameraMoveMessage: process toggle camera move message */
   void procToggleCameraMoveMessage();

   /* procDeleteModelMessage: process delete model message */
   void procDeleteModelMessage();

   /* procPhysicsMessage: process physics message */
   void procPhysicsMessage();

#ifdef MY_LUMINOUS
   /* procLuminousMessage: process luminous message */
   void procLuminousMessage();
#endif

   /* procShaderEffectMessage: process shader effect message */
   void procShaderEffectMessage();

   /* procShaderEffectScalingMessage: process shader effect scaling message */
   void procShaderEffectScalingMessage();

#ifdef MY_RESETPHYSICS
   /* procResetPhysicsMessage: process physics reset message */
   void procResetPhysicsMessage();
#endif

   /* procDisplayLogMessage: process display log message */
   void procDisplayLogMessage();

   /* procDisplayLogConsoleMessage: process display log console message */
   void procDisplayLogConsoleMessage();

   /* procHoldMessage: process hold message */
   void procHoldMessage();

   /* procWindowSizeMessage: process window size message */
   void procWindowSizeMessage(int x, int y);

   /* procLogNarrowingMessage: process log narrowing message */
   void procLogNarrowingMessage();

   /* procKeyMessage: process key message */
   bool procKeyMessage(int key, int action);

   /* procCharMessage: process char message */
   bool procCharMessage(char c);

   /* procScrollLogMessage: process log scroll message */
   void procScrollLogMessage(bool up);

   /* procDropFileMessage: process file drops message */
   void procDropFileMessage(const char *file, int x, int y);

   /* saveScreenShot: save screen shot */
   void saveScreenShot(const char *filename);

   /* callHistory: call history */
   void callHistory();

   /* getUserContext: get user context */
   void getUserContext(const char *mdffile);

   /* procOpenContentDirMessage: process open content dir message */
   void procOpenContentDirMessage();

   /* procOpenContentFileMessage: process open content file message */
   void procOpenContentFileMessage();

   /* procSaveWindowPlacementMessage: process save window placement message */
   void procSaveWindowPlacementMessage();

   /* procLoadWindowPlacementMessage: process load window placement message */
   void procLoadWindowPlacementMessage();

   /* procToggleTitleBarMessage: process toggle title bar message */
   void procToggleTitleBarMessage();

#ifdef MMDAGENT_CURRENTTIME
   void issueTimeMessage(double ellapsedFrame);
#endif

   /* setLoadHomeFlag: set load home flag */
   void setLoadHomeFlag(bool flag);

   /* getLoadHomeFlag: get load home flag */
   bool getLoadHomeFlag();

   /* getHomeDup: get home */
   char *getHomeDup();

   /* procToggleDoppelShadowMessage: process toggle doppel shadow message */
   void procToggleDoppelShadowMessage();

   /* setLog2DFlag: set log 2d flag */
   void setLog2DFlag(bool flag);

   /* getLogBaseHeight: set log base height */
   float getLogBaseHeight();

   /* getBulletPhysics: get bullet physics instance */
   BulletPhysics *getBulletPhysics();

   /* getSystemTexture: get system texture */
   SystemTexture *getSystemTexture();

   /* getStartingFrameLeft: get starting animation frame left */
   double getStartingFrameLeft();

   /* setOptionalStatusString: set optional status string */
   void setOptionalStatusString(const char *str);

   /* getKeyHandler: get key handler */
   KeyHandler *getKeyHandler();
};

class SplashScreen {
private:
   PMDTexture *m_tex;
   int m_screenWidth;
   int m_screenHeight;
   float m_width;
   float m_height;
   double m_duration;
   double m_maxDuration;
   double m_transFrame;
   Timer *m_timer;
   GLfloat m_vertices[12];
   GLfloat m_texcoords[8];
   bool m_active;

   /* initialize: initialize SplashScreen */
   void initialize();

   /* clear: free SplashScreen */
   void clear();

   /* updateScreen: update screen parameter */
   bool updateScreen(MMDAgent *mmdagent);

public:

   /* SplashScreen: constructor */
   SplashScreen();

   /* ~SplashScreen: destructor */
   ~SplashScreen();

   /* start: start */
   bool start(const char *appDirName);

   /* render: render */
   bool render(MMDAgent *mmdagent);

   /* end: end */
   void end();

   /* setActiveFlag: set active flag */
   void setActiveFlag(bool flag);

   /* isActive: return true when active */
   bool isActive();

   /* terminate: terminate animation */
   void terminate();
};

class LogToFile {
private:
   FILE *m_fp;
   char *m_dirName;
   int m_totalSize;
   int m_maxSize;
   bool m_recording;

   /* initialize: initialize LogToFile */
   void initialize();

   /* clear: free LogToFile */
   void clear();

public:

   /* LogToFile: constructor */
   LogToFile();

   /* ~LogToFile: destructor */
   ~LogToFile();

   /* open: open log file for writing */
   bool open(const char *contentDirName, const char *logIdentifier);

   /* save: save log message to file */
   void save(const char *string);

   /* close: close log file */
   void close();

   /* getDirName: return dir name */
   const char *getDirName();

   /* isLogging: return true while logging */
   bool isLogging();

   /* setRecordingFlag: set recording flag */
   void setRecordingFlag(bool flag);

   /* getRecordingFlag: get recording flag */
   bool getRecordingFlag();
};

#endif /* __mmdagent_h__ */
