#pragma once

#include <algorithm>

void* Decompress(const void* source, int* size);
void Decompress(void* destination, const void* source);
void PrintSpinner();
void PrintStatus(char* format, ...);

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
