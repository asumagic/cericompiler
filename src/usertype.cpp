#include "usertype.hpp"

UserType::UserType(UserType::Category category) : category{category}
{
	switch (category)
	{
	case Category::POINTER: new (&layout_data.pointer) PointerType();
	}
}

UserType::~UserType()
{
	switch (category)
	{
	case Category::POINTER: layout_data.pointer.~PointerType();
	}
}

bool operator==(const UserType& a, const UserType& b)
{
	if (a.category != b.category)
	{
		return false;
	}

	switch (a.category)
	{
	case UserType::Category::POINTER:
	{
		return a.layout_data.pointer.target == b.layout_data.pointer.target;
	}
	}

	return true;
}
