#include <catch.hpp>
#include <zmq.hpp>

#include <stdlib.h>

TEST_CASE("ZMQ supports curve")
{
    CHECK(zmq_has("curve"));
}