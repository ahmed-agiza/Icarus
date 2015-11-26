#ifndef NETWORKEXCEPTIONS_H
#define NETWORKEXCEPTIONS_H

#include <exception>

class NetworkException : public std::exception {
public:
	virtual const char* what() const throw() {
		return "Unknown network connection has occured.";
	}
};


class SettingsLoadException : public NetworkException {
public:
	virtual const char* what() const throw() {
		return "Failed to load settings file.";
	}
};

class ThreadCreationException : public NetworkException {
public:
	virtual const char* what() const throw() {
		return "Failed to create the thread.";
	}
};


class ThreadsLimitException : public NetworkException {
public:
	virtual const char* what() const throw() {
		return "No more available threads.";
	}
};

class InvalidOptionException : public NetworkException {
public:
	virtual const char* what() const throw() {
		return "Invalid option in settings file.";
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

class InvalidReplyException : public NetworkException {
public:
	virtual const char* what() const throw() {
		return "Invalid reply.";
	}
};

class MutexInitializationException : public NetworkException {
public:
	virtual const char* what() const throw() {
		return "Failed to initialize the mutex.";
	}
};

class CVInitializationException : public NetworkException {
public:
	virtual const char* what() const throw() {
		return "Failed to initialize the condition variable.";
	}
};




class SocketException : public NetworkException {
public:
	virtual const char* what() const throw() {
		return "Failed to create the socket.";
	}
};

class HostResolveException : public NetworkException {
public:
	virtual const char* what() const throw() {
		return "Failed to resolve hostname.";
	}
};

class ReceiveFailureException : public NetworkException {
public:
	virtual const char* what() const throw() {
		return "Failed to receive from the socket.";
	}
};

class ReceiveTimeoutException : public NetworkException {
public:
	virtual const char* what() const throw() {
		return "Receive operation timed-out.";
	}
};

class SetSendTimeoutException : public NetworkException {
public:
	virtual const char* what() const throw() {
		return "Failed to set sending timeout.";
	}
};

class SetReceiveTimeoutException : public NetworkException {
public:
	virtual const char* what() const throw() {
		return "Failed to receiving timeout.";
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
