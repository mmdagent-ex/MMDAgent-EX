Install HowTo

AndroidStudio 3.2 / MacOS Sierra / Android 21 and later

1) Install AndroidStudio-3.2

2) Launch AndroidStudio

3) Create a new project in AndroidStudio

   - Start a new Android Studio Project
     - New Project
       - Application name: Pocket MMDAgent
       - Company Domain: lee-lab.org
       - Project location: anywhere you like
     - Target Android Devices
       - Check Phone and Tablet
       - Choose API 21: Android 5.0
     - Add an activity to Mobile
       - Choose "Add No Activity"

   Required modules (JDK, SDK, NDK, etc...) will be installed automatically.
   Also recommends to use the internal JDK bundled within AndroidStudio
   instead of any Java environment you have been installed by your own, 
   since the bundled version always has full compatibility!

4) Place MMDAgent sources to jni directory


   % cd $(PROJECT_LOCATION)/app/src/main
   % git clone (MMDAgent-L's repository)
   % mv MMDAgent-L jni

5) Set up files

   % cd Android/3.2
   % make install

   This will copy build.gradle, AndroidManifest.xml and icon files to
   the android project to parent dirs.

6) Connect your Android device via USB.

7) Go build!

8) If you want to build at command line, for debug build,

   % cd $(PROJECT_LOCATION)
   % export PATH=$PATH:/Applications/"Android Studio.app"/Contents/jre/jdk/Contents/Home/bin
   % export JAVA_HOME=/Applications/"Android Studio.app"/Contents/jre/jdk/Contents/Home
   % ./gradlew assembleDebug

the setting of PATH and JAVA_HOME above is required to use the internal bundled JDK at command line, thus needed to run gradle.

The output is an apk file located in app/build/outputs/apk.
