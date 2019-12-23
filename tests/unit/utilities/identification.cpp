
#include <stdint.h>
#include <string.h>

#define BOOST_TEST_MODULE Util

#include <unit_test_common.h>

#include "cool/ng/bases.h"

using namespace cool::ng::util;


template <detail::id_policy Policy = detail::id_policy::duplicate_on_copy>
class test_class : public identified<Policy>
{

};

BOOST_AUTO_TEST_SUITE(identified_)

COOL_AUTO_TEST_CASE(T001,
  * utf::description("identified<duplicate_on_copy>"))
{
  test_class<> io;
  {
    test_class<> aux(io);
    BOOST_CHECK_EQUAL(io.id(), aux.id());
  }
  {
    test_class<> aux;
    aux =  io;
    BOOST_CHECK_EQUAL(io.id(), aux.id());
  }
  {
    test_class<> ref = io;

    BOOST_CHECK_EQUAL(io.id(), ref.id());
    BOOST_REQUIRE_NE(0, ref.id());

    test_class<> aux(std::move(ref));
    BOOST_CHECK_EQUAL(io.id(), aux.id());
    BOOST_CHECK_EQUAL(0, ref.id());
  }
  {
    test_class<> ref = io;

    BOOST_CHECK_EQUAL(io.id(), ref.id());
    BOOST_REQUIRE_NE(0, ref.id());

    test_class<> aux;
    aux = std::move(ref);
    BOOST_CHECK_EQUAL(io.id(), aux.id());
    BOOST_CHECK_EQUAL(0, ref.id());
  }
}

COOL_AUTO_TEST_CASE(T002,
  * utf::description("identified<unique_on_copy>"))
{
  test_class<detail::id_policy::unique_on_copy> io;
  {
    test_class<detail::id_policy::unique_on_copy> aux(io);
    BOOST_CHECK_NE(io.id(), aux.id());
  }
  {
    test_class<detail::id_policy::unique_on_copy> aux;
    auto store = aux.id();
    aux =  io;
    BOOST_CHECK_NE(io.id(), aux.id());
    BOOST_CHECK_EQUAL(store, aux.id());
  }
  {
    test_class<detail::id_policy::unique_on_copy> ref;

    BOOST_CHECK_NE(io.id(), ref.id());
    BOOST_REQUIRE_NE(0, ref.id());
    auto store = ref.id();

    test_class<detail::id_policy::unique_on_copy> aux(std::move(ref));
    BOOST_CHECK_EQUAL(store, aux.id());
    BOOST_CHECK_EQUAL(0, ref.id());
  }
  {
    test_class<detail::id_policy::unique_on_copy> ref = io;

    BOOST_CHECK_NE(io.id(), ref.id());
    BOOST_REQUIRE_NE(0, ref.id());
    auto store = ref.id();

    test_class<detail::id_policy::unique_on_copy> aux;
    aux = std::move(ref);
    BOOST_CHECK_EQUAL(store, aux.id());
    BOOST_CHECK_EQUAL(0, ref.id());
  }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(named_)

class test_named : public named
{
 public:
  test_named(const std::string& pfx) : named(pfx)
  { /* noop */ }
};

COOL_AUTO_TEST_CASE(T001,
  * utf::description("named operations"))
{

  {
    test_named n("test");
    BOOST_CHECK_EQUAL(n.prefix(), "test");
    test_named aux(n);
    BOOST_CHECK_NE(n.name(), aux.name());
    BOOST_CHECK_EQUAL(n.prefix(), aux.prefix());
  }
  {
    test_named n("test");
    BOOST_CHECK_EQUAL(n.prefix(), "test");
    test_named aux("copy");
    aux = n;
    BOOST_CHECK_NE(n.name(), aux.name());
    BOOST_CHECK_EQUAL(n.prefix(), aux.prefix());
  }
  {
    test_named n("test");
    auto ref = n.name();

    test_named aux(std::move(n));
    BOOST_CHECK_EQUAL(ref, aux.name());
    BOOST_CHECK_EQUAL("moved-0", n.name());
  }
  {
    test_named n("test");
    auto ref = n.name();

    test_named aux("another");
    aux = std::move(n);
    BOOST_CHECK_EQUAL(ref, aux.name());
    BOOST_CHECK_EQUAL("moved-0", n.name());
  }
}

BOOST_AUTO_TEST_SUITE_END()

