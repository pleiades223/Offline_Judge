# Offline_Judge
![image](https://github.com/pleiades223/Offline_Judge/assets/96909412/0f396c8f-ee29-455b-aa9e-d9d0fd57bead)
## Overview
事前にテストケースをダウンロードしておけばオフラインでもジャッジができるなにか。

## Requirement
- Windows
- OpenSiv3D v0.6.13
- Python3
- MSVC v143 - VS 2022 C++ x64/x86 ビルド ツール

## Usage
App/data/にcontestsフォルダを作成し以下のように配置する
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
