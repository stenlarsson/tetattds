#pragma once
#include <stdlib.h>

/**
 * An uint16_t that wraps. The comparison operators are
 * implemented to take this in consideration. This works by
 * assuming that the values we are comparing are not farther apart
 * than half of the maximum value. This way we can know if the
 * values have wrapped or not.
 */
class wrapping
{
public:
	explicit wrapping(uint16_t number) : number(number) {}

	/**
	 * Returns < 0, == 0, or > 0, if right is less, equal, or
	 * greater than this. Right must not be farther than 
	 * 0x7fff from this value in wrapped space.
	 */
	int compare(uint16_t right) {
		int diff = number - right;
		if(abs(diff) > 0x7fff) {
			// one of the values have wrapped
			diff = -diff;
		}
		return diff;
	}

	wrapping& operator ++ () { ++number; return *this; }
	wrapping operator ++ (int) { wrapping t = *this; number++; return t; }
	operator uint16_t () { return number; }

private:
	uint16_t number;
};

inline bool operator != (uint16_t left, wrapping right)
{ return right.compare(left) != 0; }
inline bool operator <  (uint16_t left, wrapping right)
{ return right.compare(left) >  0; }
inline bool operator <= (uint16_t left, wrapping right)
{ return right.compare(left) >= 0; }
inline bool operator >  (uint16_t left, wrapping right)
{ return right.compare(left) <  0; }
inline bool operator >= (uint16_t left, wrapping right)
{ return right.compare(left) <= 0; }
inline bool operator == (uint16_t left, wrapping right)
{ return right.compare(left) == 0; }
