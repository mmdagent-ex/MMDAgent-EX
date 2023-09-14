
all:
	@echo "----------------------------------------------------------------------------------"
	@echo "  make x11              to compile MMDAgent for X11 on Unix-like systems"
	@echo "  make x11-clean        to remove any files generated for X11 on Unix-like systems"
	@echo "----------------------------------------------------------------------------------"
	@echo "  make cocoa            to compile MMDAgent for Cocoa on Mac OS X"
	@echo "  make cocoa-clean      to remove any files generated for Cocoa on Mac OS X"
	@echo "----------------------------------------------------------------------------------"
	@echo "  make android          to compile MMDAgent for Android"
	@echo "  make android-clean    to remove any files generated for Android"
	@echo "----------------------------------------------------------------------------------"

x11:
	cd Library_Bullet_Physics && make
	cd Library_GLFW           && make
	cd Library_GLee           && make
	cd Library_JPEG           && make
	cd Library_libpng         && make
	cd Library_FreeType       && make
	cd Library_MMDAgent       && make
	cd Library_MMDFiles       && make
	cd Library_zlib           && make
	cd Library_PortAudio      && make
	cd Library_RE2            && make
	cd Library_hts_engine_API && make
	cd Library_Open_JTalk     && make
	cd Library_Flite_plus_hts_engine && make
	cd Library_Julius         && make
	cd Library_librdkafka     && make
	cd main                   && make
	cd Plugin_VIManager       && make
	cd Plugin_Variables       && make
	cd Plugin_LookAt          && make
	cd Plugin_Open_JTalk      && make
	cd Plugin_Flite_plus_hts_engine && make
	cd Plugin_Julius          && make
	cd Plugin_Audio           && make
	cd Plugin_ErrorBlock      && make
	cd Plugin_Network         && make
	cd Plugin_Remote          && make
	cd Plugin_TextArea        && make
	cd Plugin_Kafka           && make
#	cd Plugin_AnyScript       && make

x11-clean:
	cd Library_Bullet_Physics && make clean
	cd Library_GLFW           && make clean
	cd Library_GLee           && make clean
	cd Library_JPEG           && make clean
	cd Library_libpng         && make clean
	cd Library_FreeType       && make clean
	cd Library_MMDAgent       && make clean
	cd Library_MMDFiles       && make clean
	cd Library_zlib           && make clean
	cd Library_PortAudio      && make clean
	cd Library_RE2            && make clean
	cd Library_hts_engine_API && make clean
	cd Library_Open_JTalk     && make clean
	cd Library_Flite_plus_hts_engine && make clean
	cd Library_Julius         && make clean
	cd Library_librdkafka     && make clean
	cd main                   && make clean
	cd Plugin_VIManager       && make clean
	cd Plugin_Variables       && make clean
	cd Plugin_LookAt          && make clean
	cd Plugin_Open_JTalk      && make clean
	cd Plugin_Flite_plus_hts_engine && make clean
	cd Plugin_Julius          && make clean
	cd Plugin_Audio           && make clean
	cd Plugin_ErrorBlock      && make clean
	cd Plugin_Network         && make clean
	cd Plugin_Remote          && make clean
	cd Plugin_TextArea        && make clean
	cd Plugin_Kafka           && make clean
	cd Plugin_AnyScript       && make clean

cocoa:
	cd Library_Bullet_Physics && make
	cd Library_GLFW           && make -f Makefile.cocoa
	cd Library_GLee           && make
	cd Library_JPEG           && make
	cd Library_libpng         && make
	cd Library_FreeType       && make
	cd Library_MMDAgent       && make -f Makefile.cocoa
	cd Library_MMDFiles       && make -f Makefile.cocoa
	cd Library_zlib           && make
	cd Library_PortAudio      && make -f Makefile.cocoa
	cd Library_RE2            && make
	cd Library_hts_engine_API && make
	cd Library_Open_JTalk     && make
	cd Library_Flite_plus_hts_engine && make
	cd Library_Julius         && make
	cd Library_librdkafka     && make -f Makefile.cocoa
	cd main                   && make -f Makefile.cocoa
	cd Plugin_VIManager       && make -f Makefile.cocoa
	cd Plugin_Variables       && make -f Makefile.cocoa
	cd Plugin_Audio           && make -f Makefile.cocoa
	cd Plugin_LookAt          && make -f Makefile.cocoa
	cd Plugin_Open_JTalk      && make -f Makefile.cocoa
	cd Plugin_Flite_plus_hts_engine && make -f Makefile.cocoa
	cd Plugin_Julius          && make -f Makefile.cocoa
	cd Plugin_TextArea        && make -f Makefile.cocoa
	cd Plugin_ErrorBlock      && make -f Makefile.cocoa
	cd Plugin_Network         && make -f Makefile.cocoa
	cd Plugin_Kafka           && make -f Makefile.cocoa

cocoa-clean:
	cd Library_Bullet_Physics && make                   clean
	cd Library_GLFW           && make -f Makefile.cocoa clean
	cd Library_GLee           && make                   clean
	cd Library_JPEG           && make                   clean
	cd Library_libpng         && make                   clean
	cd Library_FreeType       && make                   clean
	cd Library_MMDAgent       && make                   clean
	cd Library_MMDFiles       && make                   clean
	cd Library_zlib           && make                   clean
	cd Library_PortAudio      && make -f Makefile.cocoa clean
	cd Library_RE2            && make                   clean
	cd Library_hts_engine_API && make                   clean
	cd Library_Open_JTalk     && make                   clean
	cd Library_Flite_plus_hts_engine && make                   clean
	cd Library_Julius         && make                   clean
	cd Library_librdkafka     && make -f Makefile.cocoa clean
	cd main                   && make -f Makefile.cocoa clean
	cd Plugin_VIManager       && make -f Makefile.cocoa clean
	cd Plugin_Variables       && make -f Makefile.cocoa clean
	cd Plugin_Audio           && make -f Makefile.cocoa clean
	cd Plugin_LookAt          && make -f Makefile.cocoa clean
	cd Plugin_Open_JTalk      && make -f Makefile.cocoa clean
	cd Plugin_Flite_plus_hts_engine && make -f Makefile.cocoa clean
	cd Plugin_Julius          && make -f Makefile.cocoa clean
	cd Plugin_TextArea        && make -f Makefile.cocoa clean
	cd Plugin_ErrorBlock      && make -f Makefile.cocoa clean
	cd Plugin_Network         && make -f Makefile.cocoa clean
	cd Plugin_Kafka           && make -f Makefile.cocoa clean

android:
	ndk-build NDK_DEBUG=0

android-clean:
	ndk-build clean

cocoa-package:
	cd main && make -f Makefile.cocoa dmg
