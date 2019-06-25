#include <doctest/doctest.h>
#include <type_traits>
#include <unistd.h>
#include "LogATE/Net/SocketDescriptorPair.hpp"

using LogATE::Net::SocketDescriptorPair;

namespace
{
TEST_SUITE("Net::SocketDescriptorPair")
{

TEST_CASE("creating pairs")
{
    const SocketDescriptorPair sp1;
    CHECK(sp1.first.opened());
    CHECK(sp1.second.opened());
    CHECK( sp1.first.get() != sp1.second.get() );

    const SocketDescriptorPair sp2;
    CHECK( sp2.first.opened() );
    CHECK( sp2.second.opened() );
    CHECK( sp2.first.get() != sp2.second.get() );

    CHECK( sp1.first.get()  != sp2.first.get() );
    CHECK( sp1.second.get() != sp2.second.get() );
}

TEST_CASE("I/O between sockets")
{
    const SocketDescriptorPair sp;

    CHECK( ::write(sp.first.get(), "#", 1) == 1 );
    char out;
    CHECK( ::read(sp.second.get(), &out, 1) == 1 );
    CHECK(out == '#');
}

TEST_CASE("is non-copyable")
{
    CHECK(not std::is_copy_assignable<SocketDescriptorPair>::value);
    CHECK(not std::is_copy_constructible<SocketDescriptorPair>::value);
}

TEST_CASE("is movable")
{
    CHECK(std::is_move_assignable<SocketDescriptorPair>::value);
    CHECK(std::is_move_constructible<SocketDescriptorPair>::value);
}

}
}
