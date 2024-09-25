# M5UnitUnified example 
## ComponentOnly
### Overview
This is an example of using only the component of unit without the management mechanism of the unit.
The source uses UnitCO2, so if you want to use other units, you will need to modify it accordingly.

### ArduinoIDE
Install each unit's library with the library manager.

### PlatformIO
For convenience of unit testing, the libraries for each unit are registered in lib\_deps.  
Use the env of the applicable Core device.

---
### 概要
ユニットの管理機構を使わず、各ユニットのコンポーネントのみで使用する例です。
ソースでは UnitCO2 を使用していますので、他のユニットを使用する場合は適宜変更が必要です。

### ArduinoIDE
各ユニットのライブラリをライブラリマネージャでインストールしてください。

### PlatformIO
ユニットテストの都合上、各ユニット毎のライブラリは lib\_deps に登録されています
該当する Core デバイスの env を使用しください。

