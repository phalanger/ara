
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test_suite.hpp>
#include "ara/threadext.h"

#include <sstream>

BOOST_AUTO_TEST_SUITE(threadext)

class TestAutoDel : public ara::auto_del_base
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

class TLocalData
{
public:
	TLocalData() {}

	int	get() const { return a_; }
	void set(int a) { a_ = a; }
protected:
	int a_ = 0;
};

struct Test2 {};

BOOST_AUTO_TEST_CASE(thread_local_storage_ptr)
{
	ara::thread_local_data_ptr<TLocalData>		a1;
	if (a1.empty()) {
		a1.reset(new TLocalData());
	}

	BOOST_REQUIRE_EQUAL(a1->get(), static_cast<int>(0));
	a1->set(2);
	BOOST_REQUIRE_EQUAL(a1->get(), static_cast<int>(2));
}

BOOST_AUTO_TEST_CASE(thread_local_storage)
{
	TLocalData & a1 = ara::thread_local_data<TLocalData>();

	BOOST_REQUIRE_EQUAL(a1.get(), static_cast<int>(2));
	a1.set(3);
	BOOST_REQUIRE_EQUAL(a1.get(), static_cast<int>(3));

	TLocalData & a2 = ara::thread_local_data<TLocalData, Test2>();
	BOOST_REQUIRE_EQUAL(a2.get(), static_cast<int>(0));
	a2.set(4);
	BOOST_REQUIRE_EQUAL(a2.get(), static_cast<int>(4));
	BOOST_REQUIRE_EQUAL(a1.get(), static_cast<int>(3));

	TLocalData & a3 = ara::thread_local_data<TLocalData>();
	BOOST_REQUIRE_EQUAL(a3.get(), static_cast<int>(3));

	int nValInThread = -1;
	std::thread		t([&nValInThread]() {
		TLocalData & a1 = ara::thread_local_data<TLocalData>();
		nValInThread = a1.get();
		ara::thread_context::destroy_context();
	});
	t.join();
	BOOST_REQUIRE_EQUAL(nValInThread, static_cast<int>(0));

	auto t2 = ara::make_thread([&nValInThread]() {
		TLocalData & a1 = ara::thread_local_data<TLocalData>();
		a1.set(1000);
		nValInThread = a1.get();
		ara::thread_context::destroy_context();
	});
	auto t3 = ara::make_thread([]() {
		TLocalData & a1 = ara::thread_local_data<TLocalData>();
		a1.set(2000);
	});
	t2.join();
	t3.join();
	BOOST_REQUIRE_EQUAL(nValInThread, static_cast<int>(1000));
}

static size_t	find_call_count(const std::string & info) {
	size_t nCount = 1;
	std::string::size_type p = 0;
	while ((p = info.find(" <- ", p)) != std::string::npos) {
		++nCount;
		p = p + 2;
	}
	return nCount;
}

BOOST_AUTO_TEST_CASE(thread_call)
{
	BEGIN_CALL_AUTOINFO;

	std::stringstream o;
	ara::thread_context::dump_all_thread_state(o);
	std::string str = o.str();

	BOOST_REQUIRE_NE(str.find("threadext::thread_call::test_method"), std::string::npos);

	{
		BEGIN_CALL("Test in sub function");
		std::stringstream o2;
		ara::thread_context::dump_all_thread_state(o2);
		std::string str2 = o2.str();

		BOOST_REQUIRE_EQUAL(find_call_count(str2), 2);
		BOOST_REQUIRE_NE(str2.find("Test in sub function"), std::string::npos);
		BOOST_REQUIRE_NE(str2.find("threadext::thread_call::test_method"), std::string::npos);

		{
			BEGIN_CALL("Test in sub function2", clock());

			std::stringstream o4;
			ara::thread_context::dump_all_thread_state(o4);
			std::string str4 = o4.str();

			BOOST_REQUIRE_EQUAL(find_call_count(str4), 3);
		}
	}

	std::stringstream o3;
	ara::thread_context::dump_all_thread_state(o3);
	std::string str3 = o3.str();
	BOOST_REQUIRE_EQUAL(find_call_count(str3), 1);
	BOOST_REQUIRE_EQUAL(str3.find("Test in sub function"), std::string::npos);
	BOOST_REQUIRE_NE(str3.find("threadext::thread_call::test_method"), std::string::npos);
}

BOOST_AUTO_TEST_SUITE_END()
