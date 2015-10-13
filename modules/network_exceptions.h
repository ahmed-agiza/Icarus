#ifndef NETWORKEXCEPTIONS_H
#define NETWORKEXCEPTIONS_H

#include <exception>

class NetworkException : public std::exception {
public:
	virtual const char* what() const throw() {
		return "Unknown network connection has occured.";
	}
};

class HostnameException : public NetworkException {
public:
	virtual const char* what() const throw() {
		return "Failed to resolve hostname.";
	}
};

class UnkownMessageTypeException : public NetworkException {
public:
	virtual const char* what() const throw() {
		return "Unknown message type.";
	}
};

class SocketException : public NetworkException {
public:
	virtual const char* what() const throw() {
		return "Failed to create the socket.";
	}
};


class BindingException : public SocketException {
public:
	virtual const char* what() const throw() {
		return "Failed to bind the socket.";
	}
};

class ServerBindingException : public BindingException {
public:
	virtual const char* what() const throw() {
		return "Error binding server socket.";
	}
};

class ClientBindingException : public BindingException {
public:
	virtual const char* what() const throw() {
		return "Error binding client socket.";
	}
};







#endif
