#pragma once

#include <cstddef>
#include <cstring>
#include <iosfwd>
#include <string>
#include <utility>

//! Lightweight C++11 std::string_view-like replacement
class string_view
{
	public:
	string_view(const std::string& str);
	constexpr string_view(const char* str) : string_view{str, constexpr_strlen(str)} {}
	constexpr string_view(const char* str, std::size_t len) : m_data{str}, m_size{len} {}

	constexpr string_view(const string_view&) = default;
	string_view& operator=(const string_view&) = default;

	string_view(string_view&&) = default;
	string_view& operator=(string_view&&) = default;

	friend bool operator==(string_view a, string_view b);
	friend bool operator!=(string_view a, string_view b);

	std::string str() const;
				operator std::string() const;

	friend std::ostream& operator<<(std::ostream& os, string_view view);

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
	std::size_t operator()(::string_view s) const noexcept;
};

} // namespace std
