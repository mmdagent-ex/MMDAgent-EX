# MMDAgent-EX

[![en](https://img.shields.io/badge/lang-en-red.svg)](https://github.com/mmd-agent/MMDAgent-EX/blob/main/README.md)
[![ja](https://img.shields.io/badge/lang-ja-blue.svg)](https://github.com/mmd-agent/MMDAgent-EX/blob/main/README.ja.md)

---

(注意事項)

- This repository contains no content assets (3D model, dialogue script, etc.).

---

## What is this?

MMDAgent-EX is an open-source R&D platform for spoken dialogue systems and avatar communication.  This system consists of built-in CG rendering engine with speech processing modules and controllers, that enables building an interactive, multi-modal spoken dialogue systems of your own. The system can also be operated as a CG avatar in that one can remotely control CG avatar, by transmitting his voice and face-tracking information from remote site.

History: the original MMDAgent was developed at the Nagoya Institute of Technology (NITech) at 2011 as an open source toolkit for spoken dialogue research. After its development, MMDAgent-EX has been developed at Lee Lab in NITech, with improved preformance, lots of enhancements, network support and much more. On December 2020, this software development has been assited by the moonshot R&D "Avatar Symbiomic Society" project, which aims to integrate avatar communication with spoken dialogue system to realize "Autonomous and Remote Fusion Dialogue Platform".

MMDAgent-EX has an OpenGL-based rendering engine compatible to MMD (MikuMikuDance) software.  It can play MMD format 3D models and motions in real time.  It has its own speech recognition and speech synthesis engine, as well as a primitive dialogue scripting function, making it possible to build a spoken dialogue system with this sytem.  It can also be fully controllable from other process, thus one can connect MMDAgent-EX with Python and other tools, thus can easily incorporate cloud-based services like speech-to-text, text-to-speech and LLM chatbots.

## Documents

Documents are located under the "Documents" folder.

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

MMDAgent-EX is distributed under Simplified BSD License.

Also MMDAgent-EX depends on various open-source softwares and libraries. Some of them are also included in this repository.  For licensing issues, please consult each license of the software:

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
- PortAudio
- Open JTalk
- Julius
- POCO C++ Libraries
- OpenCV
- FFmpeg
- RabbitMQ

When using materials in this system, i.e. MMD 3D model, music, motions etc., please take care of their licenses.

## Disclaimer

This software has been developed since 2010 by essentially one university professor, who is in their 50s as of 2023. It was originally designed for research purposes, and it does not guarantee the existence of a complete specification or documentation. Some functions might not be documented or may not behave as expected.

The coding style is somewhat dated, deprecated functions are often used (you will see tons of warnings during the build process). Some implementations are based on non-standard way.  The code is not particularly well-organized, and its readability may be considered low.

As with most open-source software, this software is provided "AS IS". Please be aware of the aforementioned points and use it at your own discretion.

## Acknowledgments

MMDAgent-EX:

- Akinobu Lee (Nagoya Institute of Technology, Japan)

MMDAgent:

- Keiichi Tokuda (Nagoya Institute of Technology, Japan)
- Akinobu Lee (Nagoya Institute of Technology, Japan)
- Keiichiro Oura (Nagoya Institute of Technology, Japan)
- Daisuke Yamamoto (Nagoya Institute of Technology, Japan)
