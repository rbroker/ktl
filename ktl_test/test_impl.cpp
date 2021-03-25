#include "test.h"

#include <list>
#include <set>
#include <string>
#include <vector>

struct complex_copyable_object
{
	complex_copyable_object() = default;
	complex_copyable_object(const complex_copyable_object& other) :
		Name(other.Name),
		Value(other.Value)
	{
	}

	complex_copyable_object(ktl::unicode_string_view name, int value) :
		Name(name),
		Value(value)
	{
	}

	complex_copyable_object(ktl::unicode_string_view name) :
		Name(name)
	{
	}

	ktl::unicode_string Name;
	int Value = 5;
	ktl::vector<int> Vec;

private:
	int NonStandard;
};

struct complex_object
{
	complex_object() = default;
	complex_object(complex_object&&) = default;
	complex_object& operator=(complex_object&&) = default;

	complex_object(ktl::unicode_string_view name, int value) :
		Name(name),
		Value(value)
	{
	}

	complex_object(ktl::unicode_string_view name) :
		Name(name)
	{
	}

	ktl::unicode_string Name;
	int Value = 5;
	ktl::vector<int> Vec;

private:
	int NonStandard = -5;
};

bool test_set()
{
	__try
	{
		ktl::set<ktl::unicode_string> set;

		// insert & grow
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"foo" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"bar" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"baz" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"hoge" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"piyo" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"fuga" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"hogera" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"toto" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"xyzzy" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"qux" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"quux" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"quuz" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"corge" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"corgi" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"corgii" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"many corgs" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"trapezium" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"a" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"b" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"c" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"d" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"$" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"¢" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"ह" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"€" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"𐍈" }), "failed to insert string into set");
		ASSERT_TRUE(set.insert(ktl::unicode_string_view{ L"よろしくお願いします。" }), "failed to insert string into set");

		// find
		{
			auto it = set.find(ktl::unicode_string_view{ L"$" });
			ASSERT_TRUE(it != set.end(), "unable to find expected entry in set");
			ASSERT_TRUE(*it == L"$", "found entry didn't contain expected value: %wZ", it->data());
		}

		{
			auto it = set.find(ktl::unicode_string_view{ L"corgi" });
			ASSERT_TRUE(it != set.end(), "unable to find expected entry in set");
			ASSERT_TRUE(*it == L"corgi", "found entry didn't contain expected value: %wZ", it->data());
		}

		// find code unit character string
		{
			auto it = set.find(ktl::unicode_string_view{ L"𐍈" });
			ASSERT_TRUE(it != set.end(), "unable to find expected entry in set");
			ASSERT_TRUE(*it == L"𐍈", "found entry didn't contain expected value: %wZ", it->data());
		}

		ktl::unicode_string extended_code_points{ L"𐍈" };
		ASSERT_TRUE(extended_code_points.byte_size() == (sizeof(L"𐍈") - sizeof(UNICODE_NULL)), "unexpected byte count for extended code point: %llu", extended_code_points.byte_size());
		ASSERT_TRUE(extended_code_points.size() == (sizeof(L"𐍈") - sizeof(UNICODE_NULL)) / sizeof(wchar_t), "unexpected character count for extended code point: %llu", extended_code_points.size());

		// iteration & erase
		ASSERT_TRUE(set.find(ktl::unicode_string_view{ L"$" }) != set.end(), "unable to find expected entry in set");
		auto sizeBefore = set.size();
		for (auto it = set.begin(); it != set.end();)
		{
			if (*it == L"$")
				it = set.erase(*it);
			else
				++it;
		}
		ASSERT_TRUE(set.find(ktl::unicode_string_view{ L"$" }) == set.end(), "erase entry still in set");
		ASSERT_TRUE(sizeBefore == set.size() + 1, "more elements than expected erased from set: %llu -> %llu", sizeBefore, set.size());
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_ERROR("[NG]: %#x\n", GetExceptionCode());
		return false;
	}

	LOG_TRACE("[OK] ktl::set!\n");
	return true;
}

bool test_vector()
{
	__try
	{
		// default constructor
		ktl::vector<int> vec;

		ASSERT_TRUE(vec.empty(), "vector was not empty after default construction");
		ASSERT_TRUE(vec.size() == 0, "vector size was not zero after default construction: %llu", vec.size());
		ASSERT_TRUE(vec.capacity() < 10, "vector capacity unexpected high at start of test: %llu", vec.capacity());

		// lvalue push
		for (int i = 0; i < 10; ++i)
		{
			ASSERT_TRUE(vec.push_back(i), "failed lvalue push_back");
		}

		for (int i = 0; i < 10; ++i)
		{
			ASSERT_TRUE(vec[i] == i, "unexpected value in vector after push_back/resize: %d != %d", vec[i], i);
		}

		ASSERT_TRUE(vec.size() == 10, "unexpected vector size after pushing lvalues");

		// rvalue push
		int x = 5;
		for (int i = 0; i < 5; ++i)
		{
			ASSERT_TRUE(vec.push_back(ktl::move(x)), "failed rvalue push_back");
			ASSERT_TRUE(vec.back() == x, "unexpected value in vector after rvalue push_back: %d != %d", vec.back(), x);
		}
		ASSERT_TRUE(vec.size() == 15, "unexpected vector size after pushing rvalues");

		// emplace back
		for (int i = 0; i < 5; ++i)
		{
			ASSERT_TRUE(vec.emplace_back(i), "failed emplace_back");
			ASSERT_TRUE(vec.back() == i, "unexpected value in vector after emplace_back: %d != %d", vec.back(), i);
		}
		ASSERT_TRUE(vec.size() == 20, "unexpected vector size after emplacing rvalues: %llu", vec.size());

		// clear
		vec.clear();
		ASSERT_TRUE(vec.size() == 0, "unexpected vector size after clearing: %llu", vec.size());
		ASSERT_TRUE(vec.capacity() >= 20, "vector capacity unexpectedly low after clearing: %llu", vec.capacity());

		// reserve
		auto initialCapacity = vec.capacity();
		ASSERT_TRUE(vec.reserve(initialCapacity + 1), "vector reserve failed");
		ASSERT_TRUE(vec.capacity() == initialCapacity + 1, "vector reserve had unexpected impact on capacity: %llu", vec.capacity());
		ASSERT_TRUE(vec.size() == 0, "vector reserve affected size: %llu", vec.size());

		// resize
		auto newSize = vec.capacity() * 2;
		ASSERT_TRUE(vec.resize(newSize), "failed to resize vector to twice current capacity");
		ASSERT_TRUE(vec.size() == newSize, "unexpected size after vector resize: %llu", vec.size());

		// pop_back
		vec.pop_back();
		ASSERT_TRUE(vec.size() == newSize - 1, "unexpected size after vector pop_back: %llu", vec.size());

		// non-trivial types
		ktl::vector<ktl::unicode_string> strVec;
		ktl::unicode_string str{ L"string" };
		ASSERT_TRUE(strVec.push_back(str), "failed to push string");
		ASSERT_TRUE(strVec.emplace_back(L"other"), "failed to emplace string");
		ASSERT_TRUE(strVec.size() == 2, "incorrect size after pushing strings");
		ASSERT_TRUE(strVec[0] == L"string", "invalid value in vector after pushing string");
		ASSERT_TRUE(strVec[1] == L"other", "invalid value in vector after pushing string");

		static_assert(!ktl::is_trivially_copyable_v<complex_object>);
		static_assert(!ktl::is_standard_layout_v<complex_object>);

		ktl::vector<complex_object> structVec;
		ASSERT_TRUE(structVec.emplace_back(L"hello world", 10), "failed to emplace first structure in vector");
		ASSERT_TRUE(structVec.emplace_back(L"world"), "failed to emplace second structure in vector");

		ASSERT_TRUE(structVec[0].Value == 10, "invalid value in first emplaced structure");
		ASSERT_TRUE(structVec[0].Name == L"hello world", "invalid name in first emplaced structure: %wZ", structVec[0].Name.data());
		ASSERT_TRUE(structVec[1].Value == 5, "invalid value in second emplaced structure");
		ASSERT_TRUE(structVec[1].Name == L"world", "invalid name in second emplaced structure: %wZ", structVec[1].Name.data());

		// range-based for
		size_t count = 0;
		for (const auto& s : strVec)
		{
			switch (count++)
			{
			case 0:
				ASSERT_TRUE(s == L"string", "first element in iteration was incorrect");
				break;
			case 1:
				ASSERT_TRUE(s == L"other", "second element in iteration was incorrect");
				break;
			}
		}
		ASSERT_TRUE(count == 2, "unexpected number of elements in iteration");

		// erase
		vec.clear();
		for (int i = 0; i < 5; ++i)
			ASSERT_TRUE(vec.push_back(i), "failed to push back elements");

		// erase front
		for (auto it = vec.begin(); it != vec.end(); )
		{
			if (*it == 0)
				it = vec.erase(it);

			++it;
		}

		ASSERT_TRUE(vec.size() == 4, "unexpected size after erase: %llu", vec.size());
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 0; }) == vec.end(), "found erased element");
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 1; }) != vec.end(), "didn't find expected element (1)");
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 2; }) != vec.end(), "didn't find expected element (2)");
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 3; }) != vec.end(), "didn't find expected element (3)");
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 4; }) != vec.end(), "didn't find expected element (4)");

		// erase back
		for (auto it = vec.begin(); it != vec.end(); )
		{
			if (*it == 4)
				it = vec.erase(it);

			++it;
		}

		ASSERT_TRUE(vec.size() == 3, "unexpected size after erase: %llu", vec.size());
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 1; }) != vec.end(), "didn't find expected element (1)");
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 2; }) != vec.end(), "didn't find expected element (2)");
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 3; }) != vec.end(), "didn't find expected element (3)");
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 4; }) == vec.end(), "found erased element");

		// erase middle
		for (auto it = vec.begin(); it != vec.end(); )
		{
			if (*it == 2)
				it = vec.erase(it);

			++it;
		}

		ASSERT_TRUE(vec.size() == 2, "unexpected size after erase: %llu", vec.size());
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 1; }) != vec.end(), "didn't find expected element (1)");
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 2; }) == vec.end(), "found erased element");
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 3; }) != vec.end(), "didn't find expected element (3)");

		// Erase remaining elements
		for (auto it = vec.begin(); it != vec.end();)
			it = vec.erase(it);

		ASSERT_TRUE(vec.empty(), "vector not empty after erasing all elements");

		// erase complex
		ktl::vector<ktl::unicode_string> vecStr;
		static_assert(!ktl::is_trivially_copyable_v<ktl::unicode_string>);

		ASSERT_TRUE(vecStr.emplace_back(L"one"), "failed to emplace test string");
		ASSERT_TRUE(vecStr.emplace_back(L"two"), "failed to emplace test string");
		ASSERT_TRUE(vecStr.emplace_back(L"three"), "failed to emplace test string");
		ASSERT_TRUE(vecStr.emplace_back(L"four"), "failed to emplace test string");
		ASSERT_TRUE(vecStr.emplace_back(L"five"), "failed to emplace test string");

		// erase complex front
		for (auto it = vecStr.begin(); it != vecStr.end(); )
		{
			if (*it == L"one")
				it = vecStr.erase(it);

			++it;
		}

		ASSERT_TRUE(vecStr.size() == 4, "unexpected size after erase: %llu", vecStr.size());
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"one"; }) == vecStr.end(), "found erased element");
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"two"; }) != vecStr.end(), "didn't find expected element (two)");
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"three"; }) != vecStr.end(), "didn't find expected element (three)");
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"four"; }) != vecStr.end(), "didn't find expected element (four)");
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"five"; }) != vecStr.end(), "didn't find expected element (five)");

		// erase back
		for (auto it = vecStr.begin(); it != vecStr.end(); )
		{
			if (*it == L"five")
				it = vecStr.erase(it);

			++it;
		}

		ASSERT_TRUE(vecStr.size() == 3, "unexpected size after erase: %llu", vecStr.size());
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"two"; }) != vecStr.end(), "didn't find expected element (two)");
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"three"; }) != vecStr.end(), "didn't find expected element (three)");
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"four"; }) != vecStr.end(), "didn't find expected element (four)");
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"five"; }) == vecStr.end(), "found erased element");

		// erase middle
		for (auto it = vecStr.begin(); it != vecStr.end(); )
		{
			if (*it == L"three")
				it = vecStr.erase(it);

			++it;
		}

		ASSERT_TRUE(vecStr.size() == 2, "unexpected size after erase: %llu", vecStr.size());
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"two"; }) != vecStr.end(), "didn't find expected element (two)");
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"three"; }) == vecStr.end(), "found erased element");
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"four"; }) != vecStr.end(), "didn't find expected element (four)");

		// Erase remaining elements
		for (auto it = vecStr.begin(); it != vecStr.end();)
			it = vecStr.erase(it);

		ASSERT_TRUE(vecStr.empty(), "vector not empty after erasing all elements");

		// huge vector.
		vec.clear();

		for (int i = 0; i < 1000000; ++i)
			ASSERT_TRUE(vec.emplace_back(i), "large emplace failed at: %d", i);

		for (int i = 0; i < 1000000; ++i)
			ASSERT_TRUE(vec[i] == i, "unexpected value at %d: %d", i, vec[i]);

		ASSERT_TRUE(vec.capacity() >= 1000000, "unexpectedly low vector capacity: %llu", vec.capacity());
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_ERROR("[NG]: %#x\n", GetExceptionCode());
		return false;
	}

	LOG_TRACE("[OK] ktl::vector!\n");
	return true;
}

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
		ktl::unicode_string heap_string = L"heap";
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

bool test_list()
{
	__try
	{
		ktl::list<int> int_list;

		ASSERT_TRUE(int_list.empty(), "default constructed list was not empty");

		// push back
		{
			for (int i = 0; i < 500; ++i)
				ASSERT_TRUE(int_list.push_back(i), "failed to push element to back of list");

			{
				auto it = int_list.begin();
				for (int i = 0; i < 500; ++i, ++it)
					ASSERT_TRUE(*it == i, "unexpected value when pushing to back of list: %d != %d", *it, i);
			}

			ASSERT_TRUE(int_list.size() == 500, "unexpected size of list after pushing elements");
			ASSERT_TRUE(int_list.front() == 0, "unexpected value at front of list");
			ASSERT_TRUE(int_list.back() == 499, "unexpected value at back of list");

			int_list.pop_front();
			ASSERT_TRUE(int_list.front() == 1, "unexpected value at front of list");

			int_list.pop_back();
			ASSERT_TRUE(int_list.back() == 498, "unexpected value at back of list");

			auto b = int_list.begin();
			auto e = int_list.end();

			for (auto it = b; it != e; ++it)
			{
				if (*it != 250 && *it != 498)
					continue;

				it = int_list.erase(it);
			}

			ASSERT_TRUE(ktl::find_if(int_list.begin(), int_list.end(), [](auto& v) { return v == 250; }) == int_list.end(), "found erased element in list: 250");
			ASSERT_TRUE(ktl::find_if(int_list.begin(), int_list.end(), [](auto& v) { return v == 498; }) == int_list.end(), "found erased element in list: 499");
			ASSERT_FALSE(ktl::find_if(int_list.begin(), int_list.end(), [](auto& v) { return v == 251; }) == int_list.end(), "did not find expected element in list: 251");
			ASSERT_FALSE(ktl::find_if(int_list.begin(), int_list.end(), [](auto& v) { return v == 249; }) == int_list.end(), "did not find expected element in list: 249");

			int_list.clear();
			ASSERT_TRUE(int_list.empty(), "list not empty after clearing");
			ASSERT_TRUE(int_list.size() == 0, "list empty, but has non-zero size");
		}


		// push front
		{
			for (int i = 0; i < 5; ++i)
				ASSERT_TRUE(int_list.push_front(i), "failed to push element to front of list");

			auto it = int_list.begin();
			for (int i = 4; i >= 0; --i, ++it)
				ASSERT_TRUE(*it == i, "unexpected value when pushing to front of list: %d != %d", *it, i);

			int_list.clear();
		}

		// emplace back
		{
			for (int i = 0; i < 5; ++i)
				ASSERT_TRUE(int_list.emplace_back(i), "failed to emplace element to back of list");


			auto it = int_list.begin();
			for (int i = 0; i < 5; ++i, ++it)
				ASSERT_TRUE(*it == i, "unexpected value when emplacing to front of list: %d != %d", *it, i);


			int_list.clear();
		}

		// emplace front
		{
			for (int i = 0; i < 5; ++i)
				ASSERT_TRUE(int_list.emplace_front(i), "failed to emplace element to front of list");

			auto it = int_list.begin();
			for (int i = 4; i >= 0; --i, ++it)
				ASSERT_TRUE(*it == i, "unexpected value when emplacing to front of list: %d != %d", *it, i);

			int_list.clear();
		}

		ktl::list<complex_object> complex_list;
		for (int i = 0; i < 5; ++i)
		{
			complex_object obj;
			ASSERT_TRUE(complex_list.emplace_back(ktl::move(obj)), "emplace back failed for list");
		}

		ASSERT_TRUE(complex_list.size() == 5, "unexpected list of complex objects");
		complex_list.clear();
		ASSERT_TRUE(complex_list.empty(), "list was not empty after clearing");

		ktl::list<complex_copyable_object> copyable_complex_list;
		for (int i = 0; i < 6; ++i)
		{
			complex_copyable_object obj;
			ASSERT_TRUE(copyable_complex_list.push_front(obj), "push front failed for list");
		}

		ASSERT_TRUE(copyable_complex_list.size() == 6, "unexpected list of complex objects");
		copyable_complex_list.clear();
		ASSERT_TRUE(copyable_complex_list.empty(), "list was not empty after clearing");
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_ERROR("[NG]: %#x\n", GetExceptionCode());
		return false;
	}

	LOG_TRACE("[OK] ktl::list!\n");
	return true;
}