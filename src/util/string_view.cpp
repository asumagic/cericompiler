#include "string_view.hpp"

#include <ostream>
#include <string>

std::ostream& operator<<(std::ostream& os, string_view view)
{
	os.write(view.m_data, view.m_size);
	return os;
}

bool operator==(string_view a, string_view b)
{
	return a.m_size == b.m_size && std::strncmp(a.m_data, b.m_data, a.m_size) == 0;
}

bool operator!=(string_view a, string_view b) { return !(a == b); }

string_view::string_view(const std::string& str) : m_data{str.data()}, m_size{str.size()} {}

std::string  string_view::str() const { return {m_data, m_data + m_size}; }
string_view::operator std::string() const { return str(); }

std::size_t std::hash<::string_view>::operator()(string_view s) const noexcept
{
	// Defer to std::string's hash algorithm for now
	return std::hash<std::string>{}(s.str());
}
