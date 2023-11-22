# MMDAgent-EX

[![en](https://img.shields.io/badge/lang-en-red.svg)](README.md)
[![ja](https://img.shields.io/badge/lang-ja-blue.svg)](README.ja.md)

---

## Cloning the Project

This project uses Git LFS (Large File Storage) to manage binary files. If Git LFS is not installed in your environment, this project would not be cloned correctly.

Before cloning the project, **please make sure to install Git LFS**. Or you should use some **LFS-enabled Git app** such as [GitHub Desktop](https://desktop.github.com/).

### Installing Git LFS on Linux/Ubuntu

```shell
sudo apt install git-lfs
```

Verify:

```shell
git lfs version
```

### Installing Git LFS on macOS

```shell
brew install git-lfs
```

Verify:

```shell
git lfs version
```

###  Installing Git LFS onWindows

1. Download the Git LFS installer for Windows from the [Git LFS download page](https://git-lfs.github.com/) and run it.
2. Follow the installer instructions to install Git LFS.

---

## What is this?

MMDAgent-EX is an open-source platform for voice dialogue systems and avatar communication. It is a standalone system that combines the display and control of CG characters with speech and language processing, allowing you to create various voice dialogue systems. By adding various modules, you can build diverse multimodal dialogue systems. Additionally, it supports features such as lip-synced speech playback and motion playback/control from external software, making it possible to operate as a human-agent frontend for a dialogue system.

MMDAgent was developed at Nagoya Institute of Technology since 2011 as an open-source toolkit for research and development in voice interaction.  Besides the its original version, it has been internally developed continuously for research purposes: support various formats, network capabilities, UI improvements, external control functions, and more.  Since December 2020, its research and development have been progressing in the "Avatar Symbiosis Society" JST moonshot program, as the platform for CG avatar communication. Here, we release some of the research and development results as open source.

MMDAgent-EX has its own OpenGL-based rendering engine compatible with MMD (MikuMikuDance), and can create interactive characters using 3D models and motion data in MMD format. It also includes speech recognition and synthesis engines developed at Nagoya Institute of Technology, such as Julius and Open JTalk, as well as an embedded FST-based state-transition primitive dialogue scripting feature, allowing you to build voice dialogue systems independently. Aside from the internal engines, it can be easily connected to other scripts like Python, so you can add another modules such as cloud-based speech recognition, speech synthesis engines, or large language models like ChatGPT with this system. Furthermore, it supports external control via socket connections, making it suitable for use as the input/output frontend for existing dialogue systems.

Short Notice:

- This repository contains no content assets (3D model, dialogue script, etc.).

## Documents

All up-to-date documents are at the [development site](https://mmdagent-ex.dev/).

This repository just contains the system. Content asset (3D model, dialogue script, etc.) is not included in this system.

## License

MMDAgent-EX is distributed under the Apache License, Version 2.0.

MMDAgent-EX depends on various open-source softwares and libraries. Some of them are also included in this repository.  For licensing issues, please consult each license of the software:

- MMDAgent
- Bullet Physics
- FreeType
- GLFW
- GLEW
- libjpeg
- libpng
- librdkafka
- libsndfile
- libsamplerate
- UTF8-CPP
- google/re2
- zlib
- PortAudio
- Open JTalk
- Julius
- POCO C++ Libraries
- RabbitMQ
- cpp-httplib

When using materials in this system, i.e. MMD 3D model, music, motions etc., please take care of their licenses.

## ELSI Guidelines

These guidelines outline the ethical, legal and social issues to be observed when using MMDAgent-EX.

Disallowed usage of our software:

- Illegal activity
- Child Sexual Abuse Material or any content that exploits or harms children
- Promotion of hateful, harassing, or violent content
- Activity that has high risk of physical harm, including:
  - Weapons development
  - Military and warfare
  - Management or operation of critical infrastructure in energy, transportation, and water
  - Content that promotes, encourages, or depicts acts of self-harm, such as suicide, cutting, and eating disorders
- Fraudulent or deceptive activity, including:
  - Scams
  - Coordinated inauthentic behavior
  - Plagiarism
  - Disinformation
  - Impersonation or misrepresentation of an actual person, group, or organization for the purpose of deceiving others
- Actions harmful to the MikuMikuDance related community and its fun society:
  - Creation and disribution of contents that violates the rights of copyright holders of models, motions, music, etc.
  - Ignoring guidelines set by the copyright holders.
  - Unauthorized redistribution, plagiarism, and impersonation of the original creators.

## Links

- [Lee-Lab, Nitech](https://www.slp.nitech.ac.jp/en/)

## Acknowledgments

MMDAgent-EX:

- Akinobu Lee (Nagoya Institute of Technology, Japan)

MMDAgent:

- Keiichi Tokuda (Nagoya Institute of Technology, Japan)
- Akinobu Lee (Nagoya Institute of Technology, Japan)
- Keiichiro Oura (Nagoya Institute of Technology, Japan)
- Daisuke Yamamoto (Nagoya Institute of Technology, Japan)
