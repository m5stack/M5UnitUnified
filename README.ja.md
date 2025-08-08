
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

各ユニットのリポジトリの例も参照のこと。

### Unit コンポーネントを UnitUnified とともに使用する (標準的な使用法)

#### Wire 使用のユニット

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
    M5.update();
    Units.update();
    if (unit.updated()) {
        // *3 ユニット固有の計測値の取得
        M5.Log.printf("CO2:%u Temp:%f Hum:%f\n", unit.co2(), unit.temperature(), unit.humidity());
    }
}
```

#### GPIO 使用のユニット

```cpp
// 他のユニットを使用する場合、インクルードファイル (*1)、インスタンス (*2)、値の取得 (*3) を変更する
#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedTUBE.h> // *1 使用するユニットのヘッダ

m5::unit::UnitUnified Units;
m5::unit::UnitTubePressure unit; // *2 使用するユニットのインスタンス

void setup()
{
    M5.begin();

    // PortB if available, PortA if not
    auto pin_num_gpio_in  = M5.getPin(m5::pin_name_t::port_b_in);
    auto pin_num_gpio_out = M5.getPin(m5::pin_name_t::port_b_out);
    if (pin_num_gpio_in < 0 || pin_num_gpio_out < 0) {
        M5_LOGW("PortB is not available");
        Wire.end();
        pin_num_gpio_in  = M5.getPin(m5::pin_name_t::port_a_pin1);
        pin_num_gpio_out = M5.getPin(m5::pin_name_t::port_a_pin2);
    }

    if (!Units.add(unit, pin_num_gpio_in, pin_num_gpio_out) // Add unit to UnitUnified manager
        || !Units.begin()) { // Begin each unit
        M5_LOGE("Failed to add/begin");
    }
}

void loop()
{
    M5.update();
    Units.update();
    if (unit.updated()) {
        // *3 ユニット固有の計測値の取得
        M5.Log.printf("Pressure:%.2f\n", unit.pressure());
    }
}
```

#### UART(Serial) 使用のユニット

```cpp
// 他のユニットを使用する場合、インクルードファイル (*1)、インスタンス (*2)、API呼び出し (*3) を変更する
#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedFINGER.h> // *1 使用するユニットのヘッダ

m5::unit::UnitUnified Units;
m5::unit::UnitFinger unit; // *2 使用するユニットのインスタンス

void setup()
{
    M5.begin();

    // PortC if available, PortA if not
    auto pin_num_in  = M5.getPin(m5::pin_name_t::port_c_rxd);
    auto pin_num_out = M5.getPin(m5::pin_name_t::port_c_txd);
    if (pin_num_in < 0 || pin_num_out < 0) {
        M5_LOGW("PortC is not available");
        Wire.end();
        pin_num_in  = M5.getPin(m5::pin_name_t::port_a_pin1);
        pin_num_out = M5.getPin(m5::pin_name_t::port_a_pin2);
    }

#if SOC_UART_NUM > 2
    auto& s = Serial2;
#elif SOC_UART_NUM > 1
    auto& s = Serial1;
#else
#error "Not enough Serial"
#endif
    s.end();
    // 備考: ユニットによって初期化パラメータは異なる
    s.begin(19200, SERIAL_8N1, pin_num_in, pin_num_out);

    if (!Units.add(unit, s) // Add unit to UnitUnified manager
        || !Units.begin()) { // Begin each unit
        M5_LOGE("Failed to begin");
    }
}

void loop() {
    M5.update();
    Units.update();
    // *3 任意の API 呼び出し...
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
- GPIO (現在は各ユニットに必要な機能のみ搭載）

UART は将来対応予定です。

### サポートされるデバイス,ユニット
[Wiki](https://github.com/m5stack/M5UnitUnified/wiki/)を参照


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

