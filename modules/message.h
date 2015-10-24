#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BODY_SIZE 65000

enum MessageType {Request, Reply, Unknown};

class Message {
private:
    MessageType _type;
    //int operation;
    size_t _length;
    char _body[BODY_SIZE];
    //int rpc_id;
public:
    Message();
    Message (const char *content);
    Message(MessageType type, size_t length, const char *body);
    const char *getBytes() const;
    size_t getMessagSize() const;

    void setType(MessageType type);
    MessageType getType() const;

    void setLength(size_t length);
    size_t getLength() const;

    void setBody(const char *body);
    const char *getBody() const;

    //Message(/*int operation,*/ void * p_message, size_t p_message_size/*, int p_rpc_id*/);
    //Message(char * marshalled_base64);
    //char * marshal ();
    //int getOperation ();
    //int getRPCId();
    //void * getRawMessage();
    //size_t getRawMessageSize();
    //MessageType getRawMessageType();
    //void setOperation (int _operation);
    //void setMessage (void * message, size_t message_size);
    //void setMessageType (MessageType message_type);
    //~Message();
};
#endif // MESSAGE_H
