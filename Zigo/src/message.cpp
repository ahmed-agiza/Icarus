#include "message.h"


Message::Message():_type(Unknown), _length(0), _valid(false){

}

Message::Message (const char *content) {
  char typeLetter;

  memset(_body, 0, MAX_READ_SIZE);
  int readValues = sscanf(content, "L %zu T %c B %64999c", &_length, &typeLetter, _body);
  if (readValues < 3) {
    char invalidFormatMessage[LOG_MESSAGE_LENGTH];
    sprintf(invalidFormatMessage, "Invalid message format(%d).", readValues);
    Logger::error(invalidFormatMessage);
    _valid = false;
    setType(Unknown);
  } else{
    _valid = true;
    setType(_letterToType(typeLetter));
  }

}

Message::Message(MessageType type, const char *body):_type(type), _length(strlen(body)), _valid(true) {
  if (_type == Packet) {
    int fd = open(body, O_RDONLY);
    if(fd < 0){
      _valid = false;
      Logger::error("Failed to open packet file for reading.");
      throw FileOpenException();
    }
    read(fd, _body, MAX_READ_SIZE);
    _length = strlen(_body);
    printf("Buffer: %s", _body);
    close(fd);
  } else {
    setBody(body);
  }
}

Message::Message(const Message &other): _type(other._type), _length(other._length), _valid(other._valid) {
  setBody(other._body);
}


size_t Message::writeFile(const char *fileName) {
  int fd = open(fileName, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  if (fd < 0) {
    Logger::error("Failed to open packet file for writing.");
    throw FileOpenException();
  }

  size_t writtenBytes = write(fd, _body, MAX_READ_SIZE);
  close(fd);
  return writtenBytes;
}

const char *Message::getBytes() const {
  size_t bodyLen = strlen(_body);
  size_t bufferSize = bodyLen + sizeof(_length) + sizeof(_type);
  char *buffer = new char[bufferSize];
  sprintf(buffer, "L %zu\nT %c\nB %s", _length, _typeToLetter(_type), _body);
  return buffer;
}

bool Message::isValid() const {
  return _valid;
}

bool Message::isTerminationMessage() const {
  return (_type == Terminate || (strcmp(_body, "q") == 0));
}

bool Message::isAcknowledgeSuccess() const {
  return (_type == Acknowledge && strcmp(_body, "1") == 0);
}

void Message::setType(MessageType type) {
  _type = type;
}
MessageType Message::getType() const {
  return _type;
}

void Message::setLength(size_t length) {
  _length = length;
}
size_t Message::getLength() const {
  return _length;
}

size_t Message::getMessagSize() const {
  const char *tempBody = getBytes();
  size_t messageSize = strlen(tempBody) + 1;
  delete tempBody;
  return messageSize;
}

void Message::setBody(const char *body) {
  strcpy(_body, body);
}
const char *Message::getBody() const {
  return _body;
}

MessageType Message:: _letterToType(char typeLetter) const {
  if (typeLetter == 'Q')
    return Request;
  else if (typeLetter == 'R')
    return Reply;
  else if (typeLetter == 'C')
    return Connect;
  else if (typeLetter == 'A')
    return Acknowledge;
  else if (typeLetter == 'Y')
    return Accept;
  else if (typeLetter == 'P')
    return Ping;
  else if (typeLetter == 'T')
    return Terminate;
  else if (typeLetter == 'F')
    return Packet;
  else
    return Unknown;
}
char Message::_typeToLetter(MessageType type) const {
  if (type == Request)
    return 'Q';
  else if (type == Reply)
    return 'R';
  else if (type == Connect)
    return 'C';
  else if (type == Accept)
    return 'Y';
  else if (type == Acknowledge)
    return 'A';
  else if (type == Ping)
    return 'P';
  else if (type == Terminate)
    return 'T';
  else if (type == Packet)
    return 'F';
  else
    return 'U';
}

Message::~Message() {

}
