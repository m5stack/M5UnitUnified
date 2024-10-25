
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
アルファ版ですが Arduino/PlatformIO のライブラリマネージャーに登録されています

### ArduinoIDE
1. ライブリマネージャから使用したいユニットのライブラリ (M5Unit-Foo) を選択してください

依存する M5UnitUnfied 関連のライブラリは自動で DL されます

### PlatformIO
1. platformio.ini の lib\_deps に記述してください
```ini
lib_deps=m5stack/M5Unit-foo ;使用したいユニットのライブラリ
```

依存する M5UnitUnfied 関連のライブラリは自動で DL されます

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


### サポートされるデバイス (動作確認済)
こちらを参照 https://docs.m5stack.com/en/products

- CORE
    - [BASIC](https://docs.m5stack.com/en/core/basic) 
        - [2.6](https://docs.m5stack.com/en/core/BASIC%20v2.6) 
        - [GRAY](https://docs.m5stack.com/en/core/gray)
    - [Core2](https://docs.m5stack.com/en/core/core2) 
	    - [For AWS](https://docs.m5stack.com/en/core/core2_for_aws) 
		- [v.1.1](https://docs.m5stack.com/en/core/Core2%20v1.1)
    - [CoreS3](https://docs.m5stack.com/en/core/CoreS3) 
	    - [CoreS3 SE](https://docs.m5stack.com/en/core/M5CoreS3%20SE)
    - [FIRE](https://docs.m5stack.com/en/core/fire) 
	    - [v2.6](https://docs.m5stack.com/en/core/fire_v2.6)
    - [M5Dial](https://docs.m5stack.com/en/core/M5Dial)
    - [M5Capsule](https://docs.m5stack.com/en/core/M5Capsule)
    - [M5DinMeter](https://docs.m5stack.com/en/core/M5DinMeter)
    - [M5NanoC6](https://docs.m5stack.com/en/core/M5NanoC6)
- STICK	
    - [M5Stick CPlus](https://docs.m5stack.com/en/core/m5stickc_plus)
    - [M5Stick CPlus2](https://docs.m5stack.com/en/core/M5StickC%20PLUS2)
- ATOM	
    - [ATOM Matrix](https://docs.m5stack.com/en/core/ATOM%20Matrix)
    - [AtomS3](https://docs.m5stack.com/en/core/AtomS3)
    - [AtomS3R](https://docs.m5stack.com/en/core/AtomS3R)
    - [AtomS3R Cam](https://docs.m5stack.com/en/core/AtomS3R%20Cam)
- STAMP
    - [StampS3](https://docs.m5stack.com/en/core/StampS3)
- E-PAPER
    - [CORE.INK](https://docs.m5stack.com/en/core/coreink)
    - [M5PAPER](https://docs.m5stack.com/en/core/m5paper)

他のデバイスは動作確認されたのち追加されます。

### サポートされるユニット

#### 動作確認済
- [M5Unit-ENV](https://github.com/m5stack/M5Unit-ENV)
  - UnitCO2
  - UnitENVIII
  - UnitTVOC
  - UnitENVPro
- [M5Unit-METER](https://github.com/m5stack/M5Unit-METER)
  - UnitAmeter
  - UnitVmeter
  - UnitKmeterISO
- [M5Unit-HUB](https://github.com/m5stack/M5Unit-HUB)
  - UnitPaHub2
- [M5Unit-GESTURE](https://github.com/m5stack/M5Unit-GESTURE)
  - UnitGESTURE
- [M5Unit-HEART](https://github.com/m5stack/M5Unit-HEART)
  - UnitHEART
- [M5Unit-TOF](https://github.com/m5stack/M5Unit-TOF)
  - UnitToF
  - UnitToF4M
- [M5Unit-WEIGHT](https://github.com/m5stack/M5Unit-WEIGHT)
  - UnitWeightI2C

他のユニットは開発、確認後追加されます。


## Examples
各ユニットのサンプルについては、各ユニットのリポジトリを参照してください。  
[このリポジトリにあるサンプル](examples/Basic)は M5UnitUnified 全般のものです。


## Doxygen ドキュメント

[GitHub Pages](https://m5stack.github.io/M5UnitUnified/)

あなたのローカルマシンでドキュメントを生成したい場合は、以下のコマンドを実行してください。
```
bash docs/doxy.sh
```
docs/html の下に出力されます。  
Git コミットのハッシュを html に出力したい場合は、 git クローンしたフォルダに対して実行してください。

### 必要な物
- [Doxyegn](https://www.doxygen.nl/)
- [pcregrep](https://formulae.brew.sh/formula/pcre2)
- [Git](https://git-scm.com/)

