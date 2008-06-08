#pragma once

#include <algorithm>
#include <functional>

void* Decompress(const void* source, int* size);
void Decompress(void* destination, const void* source);
void PrintStatus(const char* format, ...);

template <typename It>
static inline void delete_each(It begin, It end)
{
	for (It it = begin; it != end; ++it)
	{
		delete *it;
		*it = NULL;
	}
}

template <typename Container>
void delete_and_clear(Container & c)
{
	delete_each(c.begin(), c.end());
	c.clear();
}

template <typename Container, typename Predicate>
static inline void delete_and_erase_if(Container & c, Predicate const & p)
{
	typename Container::iterator keep =
		std::partition(c.begin(), c.end(), p);
	delete_each(c.begin(), keep);
	c.erase(c.begin(), keep);
}

template <typename Container, typename Unary>
static inline void for_each(Container & c, Unary const & u)
{
	std::for_each(c.begin(), c.end(), u);
}

template <typename Result, typename X, typename Arg>
static inline
typename std::binder2nd< typename std::mem_fun1_t<Result, X, Arg> >
mem_fun_with(Result (X::*f)(Arg), Arg const & arg)
{
	return std::bind2nd(std::mem_fun(f), arg);
}
