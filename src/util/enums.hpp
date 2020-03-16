#pragma once

#include <type_traits>

template<class T>
auto underlying_cast(T enum_value) -> typename std::underlying_type<T>::type
{
    return static_cast<typename std::underlying_type<T>::type>(enum_value);
}

template<class T>
bool check_enum_range(T value, T first, T last)
{
    return underlying_cast(value) >= underlying_cast(first) && underlying_cast(value) <= underlying_cast(last);
}
