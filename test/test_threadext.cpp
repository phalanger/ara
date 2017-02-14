
#include "3rd/Catch/single_include/catch.hpp"

#include "ara/threadext.h"
#include "ara/event.h"

#include <sstream>

class TestAutoDel : public ara::auto_del_base
{
public:
	TestAutoDel() { ++c_; }
	~TestAutoDel() { --c_; }

	static int count() { return c_; }
protected:
	static int	c_;
};
int TestAutoDel::c_ = 0;

class TestAutoDel2
{
public:
	TestAutoDel2() { ++c_; }
	~TestAutoDel2() { --c_; }

	static int count() { return c_; }
protected:
	static int	c_;
};
int TestAutoDel2::c_ = 0;

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

class UserDefinedData
{
public:
	int		nMaxCallTime = 0;
	int		foo_count = 0;
	void *	extdata = nullptr;
	std::string strInfo;
};

static void foo() {
	BEGIN_CALL_AUTOINFO;
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

static void test_call_stack()
{
	BEGIN_CALL_AUTOINFO;

	std::stringstream o;
	ara::thread_context::dump_all_thread_state(o);
	std::string str = o.str();

	REQUIRE(str.find("test_call_stack") != std::string::npos);

	{
		const std::string strCallInfo = "Test in sub function";
		BEGIN_CALL(strCallInfo);

		std::stringstream o2;
		ara::thread_context::dump_all_thread_state(o2);
		std::string str2 = o2.str();

		REQUIRE(find_call_count(str2) == 2);
		REQUIRE(str2.find("Test in sub function") != std::string::npos);
		REQUIRE(str2.find("test_call_stack") != std::string::npos);

		{
			BEGIN_CALL(ara::str_printf<std::string>("Function 2: in time: %v", ara::date_time::get_current().format("%H%M%S"))
					, clock());

			std::stringstream o4;
			ara::thread_context::dump_all_thread_state(o4);
			std::string str4 = o4.str();

			REQUIRE(find_call_count(str4) == 3);

			int idx = 0;
			clock_t	tBegin = 0;
			int tMax = 0;
			ara::thread_context::get().navigate_callstack([&idx, &tBegin, &tMax](const ara::internal::thread_call & c) {
				if (idx) {
					int delta = c.get_start_clock() - tBegin;
					if (delta > tMax)
						tMax = delta;
				}
				tBegin = c.get_start_clock();
				++idx;
			});

			INFO("max call time : " << tMax);
			REQUIRE(idx == 3);
		}
	}

	std::stringstream o3;
	ara::thread_context::dump_all_thread_state(o3);
	std::string str3 = o3.str();
	REQUIRE(find_call_count(str3) == 1);
	REQUIRE(str3.find("Test in sub function") == std::string::npos);
	REQUIRE(str3.find("test_call_stack") != std::string::npos);
}

TEST_CASE("threadext", "[base]") {

	SECTION("thread context") {
		auto & context = ara::thread_context::get();
		auto n = context.next_sn();
		REQUIRE(n == static_cast<uint64_t>(0));
		n = context.next_sn();
		REQUIRE(n == static_cast<uint64_t>(1));
	}

	SECTION("auto delete while thread exit") {

		REQUIRE(TestAutoDel::count() == 0);
		REQUIRE(TestAutoDel2::count() == 0);

		int n = 0;

		auto p = ara::make_thread([&n]() {
			auto & context = ara::thread_context::get();
			context.delete_on_thread_exit(new TestAutoDel);
			context.delete_on_thread_exit(new TestAutoDel2);

			context.run_on_thread_exit([&n]() {
				n = 123;
			});

			REQUIRE(n == 0);
			REQUIRE(TestAutoDel::count() == 1);
			REQUIRE(TestAutoDel2::count() == 1);
		});

		p.join();

		REQUIRE(n == 123);
		REQUIRE(TestAutoDel::count() == 0);
		REQUIRE(TestAutoDel2::count() == 0);
	}

	SECTION("thread local storage pointer") {
		ara::thread_local_data_ptr<TLocalData>		a1;
		if (a1.empty()) {
			auto p = new TLocalData();
			a1.reset(p);
			REQUIRE(a1 == p);
		}

		REQUIRE(a1->get() == static_cast<int>(0));
		a1->set(2);
		REQUIRE(a1->get() == static_cast<int>(2));

		auto		th2 = ara::make_thread([]() {
			ara::thread_local_data_ptr<TLocalData>		a1;
			REQUIRE(a1.empty());
		});

		th2.join();

		{
			ara::thread_local_data_ptr<TLocalData>		a1;
			REQUIRE(!a1.empty());
			REQUIRE(a1->get() == static_cast<int>(2));
		}
	}

	SECTION("thread local storage") {

		TLocalData & a1 = ara::thread_local_data<TLocalData>();

		REQUIRE(a1.get() == static_cast<int>(2));
		a1.set(3);
		REQUIRE(a1.get() == static_cast<int>(3));

		TLocalData & a2 = ara::thread_local_data<TLocalData, Test2>();
		REQUIRE(a2.get() == static_cast<int>(0));
		a2.set(4);
		REQUIRE(a2.get() == static_cast<int>(4));
		REQUIRE(a1.get() == static_cast<int>(3));

		TLocalData & a3 = ara::thread_local_data<TLocalData>();
		REQUIRE(a3.get() == static_cast<int>(3));

		int nValInThread = -1;
		std::thread		t([&nValInThread]() {
			TLocalData & a1 = ara::thread_local_data<TLocalData>();
			nValInThread = a1.get();
			ara::thread_context::destroy_context();
		});
		t.join();
		REQUIRE(nValInThread == static_cast<int>(0));

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
		REQUIRE(nValInThread == static_cast<int>(1000));
	}

	SECTION("thread call stack") {
		test_call_stack();
	}

	SECTION("thread call stack with user defined data") {

		UserDefinedData	data;
		ara::thread_context::get().register_after_call([&data](ara::internal::thread_call & c) {
			if (c.get_caller()) {
				int interval = c.get_start_clock() - c.get_caller()->get_start_clock();
				if (interval > data.nMaxCallTime) {
					data.extdata = c.get_ext_data();
					data.nMaxCallTime = interval;
					data.strInfo = c.get_call_info();
				}
			}
			ara::ref_string	callInfo(c.get_call_info());
			if (callInfo.find("foo") != ara::ref_string::npos)
				++data.foo_count;
		});
		ara::defer		_guard([]() {
			ara::thread_context::get().unregister_after_call();
		});

		[]() {
			clock_t	tCur = 1;
			BEGIN_CALL("Func1", tCur);
			[]() {
				clock_t	tCur = 2;
				BEGIN_CALL("Func2", tCur);
				[]() {
					clock_t	tCur = 3;
					BEGIN_CALL("Func3", tCur);
					foo();
					foo();
					foo();
					ara::sleep(std::chrono::milliseconds(100));
					foo();
				}();

			}();
		}();

		REQUIRE(data.foo_count == 4);
		REQUIRE(data.nMaxCallTime > 0);
	}

	SECTION("thread navigate all") {

		BEGIN_CALL_AUTOINFO;
		auto ToStop = std::make_shared<ara::event<int>>(0);
		auto StartWork = std::make_shared<ara::event<int>>(0);

		auto make_func = [ToStop, StartWork](int i) -> std::function<void()> {
			return [ToStop, StartWork, i]() {
				BEGIN_CALL(ara::str_printf<std::string>("Run in thread[%v]", i));
				StartWork->inc_signal_one(1);
				foo();
				ToStop->wait(1);
			};
		};

		int nTestCount = 3;
		for (int i = 0; i < nTestCount; ++i)
			ara::make_thread(make_func(i)).detach();

		StartWork->wait(nTestCount);
		int nThreadCount = 0;
		ara::thread_context::navigate_all_thread_state([&nThreadCount](ara::internal::thread_state & s) {
			++nThreadCount;
			std::stringstream	sOut;
			s.dump_callstack(sOut);
			REQUIRE(sOut.str().size() > 0);
		});
		ToStop->signal_all(1);

		REQUIRE(nThreadCount == nTestCount + 1);
	}

}
