#include <ydb-cpp-sdk/util/network/address.h>

#include <src/library/testing/unittest/registar.h>

using namespace NAddr;

Y_UNIT_TEST_SUITE(IRemoteAddr_ToString) {
    Y_UNIT_TEST(Raw) {
        THolder<TOpaqueAddr> opaque(new TOpaqueAddr);
        IRemoteAddr* addr = opaque.Get();

        std::string s = ToString(*addr);
        UNIT_ASSERT_VALUES_EQUAL("(raw all zeros)", s);

        opaque->MutableAddr()->sa_data[10] = 17;

        std::string t = ToString(*addr);

        UNIT_ASSERT_C(t.starts_with("(raw 0 0"), t);
        UNIT_ASSERT_C(t.ends_with(')'), t);
    }

    Y_UNIT_TEST(Ipv6) {
        TNetworkAddress address("::1", 22);
        TNetworkAddress::TIterator it = address.Begin();
        UNIT_ASSERT(it != address.End());
        UNIT_ASSERT(it->ai_family == AF_INET6);
        std::string toString = ToString(static_cast<const IRemoteAddr&>(TAddrInfo(&*it)));
        UNIT_ASSERT_VALUES_EQUAL(std::string("[::1]:22"), toString);
    }

    Y_UNIT_TEST(Loopback) {
        TNetworkAddress localAddress("127.70.0.1", 22);
        UNIT_ASSERT_VALUES_EQUAL(NAddr::IsLoopback(TAddrInfo(&*localAddress.Begin())), true);

        TNetworkAddress localAddress2("127.0.0.1", 22);
        UNIT_ASSERT_VALUES_EQUAL(NAddr::IsLoopback(TAddrInfo(&*localAddress2.Begin())), true);
    }
}
