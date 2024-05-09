
#include <gtest/gtest.h>
#include <M5UnitComponent.hpp>
#include <M5UnitUnified.hpp>
#include "unit_dummy.hpp"

/*
unit_list.txt
UnitFoo
UnitBar
.
.
.
みたいなリストから *1, *2 を生成して cat で 繋げて
このファイルを作るみたいな自動化を検討
 */

//  *1 includes


//

namespace {
// RTTI を使わずに 型の名前を簡易的に出す
//  マングルされているけどないよりはマシ!
template <typename U>
const char* TypeName() {
    return __PRETTY_FUNCTION__;
}
}  // namespace

namespace {

std::vector<m5::unit::types::uid_t> vec;  // For check unique
}  // namespace


TEST(Component, basic)
{
    UnitDummy u;

    EXPECT_FALSE(u.isRegistered());
    

}

TEST(Component, child) {
    UnitDummy u0, u1,u2, u3;
    
    // 
    EXPECT_FALSE(u0.hasParent());
    EXPECT_FALSE(u0.hasSiblings());
    EXPECT_FALSE(u0.hasChildren());
    
    EXPECT_EQ(0U, u0.childrenSize());
    EXPECT_FALSE(u0.add(u1, 0));

    // add 1
    auto cfg = u0.config();
    cfg.max_children = 1;
    u0.config(cfg);
    EXPECT_LT(u1.channel(), 0);
    EXPECT_TRUE(u0.add(u1, 0));
    EXPECT_FALSE(u0.add(u1, 1));
    EXPECT_FALSE(u0.add(u2, 1));
    
    EXPECT_FALSE(u0.hasParent());
    EXPECT_FALSE(u0.hasSiblings());
    EXPECT_TRUE(u0.hasChildren());
    EXPECT_EQ(1U, u0.childrenSize());

    EXPECT_EQ(0, u1.channel());
    EXPECT_TRUE(u1.hasParent());
    EXPECT_FALSE(u1.hasSiblings());
    EXPECT_FALSE(u1.hasChildren());
    
    // add 2
    cfg = u0.config();
    cfg.max_children = 2;
    u0.config(cfg);

    EXPECT_LT(u2.channel(), 0);
    EXPECT_FALSE(u0.add(u2, 0)); // same channel (failed)
    EXPECT_LT(u2.channel(), 0);
    EXPECT_TRUE(u0.add(u2, 3));
    
    EXPECT_FALSE(u0.hasParent());
    EXPECT_FALSE(u0.hasSiblings());
    EXPECT_TRUE(u0.hasChildren());
    EXPECT_EQ(2U, u0.childrenSize());

    EXPECT_TRUE(u1.hasParent());
    EXPECT_TRUE(u1.hasSiblings());
    EXPECT_FALSE(u1.hasChildren());

    EXPECT_TRUE(u2.hasParent());
    EXPECT_TRUE(u2.hasSiblings());
    EXPECT_FALSE(u2.hasChildren());
    EXPECT_EQ(3, u2.channel());

    EXPECT_LT(u3.channel(), 0);
    EXPECT_FALSE(u0.add(u3, 2)); // max = 2 (failed)
    EXPECT_LT(u3.channel(), 0);
    
    // iteration
    UnitDummy* ptr[] = { &u1, &u2 };
    size_t i = 0;
    for(auto it = u0.childBegin(); it != u0.childEnd(); ++it)
    {
        EXPECT_EQ(&(*it), ptr[i++]);
        //        printf("[%u]:%s\n", it->port(), (*it).deviceName());
    }
    EXPECT_EQ(i, u0.childrenSize());

    // -----------------
    if(0)
    {
        UnitDummy p1,p2;
        UnitDummy c0,c1,c2;
        m5::unit::UnitUnified Units;
        m5::unit::Port PortA, PortB;

        auto cfg = p1.config();
        cfg.max_children = 2;
        p1.config(cfg);
        p2.config(cfg);
        
        EXPECT_FALSE(c0.isRegistered());
        EXPECT_FALSE(c1.isRegistered());
        EXPECT_FALSE(c2.isRegistered());

        // 先に Hub に子供繋いだ場合
        p1.add(c1, 1);
        Units.add(p1, PortA);
        EXPECT_TRUE(p1.isRegistered());
        EXPECT_TRUE(c0.isRegistered());
        EXPECT_FALSE(c1.isRegistered());
        EXPECT_FALSE(c2.isRegistered());
        
        // Hub add した後に子供繋いだら???
        // 自動的に Units 管理になって欲しいわよね...
        Units.add(p2, PortB);
        EXPECT_TRUE(p2.isRegistered());
        p2.add(c1, 0);
        EXPECT_TRUE(c1.isRegistered());
        EXPECT_FALSE(c2.isRegistered());
    }
}

namespace {

// 各コンポーネントのテスト
// コンポーネントの抱える実実装部分は各ライブラリ側でテストをする
template <class U>
void each_unit_test() {
    //    SCOPED_TRACE(::testing::Message() << "Unit:" << typeName<U>()); //
    //    失敗時どれが対象かわかるように名前を出す
    SCOPED_TRACE(::testing::Message() << '[' << U::name << ']');

    U* u = new U();

    // class 経由でもインスタンス経由でも同値であること
    EXPECT_EQ(U::uid, u->identifier());
    EXPECT_EQ(U::attr, u->attribute());
    EXPECT_STREQ(U::name, u->deviceName());

    // 同値のUIDが既にあるか?
    auto it = std::find(vec.begin(), vec.end(), U::uid);
    EXPECT_EQ(it, vec.end()) << "Duplicated unit identifier:" << U::uid;

    // その他のテストを...
    // 例えば属性から、その為に必要な機能を満たしているかテストとか

    //
    vec.push_back(U::uid);
    delete u;
}
//
}  // namespace

TEST(Component, each) {
    // *2 each test
    each_unit_test<m5::unit::UnitBM8563>();
    // .
    // .
    // .

    vec.clear();
}
