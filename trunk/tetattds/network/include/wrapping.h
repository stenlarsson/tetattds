#pragma once
#include <stdlib.h>

/**
 * An uint16_t that wraps. The comparison operators are
 * implemented to take this in consideration. This works by
 * assuming that the values we are comparing are not farther apart
 * than half of the maximum value. This way we can know if the
 * values have wrapped or not.
 */
template <typename T>
class wrapping_base
{
	static const T LIMIT = (T)(0x7fffffff >> (8 * (4 - sizeof(T))));

public:
	explicit wrapping_base(T number) : number(number) {}

	/**
	 * Returns < 0, == 0, or > 0, if right is less, equal, or
	 * greater than this. Right must not be farther than 
	 * 0x7fff from this value in wrapped space.
	 */
	int compare(T right) {
		int diff = number - right;
		if(abs(diff) > LIMIT) {
			// one of the values have wrapped
			diff = -diff;
		}
		return diff;
	}

	wrapping_base<T>& operator ++ () { ++number; return *this; }
	wrapping_base<T> operator ++ (int) { wrapping_base<T> t = *this; number++; return t; }
	operator T () { return number; }

private:
	T number;
};

template <typename T>
inline bool operator != (T left, wrapping_base<T> right)
{ return right.compare(left) != 0; }
template <typename T>
inline bool operator <  (T left, wrapping_base<T> right)
{ return right.compare(left) >  0; }
template <typename T>
inline bool operator <= (T left, wrapping_base<T> right)
{ return right.compare(left) >= 0; }
template <typename T>
inline bool operator >  (T left, wrapping_base<T> right)
{ return right.compare(left) <  0; }
template <typename T>
inline bool operator >= (T left, wrapping_base<T> right)
{ return right.compare(left) <= 0; }
template <typename T>
inline bool operator == (T left, wrapping_base<T> right)
{ return right.compare(left) == 0; }

struct wrapping : public wrapping_base <uint16_t> {
	wrapping(uint16_t number) : wrapping_base<uint16_t>(number) {}
};
struct wrapping8 : public wrapping_base <uint8_t> {
	wrapping8(uint8_t number) : wrapping_base<uint8_t>(number) {}
};
