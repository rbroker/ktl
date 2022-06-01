#include "test.h"

#include <string>

bool test_unicode_string()
{
	__try
	{
		// Default & copy constructors
		ktl::unicode_string from_literal{ L"my_string" };
		ktl::unicode_string from_string{ from_literal };
		ktl::unicode_string default_constructed{};

		ASSERT_TRUE(from_literal.size() == 9, "string constructed from literal has unexpected length: %llu", from_literal.size());
		ASSERT_TRUE(from_string.size() == 9, "string constructed from string has unexpected length: %llu", from_string.size());
		ASSERT_TRUE(default_constructed.empty(), "default constructed string was not empty");
		ASSERT_TRUE(default_constructed.size() == 0, "default constructed string had non-zero size: %llu", default_constructed.size());
		ASSERT_FALSE(from_literal.data()->Buffer == from_string.data()->Buffer, "copied string points to same buffer as original!");

		ASSERT_TRUE(from_literal == from_string, "constructed strings did not match");

		// construct, with length
		ktl::unicode_string literal2{ L"my_string", 2 };
		ASSERT_TRUE(literal2 == L"my", "substring constructor didn't initialize to expected value: %wZ", literal2.data());

		// move construct
		ktl::unicode_string moveConstructed{ ktl::move(from_literal) };
		ASSERT_TRUE(moveConstructed == L"my_string", "move constructed string didn't match");
		ASSERT_TRUE(from_literal != L"my_string", "string matched after being move constructed away");

		// move assign
		ktl::unicode_string moveAssigned = ktl::move(from_string);
		ASSERT_TRUE(moveAssigned == L"my_string", "move assigned string didn't match");
		ASSERT_TRUE(from_string != L"my_string", "string matched after being move assigned away");

		// size
		ASSERT_TRUE(literal2.size() == 2, "unexpected string size: %llu", literal2.size());
		ASSERT_TRUE(moveConstructed.size() == 9, "unexpected move constructed size: %llu", moveConstructed.size());
		ASSERT_TRUE(literal2.byte_size() == (literal2.size() * sizeof(wchar_t)), "unexpected byte size: %llu", literal2.byte_size());
		ASSERT_TRUE(moveConstructed.byte_size() == (moveConstructed.size() * sizeof(wchar_t)), "unexpected move constructed byte size: %llu", literal2.byte_size());

		// resize - grow
		ASSERT_TRUE(literal2.resize(4, 'X'), "unexpected failure to resize");
		ASSERT_TRUE(literal2 == L"myXX", "unexpected string fill on resize: %wZ", literal2.data());

		// resize - shrink
		ASSERT_TRUE(literal2.resize(2), "unexpected failure to resize");
		ASSERT_TRUE(literal2 == L"my", "unexpected string value on resize");
		ASSERT_TRUE(literal2.capacity() == 4, "unexpected string capacity after shrinking: %llu", literal2.capacity());
		ASSERT_TRUE(literal2.byte_capacity() == (literal2.capacity() * sizeof(wchar_t)), "unexpected string byte capacity after shrinking");

		// resize - grow into existing capacity
		ASSERT_TRUE(literal2.resize(3, 'Y'), "unexpected failure to resize");
		ASSERT_TRUE(literal2 == L"myY", "unexpected string fill on resize: %wZ", literal2.data());
		ASSERT_TRUE(literal2.capacity() == 4, "unexpected string capacity after growing: %llu", literal2.capacity());

		// substr
		auto s1 = literal2.substr(0, 1);
		ASSERT_TRUE(s1 == L"m", "unexpected substring value: %wZ", s1.data());
		auto s2 = literal2.substr(2);
		ASSERT_TRUE(s2 == L"Y", "unexpected substring value: %wZ", s2.data());
		auto s3 = literal2.substr(1, 1);
		ASSERT_TRUE(s3 == L"y", "unexpected substring value: %wZ", s3.data());

		// append
		auto append = s1.append(L"hello");
		ASSERT_TRUE(append == L"mhello", "unexpected appended string value: %wZ", append.data());
		append += L" world";
		ASSERT_TRUE(append == L"mhello world", "unexpected appended string value: %wZ", append.data());
		append += s2;
		ASSERT_TRUE(append == L"mhello worldY", "unexpected appended string value: %wZ", append.data());

		// addition
		auto s4 = append + s3;
		ASSERT_TRUE(s4 == L"mhello worldYy", "unexpected appended string value: %wZ", s4.data());

		// compare insensitive
		ASSERT_TRUE(s2.compare(s3, true) == 0, "unexpected result for case insensitive comparison");
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_ERROR("[NG]: %#x\n", GetExceptionCode());
		return false;
	}

	LOG_TRACE("[OK] ktl::unicode_string!\n");
	return true;
}

bool test_unicode_string_view()
{
	__try
	{
		constexpr ktl::unicode_string_view compile_time{ L"compile_time" };
		constexpr ktl::unicode_string_view other_compile_time{ L"other_compile_time" };
		constexpr ktl::unicode_string_view compile_time_string_of_string{ compile_time };

		// compare
		ASSERT_TRUE(compile_time != other_compile_time, "compile time strings matched");
		ASSERT_TRUE(compile_time_string_of_string == compile_time, "compile time strings did not match");
		ASSERT_TRUE(other_compile_time.compare(L"OtHeR_cOmPiLe_TiMe", true) == 0, "case insensitive comparison failed unexpectedly");
		ASSERT_FALSE(other_compile_time.compare(L"cOmPiLe_TiMe", true) == 0, "case insensitive comparison succeeded unexpectedly");

		// substr
		constexpr ktl::unicode_string_view just_time{ compile_time.substr(8) };
		ASSERT_TRUE(just_time == L"time", "substring of second half of string did not match: %wZ", just_time.data());
		constexpr ktl::unicode_string_view just_ompile{ compile_time.substr(1, 6) };
		ASSERT_TRUE(just_ompile == L"ompile", "substring of first half of string did not match: %wZ", just_ompile.data());
		constexpr ktl::unicode_string_view just_compile{ compile_time.substr(0, 7) };
		ASSERT_TRUE(just_compile == L"compile", "substring of first half of string did not match: %wZ", just_compile.data());

		// ends_with
		ASSERT_TRUE(compile_time.ends_with(L"_time"), "ends_with failed to find correct string end");
		ASSERT_TRUE(compile_time.ends_with(L""), "ends_with failed with empty string");
		ASSERT_FALSE(compile_time.ends_with(L"compile"), "ends_with found incorrect ending substring");

		// starts_with
		ASSERT_TRUE(compile_time.starts_with(L"compile"), "starts_with failed to find correct string start");
		ASSERT_TRUE(compile_time.starts_with(L""), "starts_with failed with empty string");
		ASSERT_FALSE(compile_time.starts_with(L"time"), "starts_with found incorrect starting substring");

		// size
		ASSERT_TRUE(compile_time.size() == 12, "unexpected string view size");
		ASSERT_TRUE(compile_time.byte_size() == 12 * sizeof(wchar_t), "unexpected string view size");

		// copy construct/assign.
		ktl::unicode_string heap_string = ktl::unicode_string{ L"heap" };
		ktl::unicode_string_view heap_string_view = heap_string;
		ktl::unicode_string_view copy_construct_heap_view = heap_string;
		ASSERT_TRUE(heap_string_view.data()->Buffer == heap_string.data()->Buffer, "copy assigned string view did not point to original string");
		ASSERT_TRUE(copy_construct_heap_view.data()->Buffer == heap_string.data()->Buffer, "copy constructed string view did not point to original string");
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_ERROR("[NG]: %#x\n", GetExceptionCode());
		return false;
	}

	LOG_TRACE("[OK] ktl::unicode_string_view!\n");
	return true;
}

