# M5UnitUnified example 
## MultipleUnits
### Overview
This is a demo program that was shown at MFT2024 and M5JPTour2024.  
It displays information on 4 units (Vmeter, TVOC, ENVIII, HEART) via UnitPaHub2.  
The address of UnitPaHub2 must be 0x71 (because it conflicts with ENVIII).  
See also https://docs.m5stack.com/en/unit/pahub2

NOTICE: Use Core devices capable of displaying 320x240 pixels  

### ArduinoIDE
Install each unit's library with the library manager.

### PlatformIO
For convenience of unit testing, the libraries for each unit are registered in lib\_deps.  
Use the env of the applicable Core device.

---
### 概要
MFT2024, M5JPTour2024 にて公開されていたデモプログラムです。  
UnitPaHub2 を介して 4つのユニット (Vmeter, TVOC, ENVIII, HEART) の情報を表示します。  
UnitPaHub2 のアドレスを 0x71 にする必要があります(ENVIII と衝突するため)  
こちらを参照 https://docs.m5stack.com/en/unit/pahub2

注意: 320x240 ピクセルの表示ができる Core デバイスを使用してください。

### ArduinoIDE
各ユニットのライブラリをライブラリマネージャでインストールしてください。

### PlatformIO
ユニットテストの都合上、各ユニット毎のライブラリは lib\_deps に登録されています
該当する Core デバイスの env を使用しください。

