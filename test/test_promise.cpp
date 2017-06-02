
#include "3rd/Catch/single_include/catch.hpp"

#include "ara/promise.h"
#include "ara/threadext.h"
#include "ara/event.h"

#include <iostream>

ara::async_result<int, bool, std::string>	myfunc(int a)
{
	ara::async_result<int, bool, std::string>	result;

	ara::make_thread( [result, a]() {
	
		int b = a + 123;
		bool c = ((b % 2) == 0);

		result.set( b, c, "Hello" );
	}).detach();

	return result;
}


TEST_CASE("promise", "[base]") {

	SECTION("base") {

		auto result = myfunc(10);

		auto res2 = result.then([](int a, bool b, const std::string & s)-> ara::async_result<size_t> {
			ara::async_result<size_t> res;
			return res.set( (a - 10) * (b ? 1 : 2) + s.length());
		}).then([](size_t a) -> int {
			return static_cast<int>(a * 2);
		});

		//  myfunc : a = 10
		//		b = 133, c = false, s = "Hello"
		//	then1 : a = 133, b = false, s = "Hello"
		//		res = (133 - 10) * 2 + 5 = 123 *2 + 5 = 251
		//	then2 : a = 251
		//		res = 251 * 2
		//	so result = 502

		int a = res2.get();
		REQUIRE(a == 502);
	}

	SECTION("exception") {

		ara::async_result<int>		result;
		ara::make_thread([result]() {
			result.throw_exception(std::bad_exception());
		}).detach();

		REQUIRE( result.wait_from_now(100_sec) );
		REQUIRE(result.ready());
		REQUIRE(result.has_exception());

		REQUIRE_THROWS_AS(result.get<0>(), std::bad_exception);
	}

	SECTION("exception2") {

		ara::async_result<int>		result;
		ara::make_thread([result]() {
			result.set(100);
		}).detach();

		auto res2 = result.then([](int a)->int {
			throw std::bad_exception();
		});

		REQUIRE_THROWS_AS(res2.get<0>(), std::bad_exception);
	}

	SECTION("exception3") {

		ara::async_result<int>		result;
		ara::make_thread([result]() {
			result.throw_exception(std::bad_exception());
		}).detach();

		bool boHasException = false;
		auto res2 = result.then([](int a)->int {
			throw std::bad_alloc();
		}).on_exception([&boHasException](std::exception_ptr p) {
			try {
				if (p)
					std::rethrow_exception(p);
			} catch (std::bad_alloc &) {
				boHasException = true;
			} catch (std::bad_exception &) {
				boHasException = false;
			}
		});

		res2.wait();
		REQUIRE(boHasException);
	}

	SECTION("exception4") {

		ara::async_result<int>		result;
		ara::make_thread([result]() {
			result.throw_exception(std::bad_exception());
		}).detach();

		bool boHasException = false;
		auto res2 = result.then([](int a)->ara::async_result<int> {
			ara::async_result<int> res;
			res.throw_exception( std::bad_alloc() );
			return res;
		}).on_exception([&boHasException](std::exception_ptr p) {
			try {
				if (p)
					std::rethrow_exception(p);
			} catch (std::bad_alloc &) {
				boHasException = true;
			} catch (std::bad_exception &) {
				boHasException = false;
			}
		});

		res2.wait();
		REQUIRE(boHasException);
	}

	SECTION("async_exec") {

		auto res = ara::async_exec([]() -> int {
			return 100;
		});

		res.wait();
		REQUIRE(res.get() == 100);
	}

	SECTION("async_exec2") {

		auto res = ara::async_exec([]() -> int {
			int a = 10;
			throw std::bad_exception();
		});

		res.wait();
		REQUIRE_THROWS_AS(res.get<0>(), std::bad_exception);
	}

}