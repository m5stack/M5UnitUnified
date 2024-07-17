# M5UnitComponent

Work in progress

## Overview

Library for componentising the functions of each device so that they can be handled by M5UnitUnified.  
Works with C++11 or later.

## License

- [M5UnitComponent- MIT](LICENSE)







## 開発用メモ (当リポジトリ公開時は別の md とするか破棄すること!)
### 派生ユニットクラス作成指針 Rules and guidelines for creating UnitComponent-derived classes
- 通信を伴う関数は bool 型として成否を返す
- チップとやりとりして値を得るものは readFoo 定期計測などを内部にストックしたものの取得は名詞(e.g. tempeatuure() )
- read系は、出力先を最初に初期化または不正値にしておくこと  
	- float -> std::numeric_limits<float>::quiet_NaN(); integral -> 0 / -1 etc.
  - ポインタや返り値で値を返すのではなく、参照型引数に対して値を返す
- チップとやりとりして値を設定するものは setFoo
- enum は class emum Foo : type として、曖昧さを回避し、 type でサイズを束縛する
  - enum class は static_assert で想定型と同一であることを確認  
    static_assert(sizeof(Enum::Foo) == sizeof(decltype(Struct::foo), "Invalid");
- Class::config_t は begin 時の挙動を設定する
  - 取得 config_t config() const;
  - 設定 void config(const config_t &cfg);
- begin()
  - 可能であればチップの存在確認
  - チップから必要な情報の取得
  - config_t に基づく初期設定と初期動作の設定
- update()
  - loop() またはタスク内で呼ばれることを想定
  - 毎回チップとの通信を行わない形になるように努める  
    例えば計測間隔が設定されているチップでは、所定の期間を超えた時に初めて通信を行うなど。
- 定期計測可能なものは以下の API を整備する
  - 定期計測中?  bool inPeriodic() const;
  - 計測データが更新された?  bool updated() const;
  - 更新時の時刻 (ms) unsigned long updatedMillis() const;
  - 定期計測開始 bool startPeriodicMeasurement(/\*適宜必要な引数\*/);
  - 定期計測停止 bool stopPeriodicMeasurement();
  - ストックした最新の計測データ取得 e.g. float tempeature() const;
- 能動的単一計測(いわゆるシングルショット系)は以下の API で
  - bool measureSingleshot(/\*任意の引数\*/);  
  シングルショット計測では内部に値を蓄積せず、与えられた参照型引数へ計測値を返す
- UnitTest で可能な限り API についてのテストを行う
- m5::unit 以下に class は記述する
- チップ特有の値や型、構造体
  - m5::unit::chipname 以下に列挙する
- コマンド I/O レジスタ
  - m5::unit::chipname::command 以下に consttexpr として列挙する
- コマンド送信間隔があるものについては、最後の引数で待ち時間を指定できるようにする
  - default 引数として規定値を設定する  
    bool setFoo(const uint32_t duration = 10);
  - ユーザーが 0 指定で待ち無しで関数から返れるようにする
  - 待ちは必要な時のみ(e.g. コマンド成功時) とする(失敗時はできるだけ早く関数から出る)
