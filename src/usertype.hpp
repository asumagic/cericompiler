#pragma once

#include "types.hpp"

#include <new>

struct UserType
{
	enum class Category
	{
		POINTER,
		/*ARRAY,
		RECORD*/
	};

	struct PointerType
	{
		Type target;
	};

	UserType(Category category);
	~UserType();

	Category category;

	union
	{
		PointerType pointer;
	} layout_data;

	friend bool operator==(const UserType& a, const UserType& b);
};
