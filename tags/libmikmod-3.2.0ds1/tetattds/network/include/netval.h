#ifdef ARM9
// Gah, silence the malicous warnings of sys/socket
#ifdef FD_SETSIZE
#undef FD_SETSIZE
#endif
#include <sys/socket.h>
#elif WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

template <typename T>
class netval;
 
template <>
struct netval<unsigned int> {
	netval() {}
  	netval(unsigned int i) : val(htonl(i)) {}
	void operator =(unsigned int i) { val = htonl(i); }
  	operator unsigned int() const { return ntohl(val); }
private:
	unsigned int val;
};
 
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
  	netval(bool i) : val(htonl(i) != 0) {}
	void operator =(bool i) { val = htonl(i) != 0; }
  	operator bool() const { return ntohl(val) != 0; }
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
 
