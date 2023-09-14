Use AndroidStudio 2.0 and later. (Tested in 2.2.3)

1) Install AndroidStudio.

2) Launch AndroidStudio.

3) Will be asked to setup Android SDK, so do that.   

4) Install NDK.

   4-1) In Android Studio,
       - File > Settings
       - Appearance & Behavior > System Settings > Android SDK > SDK Tools > "NDK" > Apply

   4-2) If above menu is not found, directly download from

          http://developer.android.com/intl/ja/ndk/downloads/index.html
        
        and place the unpacked NDK on c:\Users\(username)\AppData\Local\Android
	(where SDK installed above exists as "sdk" dir)

5) Create a new project in AndroidStudio

   - Start a new Android Studio Project
     - New Project
       - Application name: MMDAgent
       - Company Domain: example.com
       - Project location: anywhere you like
     - Target Android Devices
       - Check Phone and Tablet
       - Choose API 21: Android 5.0
     - Add an activity to Mobile
       - Choose Add No Activity
       - Click "Finish"

   After opening the new project, showing folder view is recommended to do
   the following procedure.  You can switch to folder view by:
          click the "1: Project"
          click "Android" tab at top left
          select "Project"

6) Place MMDAgent sources to jni directory

   Create jni folder:

   - open "app/src/main" in the folder view and right click on the "main"
   - choose New > Folder > JNI Folder
   - choose finish

   then, place all the files in "MMDAgent" under the jni dir.
   It should look like this:

         app/src/main/jni/
			  Library_*/
		    	  main/
		    	  Plugin_*/
		    	  README.txt
		    	  ...

7) Set up application resources

     - copy all icon files in "Android/res/" to "(Project location)/app/src/main/res/"

8) Set up configurations

   Manually set up these files:

      1) (Project location)/local.properties

          check if "sdk.dir" and "ndk.dir" are correct.
          if you've installed NDK from menu, it should be set correct.
          if youve installed NDK manually, set the path here.

      2) (Project location)/app/build.gradle

      	  set up to use "ndk-build" instead of gradle-based build system.	  
          See Android/build.gradle.* for example.

      3) (Project location)/app/src/main/AndroidManifest.xml

          modify for MMDAgent.
          See Android/AndroidManifest.xml (you can copy it if you are from 1.5 to 2.2)

9) Connect your Android device via USB.

   	   - require Android 5.0 and above.
	   - require network connection to download system data when start building.

10) Set up "Application.mk" and "Android.mk" at the "jni" top dir to
   select the architecture of your device (arm / x86).

11) Go for build and test! [Run] - [Run...]

    At device selection, always select your connected hardware device.

    It will take about several minutes before build process finishes.
    You can see the building process by clicking "gradle console" on
    the right bottom side of the window.

12) After build, MMDAgent will launch automatically.
    
    At first launch, it will try to download 100MBytes of system data
    from our web site.  it will be cached on your device unless uninstalled.
    
    If fails to execute .mmda file or .mdf file, set app permission of MMDAgent
    to enable storage access in Android setting menu.