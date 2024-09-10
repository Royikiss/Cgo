/*************************************************************************
	> File Name: Cgo-Socket.h
	> Author:Royi 
	> Mail:royi990001@gmail.com 
	> Created Time: Mon 29 Jul 2024 08:10:18 PM CST
	> Describe: 
 ************************************************************************/
#ifndef _TCP_SOCKET_H__
#define _TCP_SOCKET_H__

#include <iostream>
#include <cstdint>
#include <functional>
#include <new>
#include <ostream>
#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory>
#include <type_traits>
#include <sys/un.h>


#define __NAMESPACE_Cgo_BEGIN__ namespace Cgo {
#define __NAMESPACE_Cgo_END__ } 

__NAMESPACE_Cgo_BEGIN__

// mark
class tcp_ip4 {};
class tcp_unix {};

template <class T> class sockaddr;
template <class T> class socket;

/**
 * @brief Extract the corresponding storage structure structure according to the service type.
 */
template <typename T> struct sockaddr_traits;

template <>
struct sockaddr_traits<class tcp_ip4> {
	using type = struct ::sockaddr_in;
};

template <>
struct sockaddr_traits<class tcp_unix> {
    using type = struct ::sockaddr_un;
};


template <typename T>
class _base_sockaddr {
	using len_t = socklen_t;
	using addr_t = typename sockaddr_traits<T>::type;
protected:
	virtual len_t flush_len() = 0;
public:	
	len_t len;
	virtual ~_base_sockaddr() = default;
	virtual len_t &get_len() = 0;
	virtual addr_t &get_addr() = 0;
	virtual void _message() = 0;
};

template <typename T>
class _base_socket {
	using socket_t = int;
	using csocket_t = Cgo::socket<T>;
	using addr_t = Cgo::sockaddr<T>;
protected:	
	socket_t sockfd;
public:
	_base_socket() : sockfd(-1) {}
	_base_socket(const int sockfd) : sockfd(sockfd) {}
	virtual ~_base_socket() = default;
	csocket_t operator=(const csocket_t &other) {
		this->sockfd = other.sockfd;
		return *this;
	} 
	bool operator<(const csocket_t &other) const {
		return this->sockfd < other.sockfd;
	}

	bool operator<(const int fd) const {
		return this->sockfd < fd;
	}
	bool operator>(const csocket_t &other) const {
		return this->sockfd > other.sockfd;
	}
	bool operator>(const int fd) const {
		return this->sockfd > fd;
	}
	bool operator<=(const csocket_t &other) const {
		return this->sockfd <= other.sockfd;
	}
	bool operator<=(const int fd) const {
		return this->sockfd <= fd;
	}
	bool operator>=(const csocket_t &other) const {
		return this->sockfd >= other.sockfd;
	}
	bool operator>=(const int fd) const {
		return this->sockfd >= fd;
	}
	bool operator==(const csocket_t &other) const {
		return this->sockfd == other.sockfd;
	}
	bool operator==(const int fd) const {
		return this->sockfd == fd;
	}
	bool operator!=(const csocket_t &other) const {
		return this->sockfd != other.sockfd;
	}
	bool operator!=(const int fd) const {
		return this->sockfd != fd;
	}
	operator int() const {
		return this->sockfd;
	}

	virtual int bind(Cgo::sockaddr<T> &) = 0;
	virtual int listen(int) = 0; 
	virtual csocket_t accept() = 0;
	virtual csocket_t accept(addr_t &) = 0;
	virtual ::ssize_t recv(void *, ::size_t, int) = 0;
	virtual ::ssize_t send(const void *, ::size_t len, int) = 0;
	virtual int connect(Cgo::sockaddr<T> &) = 0;
	virtual int close() = 0;
};


/**
 * @brief based on tcpSocketaddr
 * @details detailed description
 */
template <>
class sockaddr<Cgo::tcp_ip4> : public _base_sockaddr<Cgo::tcp_ip4> {
	using self = sockaddr<Cgo::tcp_ip4>;
	using used_t = Cgo::tcp_ip4;
	using addr_t = typename Cgo::sockaddr_traits<Cgo::tcp_ip4>::type;
	using len_t = socklen_t ;

private:

	addr_t addr;
	len_t flush_len() override {
		this->len = sizeof(this->addr);
		return this->len;
	}
	
public:

	/**
	 * @brief Default constructor
	 * @details 
	 * 		Default: 
	 *		family = AF_INET
	 *		port = 0 
	 *		addr = INADDR_ANY
	 */
    sockaddr()
	{
		addr.sin_family = AF_INET;
		addr.sin_port = htons(0);
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		this->flush_len();
	}

	/**
	 * @brief Structure with parameters
	 * @details 
	 *		default:
	 *		family = AF_INET
	 * @param add (const uint32_t) IP address, this constructor is usually used on the server side
	 * @param port (const uint16_t) port
	 */
	sockaddr(const uint32_t add, const uint16_t port) 
	{
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = htonl(add);
		this->flush_len();
	}

	/**
	 * @brief constructor
	 * @details 
	 *		default: 
	 *		family = AF_INET
	 * @param p (const char *) IP address, this constructor is usually used on the client side
	 * @param port (const int) port
	 */
	sockaddr(const char *p, const uint16_t port)
	{
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = inet_addr(p);
		this->flush_len();
	}

	/**
	 * @brief transstructor
	 * @details 
	 * 		Trans the incoming structure to tcpSocketaddr. 
	 * @param addr (struct sockaddr_in *) incoming structure 
	 */
	sockaddr(const struct sockaddr_in *addr) :
		addr(*addr)
	{
		this->flush_len();
	}

	/**
	 * @brief copy constructor
	 * @details 
	 * 		Convert the incoming structure to tcpSocketaddr. 
	 * @param other (Cgo::tcpSocketaddr &) incoming structure 
	 */
	sockaddr(const Cgo::sockaddr<used_t> &other) :
		addr(other.addr)
	{
		this->flush_len();
	}

	void _message() override {
		std::cout << "family: " << addr.sin_family << std::endl;
		std::cout << "port: " << ::ntohs(addr.sin_port) << std::endl;
		std::cout << "addr: " << ::inet_ntoa(addr.sin_addr) << std::endl;
		std::cout << "len: " << len << std::endl;
		return ;
	}

	char *show_ip() {
		return ::inet_ntoa(addr.sin_addr);
	}		

	uint16_t show_prot() {
		return ::ntohs(addr.sin_port);
	}

	/**
	 * @brief get_addr
	 * @return (addr_t &) type
	 */
	addr_t &get_addr() override {
		return this->addr;
	}

	/**
	 * @brief get_len
	 * @return (len_t &) type 
	 */
	len_t &get_len() override {
		return this->len;
	}

	/**
	 * @brief set_family
	 * @details 
	 * 		set the type of address family
	 * @param family (const int), such as AF_INET
	 */
	void set_family(const int family) {
		this->addr.sin_family = family;
		return ;
	}

	/**
	 * @brief set_port
	 * @details 
	 *		set the port of address
	 * @param port (const int)
	 */
	void set_port(const int port) {
		this->addr.sin_port = ::htons(port);
		return ;
	}

	/**
	 * @brief set_addr
	 * @details 
	 *		set the IP address of this structor
	 * @param ip (const char *)
	 */
	void set_addr(const char *ip) {
		this->addr.sin_addr.s_addr = ::inet_addr(ip);
		return ;
	}

	~sockaddr() = default;

};

template <>
class socket<Cgo::tcp_ip4> : public Cgo::_base_socket<Cgo::tcp_ip4> {
	using self = Cgo::socket<Cgo::tcp_ip4>;
	using used_t = Cgo::tcp_ip4;
	using base_t = Cgo::_base_socket<Cgo::tcp_ip4>;
	using addr_t = Cgo::sockaddr<Cgo::tcp_ip4>;
	using socket_t = int;
private:

	bool is_sockfd_open() {
		int flags = ::fcntl(sockfd, F_GETFL);
		if (flags == -1) {
			return false;
		} else {
			return true;
		}
	}
	
public:

	/**
	 * @brief default constructor
	 * @details 
	 * 		Create a new socket object with the lowest currently unused value as the socket descriptor.
	 */
	socket() : base_t::_base_socket() {}

	/**
	 * @brief transtructor
	 * @details 
	 *		Create a new socket object using the passed parameters as the socket descriptor. 
	 *	However, please note that the file descriptor corresponding to this socket is undefined. 
	 *	It is best not to use this construction method. If you want to use , you can only pass 
	 *	in one other known socket descriptor.
	 * @param fd (const int) known socket descriptor
	 */
	socket(const int fd) :
		base_t::_base_socket(fd)
	{}

	/**
	 * @brief copy constructor
	 * @details 
	 * 		transform the incoming socket object to this object
	 * @param other (const Cgo::tcp_socket &) incoming socket object
	 */
	socket(const self &other) :
		base_t::_base_socket(other.sockfd)
	{}

	~socket() = default;

	/**
	 * @brief make the object to a socket
	 */		
	socket_t socket_construct() {
		if ((this->sockfd = ::socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			return sockfd;
		}
		return sockfd;
	}

	/**
	 * @brief bind to socket 
	 * @details 
	 * 		Bind the Cgo::tcpSocketaddr object to the current Cgo::tcp_socket object
	 * @param addr (Cgo::tcpSocketaddr &) Cgo::tcpSocketaddr object
	 * @return (int) 
	 * 		On success, zero is returned.  On error, -1 is returned, 
	 * 	and errno is set appropriately.
	 */
	int bind(addr_t &addr) override {
		return ::bind(sockfd, (struct ::sockaddr *)&addr.get_addr(), addr.get_len());
	}
	
	/**
	 * @brief set socket to listening mode
	 * @details 
	 * 		Set the file descriptor corresponding to the current socket 
	 *	object to listening mode. This method is irreversible.
	 * @param backlog (int) Maximum request waiting queue
	 * @return (int) 
	 *		On success, zero is returned.  On error, -1 
	 *	is returned, and errno is set appropriately.
	 */
	int listen(int backlog = 3) override {
		return ::listen(sockfd, backlog);
	}

	/**
	 * @brief init a tcp socket as server
	 * @details 
	 *		Makes the current object bound to a newly created file descriptor,
	 *	 and automatically performs the bind and listen operations based on 
	 *	the two parameters passed in. After binding, details about the 
	 *	incoming will not be preserved
	 * @param port (const uint16_t) port
	 * @param backlog (int) backlog
	 * @return Sucessfully return 0, otherwise return will depends on the bind() and listen() functions
	 */
	int init(const uint16_t port, int backlog = 3) {
		addr_t taddr(INADDR_ANY, port);
		int temp;
		this->socket_construct();
		if ((temp = this->bind(taddr)) < 0) {
			return temp;
		}
		if ((temp = this->listen(backlog) < 0)) {
			return temp;
		}
		return 0;
	}

	self accept() override {
		int fd = ::accept(sockfd, NULL, NULL);
		if (fd < 0) {
			return fd;
		} else {
			self temp(fd);
			return temp;
		}
	}
	
	/**
	 * @brief accept new connection
	 * @details new connection will be created and return a new socket object
	 *	Information of new connection will be stored in the passed Cgo::tcpSocketaddr object.
	 * @param addr (Cgo::tcpSocketaddr &) incoming Cgo::tcpSocketaddr object
	 * @return (Cgo::tcp_socket ) 
	 *		new socket object
	 */
	self accept(addr_t &addr) override {
		int fd = ::accept(sockfd, (struct ::sockaddr *)&addr.get_addr(), &addr.get_len());
		if (fd < 0) {
			return fd;
		} else {
			self temp(fd);
			return temp;
		}
	}

	/**
	 * @brief Receive messages from socket
	 * @param buf (void *) buffer to store data
	 * @param len (::size_t) length of buffer
	 * @param flags (int) flags more detiails see man pages
	 * @return (ssize_t)
	 *		       These calls return the number of bytes received, or 
	 *	-1 if an error occurred.  In the event of an error, errno is set 
	 *	to indicate the error.
	 *		When a stream socket peer has performed an orderly shutdown, 
	 *	the return value will be 0 (the traditional "end-of-file" return).
	 *		Datagram sockets in various domains (e.g., the UNIX and 
	 *	nternet domains) permit zero-length datagrams.  When such  a  datagram  
	 *	is  re‐ceived, the return value is 0.
	 *		The value 0 may also be returned if the requested number of 
	 *	bytes to receive from a stream socket was 0.
	 */	
	::ssize_t recv(void *buf, ::size_t len, int flags = 0) override {
		return ::recv(this->sockfd, buf, len, flags);
	}

	/**
	 * @brief Send messages to socket
	 * @param buf (const void *) buffer to store data
	 * @param len (::size_t) length of buffer
	 * @param flags (int) flags more detiails see man pages
	 * @return return type return description
	 */
	::ssize_t send(const void *buf, ::size_t len, int flags = 0) override {
		return ::send(this->sockfd, buf, len, flags);
	}

	/**
	 * @brief connect to the address of Cgo::tcpSocketaddr 
	 * @details 
	 *		This method is irreversible. At the same time, The current 
	 *	tcp_socket object will bind the incoming Cgo::tcpSocketaddr address. 
	 *	You can manage it directly through the current object.
	 * @param addr (Cgo::tcpSocketaddr &) address object of willing to connect
	 * @return (int) If the connection or binding succeeds, zero is returned.  
	 *	On error, -1 is returned, and errno is set appropriately.
	 */
	int connect(addr_t &addr) override {
		return ::connect(sockfd, (struct ::sockaddr *)&addr.get_addr(), addr.get_len());
	}

	/**
	 * @brief close the socket
	 * @details if the socket is not open, it will not be closed. And notihing 
	 *	will happen.
	 * @return (int) 
	 * 		close() returns zero on success or has been closed.  On error, -1 is 
	 *	returned, and errno is set appropriately.
	 */
	int close() override {
		if (is_sockfd_open()) return ::close(sockfd);
		return 0;
	}
};	


template <>
class sockaddr<Cgo::tcp_unix> : public Cgo::_base_sockaddr<Cgo::tcp_unix> {
	using self = sockaddr<Cgo::tcp_unix>;
	using used_t = Cgo::tcp_unix;
	using addr_t = typename Cgo::sockaddr_traits<Cgo::tcp_unix>::type;
	using len_t = socklen_t ;

private:

	addr_t addr;

	len_t flush_len() override {
		this->len = sizeof(this->addr);
		return this->len;
	}

public:
    sockaddr()
	{
		this->addr.sun_family = AF_UNIX;
		this->addr.sun_path[0] = '\0';
		this->flush_len();
	}

	sockaddr(const char *path) {
		addr.sun_family = AF_UNIX;
		addr.sun_path[0] = '\0';
		this->flush_len();
	}

	sockaddr(const addr_t &addr) {
		this->addr.sun_family = addr.sun_family;
		::strcpy(this->addr.sun_path, addr.sun_path);
		this->flush_len();
	}	

	len_t &get_len() override {
		return this->len;
	}

	addr_t &get_addr() override {
		return this->addr;
	}

	void _message() override {
		std::cout << "family: " << this->addr.sun_family << std::endl;
		std::cout << "path: " << this->addr.sun_path << std::endl;
		std::cout << "len: " << len << std::endl;
		return ;
	}

	void set_family(int family) {
		this->addr.sun_family = family;
		return ;
	}	

	void set_path(const char *path) {
		::strncpy(this->addr.sun_path, path, sizeof(this->addr.sun_path) - 1);
		return ;
	}
};

template <>
class socket<Cgo::tcp_unix> : public Cgo::_base_socket<Cgo::tcp_unix> {
	using self = Cgo::socket<Cgo::tcp_unix>;
	using used_t = Cgo::tcp_unix;
	using base_t = Cgo::_base_socket<Cgo::tcp_unix>;
	using addr_t = Cgo::sockaddr<Cgo::tcp_unix>;
	using socket_t = int;
private:

	bool is_sockfd_open() {
		int flags = ::fcntl(sockfd, F_GETFL);
		if (flags == -1) {
			return false;
		} else {
			return true;
		}
	}
	
public:

	socket() : base_t::_base_socket() {};

	socket(const int sockfd) : base_t::_base_socket(sockfd) {}
	
	socket(const self &other) :
		base_t::_base_socket(other.sockfd)
	{}

	~socket() = default;

	socket_t init() {
		if ((this->sockfd = ::socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
			return sockfd;
		}
		return sockfd;
	}

	int bind(addr_t &addr) override {
		return ::bind(this->sockfd, (struct ::sockaddr *)&addr.get_addr(), addr.get_len());
	}

	int listen(int backlog = 3) override {
		return ::listen(sockfd, backlog);
	}

	self accept() override {
		int fd = ::accept(sockfd, NULL, NULL);
		if (fd < 0) {
			return fd;
		} else {
			self temp(fd);
			return temp;
		}
	}

	self accept(addr_t &addr) override {
		int fd = ::accept(sockfd, (struct ::sockaddr *)&addr.get_addr(), &addr.get_len());
		if (fd < 0) {
			return fd;
		} else {
			self temp(fd);
			return temp;
		}
	}

	::ssize_t recv(void *buf, ::size_t len, int flags = 0) override {
		return ::recv(this->sockfd, buf, len, flags);
	}

	::ssize_t send(const void *buf, ::size_t len, int flags = 0) override {
		return ::send(this->sockfd, buf, len, flags);
	}

	int connect(addr_t &addr) override {
		return ::connect(sockfd, (struct ::sockaddr *)&addr.get_addr(), addr.get_len());
	}

	int close() override {
		if (is_sockfd_open()) return ::close(sockfd);
		return 0;
	}
};

__NAMESPACE_Cgo_END__

// DATE: 2024-08-14
// FILENAME: CgoSocket.h
// AUTHOR: royi
// END:
#endif
