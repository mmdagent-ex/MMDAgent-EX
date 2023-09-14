# MMDAgent-EX

[![en](https://img.shields.io/badge/lang-en-red.svg)](https://github.com/mmd-agent/MMDAgent-EX/blob/main/README.md)
[![ja](https://img.shields.io/badge/lang-ja-blue.svg)](https://github.com/mmd-agent/MMDAgent-EX/blob/main/README.ja.md)

---

(注意事項)

- This repository contains no content assets (3D model, dialogue script, etc.).
- This system can be used only for acadmic purpose.  Please contact us for commercial use.

---

## What is this?

MMDAgent-EX is an R&D platform for spoken dialogue systems and avatar communication.  This system consists of MMD-compatible CG rendering engine with speech and language processing modules, that enables building a multi-modal spoken dialogue systems of your own. The system can also operate as a CG avatar in that one can remotely control CG avatar remotely, by sending voice and face-tracking information from remote site.

History: the original MMDAgent was developed at the Nagoya Institute of Technology in 2011 as an open source toolkit for spoken dialogue research. After its development, MMDAgent-EX has been developed at the Lee Lab, including various extensions for research and experiments such as network support, etc.  On December 2020, this software development has been assited by the moonshot R&D "Avatar Symbiomic Society" project, which aims to integrate avatar communication with spoken dialogue system to realize "Autonomous and Remote Fusion Dialogue Platform".

MMDAgent-EX itself has a proprietary OpenGL-based rendering engine compatible with MMD (MikuMikuDance), and can build dialogue characters using MMD 3D models and motions. It has its own speech recognition and speech synthesis engine, as well as a primitive dialogue scripting function, making it possible to build a spoken dialogue system only with this sytem. It is also easy to integrate cloud-based services like speech-to-text, text-to-speech and chatbots with LLM.

## Documents

Documents are located in "Documents" folder.

- Set up
- How to build
- Tutorial
- How to use
- Configuration
- Communication
- External app
- Command references

This repository just contains the system (main app + plugins).  Contents (3D model, dialogue script, etc.) is not included in this system.

## License

### Short summary

- The main system source code is developed under the Modified BSD License.
- This system can be used for academic purpose.
- Please do not re-distribute this system to others.
- When using content resources, i.e. MMD 3D model, music, motions etc., with this system, please take care of their licenses.
- This software is not free, but we are willing to promote commercial use of this software.  Please contact us for commercial use.
- This license shall be changed without notice in the future.

### Speech recognition module

The speech recognition module in "Release/AppData/msasr" has been developed at Kawahara Lab., Kyoto University.  To use, please read the README and LICENSE document:

- [Release/AppData/msasr/src/README.md](https://github.com/mmd-agent/MMDAgent-EX/blob/main/Release/AppData/msasr/src/README.md)
- [Release/AppData/msasr/src/LICENSE-r2asr.txt](https://github.com/mmd-agent/MMDAgent-EX/blob/main/Release/AppData/msasr/src/LICENSE-r2asr.txt)

### Libraries

MMDAgent-EX depends on various open-source softwares and libraries.  Please consult the license of each software at your use:

- Bullet Physics
- FreeType
- GLFW
- GLEW
- libjpeg
- libpng
- librdkafka
- Libsndfile
- UTF8-CPP
- google/re2
- zlib
- POCO C++ Libraries
- OpenCV
- FFmpeg
- PortAudio
- Open JTalk
- Julius

## Acknowledgments

MMDAgent-EX:

- Akinobu Lee (Nagoya Institute of Technology, Japan)

MMDAgent:

- Keiichi Tokuda (Nagoya Institute of Technology, Japan)
- Akinobu Lee (Nagoya Institute of Technology, Japan)
- Keiichiro Oura (Nagoya Institute of Technology, Japan)
- Daisuke Yamamoto (Nagoya Institute of Technology, Japan)
