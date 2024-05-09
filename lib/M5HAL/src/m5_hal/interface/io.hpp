
#ifndef M5_HAL_INTERFACE_IO_HPP_
#define M5_HAL_INTERFACE_IO_HPP_

#include "../error.hpp"
#include "../types.hpp"

#include <M5Utility.hpp>

#include <future>

/*!
  @namespace m5
  @brief Toplevel namespace of M5
 */
namespace m5 {

namespace hal {

namespace interface {

namespace io {

// struct AsyncParam {
// public:
//     // タイムアウト時間を設定する変数 (呼び出し側が設定しておく。0なら処理を終えるかエラーになるまで無限待ち)
//     uint32_t timeout_ms;

//     // read/writeの結果得られた長さを格納する変数
//     size_t length;

//     // 処理の結果を格納する変数 (ASYNC_RUNNINGの場合はまだ処理中)
//     error::error_t error;
// };

struct Input {
public:
    virtual ~Input() = default;

    // @param data 読み込んだデータを格納するバッファ
    // @param len 読み込むデータの長さ
    // @return 読み込んだデータの長さ / エラー
    virtual m5::stl::expected<size_t, m5::hal::error::error_t> read(uint8_t* data, size_t len) = 0;
    // virtual std::future<m5::stl::expected<size_t, m5::hal::error::error_t> > asyncRead(uint8_t* data, size_t len, AsyncParam* async = nullptr) = 0;
    virtual m5::stl::expected<size_t, m5::hal::error::error_t> availableRead(void) const
        { return m5::stl::make_unexpected(error::error_t::NOT_IMPLEMENTED); };
};

struct Output {
public:
    virtual ~Output() = default;

    virtual m5::stl::expected<size_t, m5::hal::error::error_t> write(const uint8_t* data, size_t len) = 0;
    // virtual std::future<m5::stl::expected<size_t, m5::hal::error::error_t> > asyncWrite(const uint8_t* data, size_t len, AsyncParam* async = nullptr) = 0;
    virtual m5::stl::expected<size_t, m5::hal::error::error_t> availableWrite(void) const
        { return m5::stl::make_unexpected(error::error_t::NOT_IMPLEMENTED); };
};

// Push型の入力はデータが一定量たまったらコールバックが呼ばれ、readで取得する
struct PushInput : public Input {
public:
    virtual ~PushInput() = default;
    virtual m5::stl::expected<void, m5::hal::error::error_t> setCallbackRead(void(*cb)(PushInput* bus, size_t available_len, void* cb_obj)) = 0;
};

// Push型の出力は新しいデータが必要になったらコールバックが呼ばれ、writeで書き込む
struct PushOutput : public Output {
public:
    virtual ~PushOutput() = default;
    virtual m5::stl::expected<void, m5::hal::error::error_t> setCallbackWrite(void(*cb)(PushOutput* bus, size_t request_len, void* cb_obj)) = 0;
};

} // namespace io
} // namespace interface
} // namespace hal
} // namespace m5

#endif
