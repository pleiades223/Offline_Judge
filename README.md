# Offline Judge
![image](https://github.com/pleiades223/Offline_Judge/assets/96909412/275c6f24-fa49-4429-801e-ca1069d55149)

## Overview
事前にテストケースをダウンロードしておけばオフラインでもジャッジができるなにか。
C++とPythonが使えます。

## Requirement
- Windows
- OpenSiv3D v0.6.13
- Python 3
- MSVC v143 - VS 2022 C++ x64/x86 ビルド ツール

## Usage
App/data/にcontestsフォルダを作成し以下のように配置してください。
```
App/data/contests
├── contestname1
│   ├── A
│   │   ├── in
│   │   │   ├──sample_01.txt
│   │   │   └──sample_02.txt
│   │   └── out
│   │       ├──sample_01.txt
│   │       └──sample_02.txt
│   └── B
└── contestname2
```
