# Offline Judge
![image](https://github.com/pleiades223/Offline_Judge/assets/96909412/f3f0cb1c-69a6-40d7-87ca-f5a62b6ad54c)

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
