#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "logger.h"
#include "crypto.h"

#define MAX_READ_SIZE 65000

#define SEEDER_ID "XXXXXX"
#define DEFAULT_MESSAGE_ID "NOIDMSG"

enum MessageType {
  Request = 0, // Q
  Reply = 1, // R
  Connect = 2, // C
  Accept = 3, // A
  Decline = 4, // D
  Verify = 5, //V
  Ping = 6, // P
  Pong = 7, //G
  Query = 8, //U
  Terminate = 9,
  Acknowledge = 10,
  Packet = 11, //F
  Open = 12, //O
  Close = 13, //X
  Read = 14, //B
  Write = 15, //W
  Lseek = 16, //L
  Eof = 17, //E
  Unknown = -1 // N
};

enum Encoding {
  NoEncoding = 0,
  Base64 = 1
};

class Message {
private:
  MessageType _type;
  Encoding _encoding;
  size_t _length;
  char _body[MAX_READ_SIZE];
  MessageType _letterToType(char typeLetter) const;
  char _typeToLetter(MessageType type) const;
  bool _valid;
  long _timestamp;
  char _ownerId[128];
  char _messageId[128];
  char _fullId[128];
  unsigned int _numberOfDigits (unsigned num) const;

public:
  Message(Encoding encoding = NoEncoding); //message constructor -empty
  Message(const char *content); //message constructor with content
  Message(MessageType type, const char *body, char *ownerId, char *messageId, Encoding encoding = NoEncoding, size_t encodeLength = 0); //overloading constructor

  Message (const Message &other);

  size_t writeFile(const char *fileName);
  bool isFileOperation();

  const char *getBytes() const;
  size_t getMessagSize() const;

  void setType(MessageType type);
  MessageType getType() const;

  void setLength(size_t length);
  size_t getLength() const;

  void setBody(const char *body);
  const char *getBody() const;

  const char *getOwnerId() const;
  const char *getMessageId() const;

  long getTimestamp() const;

  void setEncoding(Encoding encoding);
  Encoding getEncoding() const;

  bool isValid() const;
  bool isTerminationMessage() const;
  bool isAcknowledgeSuccess() const;

  ~Message();
};
#endif // MESSAGE_H
