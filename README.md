# MMDAgent-EX

[![en](https://img.shields.io/badge/lang-en-red.svg)](README.md)
[![ja](https://img.shields.io/badge/lang-ja-blue.svg)](README.ja.md)

---

Short Notice:

This repository contains no content assets (3D model, dialogue script, etc.).

---

## What is this?

MMDAgent-EX is an open-source platform for voice dialogue systems and avatar communication. It is a standalone system that combines the display and control of CG characters with speech and language processing, allowing you to create various voice dialogue systems. By adding various modules, you can build diverse multimodal dialogue systems. Additionally, it supports features such as lip-synced speech playback and motion playback/control from external software, making it possible to operate as a human-agent frontend for a dialogue system.

MMDAgent was developed at Nagoya Institute of Technology since 2011 as an open-source toolkit for research and development in voice interaction.  Besides the its original version, it has been internally developed continuously for research purposes: support various formats, network capabilities, UI improvements, external control functions, and more.  Since December 2020, its research and development have been progressing in the "Avatar Symbiosis Society" JST moonshot program, as the platform for CG avatar communication. Here, we release some of the research and development results as open source.

MMDAgent-EX has its own OpenGL-based rendering engine compatible with MMD (MikuMikuDance), and can create interactive characters using 3D models and motion data in MMD format. It also includes speech recognition and synthesis engines developed at Nagoya Institute of Technology, such as Julius and Open JTalk, as well as an embedded FST-based state-transition primitive dialogue scripting feature, allowing you to build voice dialogue systems independently. Aside from the internal engines, it can be easily connected to other scripts like Python, so you can add another modules such as cloud-based speech recognition, speech synthesis engines, or large language models like ChatGPT with this system. Furthermore, it supports external control via socket connections, making it suitable for use as the input/output frontend for existing dialogue systems.

## Documents

See the "Documents" folder for tool documents.

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

MMDAgent-EX depends on various open-source softwares and libraries. Some of them are also included in this repository.  For licensing issues, please consult each license of the software:

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

(The content in this section represents the author's opinion and is not the software's terms of use)

This software has been developed since 2010 by essentially one, rather old-aged university professor. It was originally designed for research purposes, and it does not guarantee the existence of a complete specification or documentation. Some functions might not be documented or may not behave as expected.

The coding style is somewhat dated, deprecated functions are often used (you will see tons of warnings during the build process). Some implementations are based on non-standard way.  The code is not particularly well-organized, and its readability may be considered low.

As with most open-source software, this software is provided "AS IS". Please be aware of the aforementioned points and use it at your own discretion.

Furthermore, this software is also related to the culture of MMD. Users are encouraged to understand and respect the culture of MMD at using the materials. When using materials such as 3D models, motions, sounds, and images with this system, it is our sincere hope that users not only comply with the terms of use specified by their respective authors (rights holders) but also use them in a manner that shows deep respect for the creators and their creative worlds.

## Acknowledgments

MMDAgent-EX:

- Akinobu Lee (Nagoya Institute of Technology, Japan)

MMDAgent:

- Keiichi Tokuda (Nagoya Institute of Technology, Japan)
- Akinobu Lee (Nagoya Institute of Technology, Japan)
- Keiichiro Oura (Nagoya Institute of Technology, Japan)
- Daisuke Yamamoto (Nagoya Institute of Technology, Japan)
