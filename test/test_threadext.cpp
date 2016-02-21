
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test_suite.hpp>
#include "ara/threadext.h"

BOOST_AUTO_TEST_SUITE(threadext)

class TestAutoDel : public ara::internal::auto_del_base
{
public:
	TestAutoDel() {}
protected:
	int	a_ = 100;
};

BOOST_AUTO_TEST_CASE(thread_context)
{
	auto & context = ara::thread_context::get();
	auto n = context.next_sn();
	BOOST_REQUIRE_EQUAL(n, static_cast<uint64_t>(0));
	n = context.next_sn();
	BOOST_REQUIRE_EQUAL(n, static_cast<uint64_t>(1));

	context.delete_on_thread_exit(new TestAutoDel);
}

BOOST_AUTO_TEST_SUITE_END()
