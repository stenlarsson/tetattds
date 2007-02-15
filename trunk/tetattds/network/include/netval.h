#ifdef ARM9
#include <sys/socket.h>
#else
#include <arpa/inet.h>
#endif

template <typename T>
class netval;
 
template <>
struct netval<int> {
	netval() {}
  	netval(int i) : val(htonl(i)) {}
	void operator =(int i) { val = htonl(i); }
  	operator int() const { return ntohl(val); }
private:
	int val;
};
 
template <>
struct netval<bool> {
	netval() {}
  	netval(bool i) : val(htonl(i)) {}
	void operator =(int i) { val = htonl(i); }
  	operator int() const { return ntohl(val); }
private:
	bool val;
};
 
template <>
struct netval<short> {
	netval() {}
  	netval(short i) : val(htons(i)) {}
	void operator =(short i) { val = htons(i); }
  	operator short() const { return ntohs(val); }
private:
	short val;
};
 
