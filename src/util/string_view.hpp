#pragma once

#include <cstddef>
#include <cstring>
#include <ostream>
#include <string>

//! Lightweight C++11 std::string_view-like replacement
class string_view
{
	public:
	constexpr string_view(const std::string& str) : m_data{str.data()}, m_size{str.size()} {}
	constexpr string_view(const char* str) : string_view{str, constexpr_strlen(str)} {}
	constexpr string_view(const char* str, std::size_t len) : m_data{str}, m_size{len} {}

	constexpr string_view(const string_view&) = default;
	string_view& operator=(const string_view&) = default;

	string_view(string_view&&) = default;
	string_view& operator=(string_view&&) = default;

	friend bool operator==(const string_view& a, const string_view& b)
	{
		return a.m_size == b.m_size && std::strncmp(a.m_data, b.m_data, a.m_size) == 0;
	}

	friend bool operator!=(const string_view& a, const string_view& b) { return !(a == b); }

	std::string str() const { return {m_data, m_data + m_size}; }

	friend std::ostream& operator<<(std::ostream& os, const string_view& view)
	{
		os.write(view.m_data, view.m_size);
		return os;
	}

	private:
	//! Constexpr version of std::strlen as it is not constexpr in C++11
	static constexpr std::size_t constexpr_strlen(const char* s)
	{
		std::size_t length = 0;

		while (*s != '\0')
		{
			++length;
			++s;
		}

		return length;
	}

	const char* m_data;
	std::size_t m_size;
};

namespace std
{
template<>
struct hash<::string_view>
{
	std::size_t operator()(const ::string_view& s) const noexcept
	{
		// Defer to std::string's hash algorithm for now
		return std::hash<std::string>{}(s.str());
	}
};

} // namespace std
