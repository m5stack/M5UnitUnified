
# M5UnitUnified(α リリース)

[English](README.md)

**M5Stack に色々な M5 ユニットをつないで扱う為の新たなアプローチ**  
M5Stack シリーズ、 M5Unitシリーズの為のライブラリです。

**注意: 現在αバージョンです**  
ご意見ご要望などは Issue または PR にてお願いします。

## 概要
M5UnitUnified は、様々な M5 ユニット製品を統一的に扱うためのライブラリです。

### APIの統一化
各ユニットの外部ライブラリは、それぞれ独自の API デザインがされています。  
基本的なAPIを統一し、すべてのユニットが同じように扱えるようにします。

### 接続と通信の統一化
各ユニットの外部ライブラリは独自の通信機能と前提条件が必要です。  
前提条件や通信方法を統一化します。  
将来的には[M5HAL(Hardware Abstraction Layer)](https://github.com/m5stack/M5HAL)と連携し、各ユニットとの通信を統一する予定です。

### ライセンスの統一化
各ユニットの外部ライブラリのライセンスは様々な物が混在しています。  
すべてのM5UnitUnifiedおよび関連ライブラリは、[MITライセンス](LICENSE)下にあります。


## インストール方法
Arduino/PlatformIO のライブラリへの登録はベータ版移行を予定しています。  
それまでは手動でのインストールをお願いします。

### ArduinoIDE
1.ZIP ファイルを各リポジトリからダウンロードします
- M5UnitUnified
- [M5Utility](https://github.com/m5stack/M5Utility/)
- [M5HAL](https://github.com/m5stack/M5HAL)
- [Unit to be used](#supported-units)
2. あなたの ArduinoIDE のライブラリフォルダへ展開します

### PlatformIO
1. platformio.ini の lib_deps に記述します
```ini
lib_deps= https://github.com/m5stack/M5UnitUnified
  https://github.com/m5stack/M5Utility
  https://github.com/m5stack/M5HAL
  https://github.com/m5stack/M5Unit-foo ; Unit to be used
```

## 使い方

各ユニットのレポジトリの例も参照のこと。

### Unit コンポーネントを UnitUnified とともに使用する (標準的な使用法)

```cpp
// 他のユニットを使用する場合、インクルードファイル (*1)、インスタンス (*2)、値の取得 (*3) を変更する
#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedENV.h>  // *1 使用するユニットのヘッダ

m5::unit::UnitUnified Units;
m5::unit::UnitCO2 unit;  // *2 使用するユニットのインスタンス

void setup() {
    M5.begin();

    auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
    auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
    M5_LOGI("getPin: SDA:%u SCL:%u", pin_num_sda, pin_num_scl);
    Wire.begin(pin_num_sda, pin_num_scl, 400 * 1000U);

    M5.Display.clear(TFT_DARKGREEN);
    if (!Units.add(unit, Wire)  // unit を UnitUnified マネージャへ追加
        || !Units.begin()) {    // ユニットの始動
        M5_LOGE("Failed to add/begin");
        M5.Display.clear(TFT_RED);
    }
}

void loop() {
    M5.begin();
    Units.update();
    if (unit.updated()) {
        // *3 ユニット固有の計測値の取得
        M5_LOGI("CO2:%u Temp:%f Hum:%f", unit.co2(), unit.temperature(), unit.humidity());
    }
}
```

- 標準外の使い方
  - [自分でユニットの更新を行う例](examples/Basic/SelfUpdate)
  - [UnitUnified マネージャを使用せず、コンポーネントのみでの例](examples/Basic/ComponentOnly)


## サポートされているもの
### サポートされるフレームワーク
- Arduino

ESP-IDF は将来対応予定です。

### サポートされる通信
- TwoWire による I2C 通信

GPIO、UART は将来対応予定です。


### サポートされる Core デバイス (動作確認済)
- M5Stack Core (BASIC / GRAY / FIRE)
- M5Stack Core2
- M5Stick CPlus
- M5Paper
- M5Stack CoreS3 CoreS3SE
- M5ATOMS3
- M5STAMPS3
- M5Dial
- M5Capsule
- M5NanoC6

他のデバイスは動作確認されたのち追加されます。

### サポートされるユニット

#### 動作確認済
- [UnitCO2](https://github.com/m5stack/M5Unit-ENV)
- [UnitENVIII](https://github.com/m5stack/M5Unit-ENV)
- [UnitAmeter](https://github.com/m5stack/M5Unit-METER)
- [UnitVmeter](https://github.com/m5stack/M5Unit-METER)
- [UnitPaHub2](https://github.com/m5stack/M5Unit-HUB)
- [UnitGESTURE](https://github.com/m5stack/M5Unit-GESTURE)
- [UnitHEART](https://github.com/m5stack/M5Unit-HEART)
- [UnitKmeterISO](https://github.com/m5stack/M5Unit-KMeterISO)
- [UnitTVOC](https://github.com/m5stack/M5Unit-TVOC)

#### 問題があり修正中
- [UnitENVPro]((https://github.com/m5stack/M5Unit-ENV))

他のユニットは開発、確認後追加されます。

