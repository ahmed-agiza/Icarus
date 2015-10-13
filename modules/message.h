#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "network_exceptions.h"

enum MessageType {Request, Reply};

class Message {
private:
    MessageType _type;
    //int operation;
    size_t _length;
    char _body[65000];
    //int rpc_id;
public:
    Message (char *content);
    Message(MessageType type, size_t length, char *body);
    char *getBytes() const;

    void setType(MessageType type);
    MessageType getType() const;

    void setLength(size_t length);
    size_t getLength() const;

    void setBody(char *body);
    const char *getBody() const;

    //Message(/*int operation,*/ void * p_message, size_t p_message_size/*, int p_rpc_id*/);
    //Message(char * marshalled_base64);
    //char * marshal ();
    //int getOperation ();
    //int getRPCId();
    //void * getMessage();
    //size_t getMessageSize();
    //MessageType getMessageType();
    //void setOperation (int _operation);
    //void setMessage (void * message, size_t message_size);
    //void setMessageType (MessageType message_type);
    //~Message();
};
#endif // MESSAGE_H
