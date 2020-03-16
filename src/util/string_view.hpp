#pragma once

#include <cstddef>
#include <cstring>
#include <string>

//! Lightweight C++11 std::string_view-like replacement
class string_view
{
	public:
	explicit string_view(const std::string& str)
	{
		m_data = str.data();
		m_size = str.size();
	}

	string_view(const char* str)
	{
		m_data = str;
		m_size = std::strlen(str);
	}

	string_view(const char* str, std::size_t len)
	{
		m_data = str;
		m_size = len;
	}

	string_view(const string_view&) = default;
	string_view& operator=(const string_view&) = default;

	string_view(string_view&&) = default;
	string_view& operator=(string_view&&) = default;

	bool operator==(const string_view& other) const
	{
		return m_size == other.m_size && std::strncmp(m_data, other.m_data, m_size) == 0;
	}

	bool operator!=(const string_view& other) const { return !(*this == other); }

	std::string str() const { return {m_data, m_data + m_size}; }

	private:
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
