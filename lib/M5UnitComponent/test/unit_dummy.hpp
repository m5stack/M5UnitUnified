
// 挙動テスト用のダミーユニットクラス
#ifndef __TEST_UNIT_DUMMY_HPP__
#define __TEST_UNIT_DUMMY_HPP__

#include <M5UnitComponent.hpp>

class UnitDummy : public m5::unit::Component
{
  public:
    static const m5::unit::types::uid_t uid;
    static const m5::unit::types::attr_t attr;
    static const char name[];
    UnitDummy() : Component() {}
    virtual ~UnitDummy(){}

    virtual bool begin() override { return true; }
    virtual void update() override {}

   protected:
    virtual const char* unit_device_name() const override {
        return name;
    }
    virtual m5::unit::types::uid_t unit_identifier() const override {
        return uid;
    }
    virtual m5::unit::types::attr_t unit_attribute() const override {
        return attr;
    }
};



#endif

