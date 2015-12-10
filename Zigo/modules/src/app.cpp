#include "server.h"
#include "client_manager.h"
#include "heartbeat.h"
#include "peer.h"
#include "steganography.h"
#include <map>
using std::map;


pthread_mutex_t globalMutex;
bool heartBeatIsDisconnected = true;

HeartBeat *heartBeat;

void *heartBeatDisconnected(Thread *, void*) {
  pthread_mutex_lock(&globalMutex);
  heartBeatIsDisconnected = false;
  printf("Heart beat is disconnected!\n");
  pthread_mutex_unlock(&globalMutex);
  printf("Re-connecting to seeding server..\n");
  printf("Heartbeat start(%d).\n", heartBeat->start());
  return (void*)0;
}

int main(int argc, char const *argv[]) {
  if (argc < 5) {
    fprintf(stderr, "Usage %s <Username> <ServerPort> <SeederIP> <SeederPort>.\n", argv[0]);
    exit(1);
  }

  if (pthread_mutex_init(&globalMutex, NULL) != 0) {
    fprintf(stderr, "Mutex initialization failed.\n");
    exit(1);
  }

  const char *username = argv[1];
  uint16_t serverPort = (uint16_t) atoi(argv[2]);
  (void) serverPort;
  const char *seederIP = argv[3];
  uint16_t seederPort = (uint16_t) atoi(argv[4]);
  char appId[128];
  char command[128];
  char param[128];
  char buf1[2048];
  char buf2[2048];
  char buf3[2048];
  char stegKey[2048];
  memset(stegKey, 0, 2048);
  Crypto::generateRandomString(stegKey, 64);

  ClientManager clients;

  PeersMap peers;

  heartBeat = new HeartBeat(username, seederIP, seederPort, serverPort);
  strcpy(appId, heartBeat->getId());
  Server *server = new Server(serverPort);
  server->setStegKey(stegKey);
  const char *publicRSA = server->getPublicRSA();
  const char *privateRSA = server->getPrivateRSA();
  server->start();

  heartBeat->addDoneCallback(heartBeatDisconnected);
  printf("Heartbeat start(%d).\n", heartBeat->start());
  printf("App ID: %s\n", appId);

  while (strcmp(command, "quit") != 0) {
    memset(command, 0, 128);
    memset(param, 0, 128);
    memset(buf1, 0, 2048);
    memset(buf2, 0, 2048);
    memset(buf3, 0, 2048);

    printf("New command: ");
    scanf("%s", command);

    printf("Command: %s\n", command);
    if (strcmp(command, "reconnect") == 0) {
      printf("Re-connecting to seeding server..\n");
      printf("Heartbeat start(%d).\n", heartBeat->start());
    } else if (strcmp(command, "query") == 0) {
      printf("Query type?\n1-All\n2-Username\n3-ID\n4-Recent\n");
      scanf("%s", command);
      if (strcmp(command, "2") == 0) {
        printf("Username: ");
        scanf("%s", buf1);
        heartBeat->queryUsername(buf1);
      } else if (strcmp(command, "3") == 0) {
        printf("ID: ");
        scanf("%s", buf1);
        heartBeat->queryId(buf1);
      } else if(strcmp(command, "4") == 0) {
        heartBeat->queryRecent();
      }else {
        heartBeat->queryOnline();
      }

      heartBeat->fetchResults(buf2);

      peers = Peer::fromStringList(buf2);

      for(PeersMap::iterator it = peers.begin(); it != peers.end(); ++it) {
        Peer &peer = it->second;
        printf("Peer(%s):\nUsername: %s\n IP: %s\nPort: %u\n", peer.getId(), peer.getUsername(), peer.getPeerAddress(), peer.getPortNumber());
      }

    } else if (strcmp(command, "connect-user") == 0) {

      printf("Target User ID: ");
      scanf("%s", command);
      bool executed = false;
      for(PeersMap::iterator it = peers.begin(); it != peers.end(); ++it) {
        Peer &peer = it->second;
        if (strcmp(command, peer.getId()) == 0) {
          printf("Address: %s\n", peer.getPeerAddress());
          printf("Exists.\n");
          fflush(stdout);
          const char *ip = peer.getPeerAddress();
          uint16_t port = peer.getPortNumber();
          Client *client = clients.get(command, username, ip, port);
          client->queryKeys();
          client->fetchResults(buf1);
          if(sscanf(buf1, "%[^;]%*c%[^;];", buf2, buf3) != 2) {
            fprintf(stderr, "Invalid reply %s\n", buf1);
            continue;
          }
          printf("Steg Key: %s\n", buf2);
          peer.setStegKey(buf2);
          printf("RSA: %s\n", buf3);
          peer.setRSA(buf3);


          executed = true;
        }

      }
      if (!executed)
        printf("Peer %s does not exist in your map!\n", command);

    } else if (strcmp(command, "send-image") == 0) {
      int numberOfViews = 0;
      printf("Target User: ");
      scanf("%s", command);

      if(peers.find(command) == peers.end()) {
        printf("Peer %s does not exist in your map!\n", command);
        continue;
      }
      Peer &peer = peers[command];
      printf("Address: %s\n", peer.getPeerAddress());
      printf("Exists.\n");
      fflush(stdout);
      const char *ip = peer.getPeerAddress();
      uint16_t port = peer.getPortNumber();
      Client *client = clients.get(command, username, ip, port);

      printf("Image file: ");
      scanf("%s", buf1);
      char *imagePath = buf1;
      if (!File::exists(imagePath)) {
        printf("Image %s does not exist!\n", imagePath);
        continue;
      }
      printf("Number of views: ");
      scanf("%s", buf2);
      numberOfViews = atoi(buf2);
      char extraData[256];
      memset(extraData, 0, 256);
      sprintf(extraData, "%s;%d;%d;", appId, numberOfViews, numberOfViews);
      char *rsa = (char *) peer.getRSA();
      char *stegKey = (char *) peer.getStegKey();
      printf("Sending %s\nViews: %s\nStegkey: %s\n", imagePath, extraData, stegKey);

      char userFilePath[PATH_MAX];
      char fileName[PATH_MAX];
      char encryptedFilePath[PATH_MAX];
      char finalFilePath[PATH_MAX];
      char fileId[PATH_MAX];
      char originalHash[64];
      char coverFile[] = "cover.jpg";

      Crypto::md5HashFile(imagePath, originalHash);
      memset(userFilePath, 0, PATH_MAX);
      memset(fileName, 0, PATH_MAX);
      memset(encryptedFilePath, 0, PATH_MAX);
      memset(finalFilePath, 0, PATH_MAX);
      memset(fileId, 0, PATH_MAX);
      sprintf(userFilePath, "storage/%s/sent", peer.getId());
      File::joinPaths(encryptedFilePath, userFilePath, originalHash);
      strcat(fileId, peer.getId());
      strcat(fileId, "-");

      int rc = Steganography::encryptImage(userFilePath, imagePath, coverFile, originalHash, extraData, stegKey);
      (void) rc;

      strcat(fileId, originalHash);
      File::joinPaths(finalFilePath, userFilePath, fileId);
      File::rename(encryptedFilePath, finalFilePath);

      client->sendFile(finalFilePath, fileId);

      client->fetchResults(buf1);

      FileState operationResult = (FileState) atoi(buf1);

      printf("Opeartion result: %d\n", (int) operationResult);
    } else if (strcmp(command, "send-plain") == 0) {
      char fileId[512];
      printf("Target User: ");
      scanf("%s", command);
      if(peers.find(command) == peers.end()) {
        printf("Peer %s does not exist in your map!\n", command);
        continue;
      }
      Peer &peer = peers[command];
      printf("Address: %s\n", peer.getPeerAddress());
      printf("Exists.\n");
      fflush(stdout);
      const char *ip = peer.getPeerAddress();
      uint16_t port = peer.getPortNumber();
      Client *client = clients.get(command, username, ip, port);

      printf("Image file: ");
      scanf("%s", buf1);
      char *imagePath = buf1;
      if (!File::exists(imagePath)) {
        printf("Image %s does not exist!\n", imagePath);
        continue;
      }
      Crypto::md5HashFile(imagePath, fileId);
      client->sendFile(imagePath, fileId);

      client->fetchResults(buf1);
      printf("Results: %s\n", buf1);
    } else if (strcmp(command, "send-encrypted") == 0) {
      int numberOfViews = 0;
      printf("Target User: ");
      scanf("%s", command);

      if(peers.find(command) == peers.end()) {
        printf("Peer %s does not exist in your map!\n", command);
        continue;
      }
      Peer &peer = peers[command];
      printf("Address: %s\n", peer.getPeerAddress());
      printf("Exists.\n");
      fflush(stdout);
      const char *ip = peer.getPeerAddress();
      uint16_t port = peer.getPortNumber();
      Client *client = clients.get(command, username, ip, port);

      printf("Image file: ");
      scanf("%s", buf1);
      char *imagePath = buf1;
      if (!File::exists(imagePath)) {
        printf("Image %s does not exist!\n", imagePath);
        continue;
      }
      printf("Number of views: ");
      scanf("%s", buf2);
      numberOfViews = atoi(buf2);
      char extraData[256];
      memset(extraData, 0, 256);
      sprintf(extraData, "%s;%d;%d;", appId, numberOfViews, numberOfViews);
      char *rsa = (char *) peer.getRSA();
      char *stegKey = (char *) peer.getStegKey();
      printf("Sending %s\nViews: %s\nStegkey: %s\n", imagePath, extraData, stegKey);
      client->setPeerRSA(rsa);

      char userFilePath[PATH_MAX];
      char fileName[PATH_MAX];
      char encryptedFilePath[PATH_MAX];
      char finalFilePath[PATH_MAX];
      char fileId[PATH_MAX];
      char originalHash[64];
      char coverFile[] = "cover.jpg";

      Crypto::md5HashFile(imagePath, originalHash);
      memset(userFilePath, 0, PATH_MAX);
      memset(fileName, 0, PATH_MAX);
      memset(encryptedFilePath, 0, PATH_MAX);
      memset(finalFilePath, 0, PATH_MAX);
      memset(fileId, 0, PATH_MAX);
      sprintf(userFilePath, "storage/%s/sent", peer.getId());
      File::joinPaths(encryptedFilePath, userFilePath, originalHash);
      strcat(fileId, peer.getId());
      strcat(fileId, "-");

      int rc = Steganography::encryptImage(userFilePath, imagePath, coverFile, originalHash, extraData, stegKey);
      (void) rc;

      strcat(fileId, originalHash);
      File::joinPaths(finalFilePath, userFilePath, fileId);
      File::rename(encryptedFilePath, finalFilePath);

      client->sendEncryptedFile(finalFilePath, fileId);

      client->fetchResults(buf1);

      FileState operationResult = (FileState) atoi(buf1);

      printf("Opeartion result: %d\n", (int) operationResult);

    } else if (strcmp(command, "update-image") == 0) {
      printf("Target User: ");
      scanf("%s", command);
      if(peers.find(command) == peers.end()) {
        printf("Peer %s does not exist in your map!\n", command);
        continue;
      }
      Peer &peer = peers[command];
      printf("Address: %s\n", peer.getPeerAddress());
      printf("Exists.\n");
      fflush(stdout);
      const char *ip = peer.getPeerAddress();
      uint16_t port = peer.getPortNumber();
      Client *client = clients.get(command, username, ip, port);

      printf("Image ID: ");
      scanf("%s", buf1);
      printf("New number of views: ");
      scanf("%s", buf2);

      client->updateImage(buf1, buf2);
      client->fetchResults(buf1);
    } else if (strcmp(command, "send-temp") == 0) {
      int numberOfViews = 0;
      printf("Target User: ");
      scanf("%s", command);

      if(peers.find(command) == peers.end()) {
        printf("Peer %s does not exist in your map!\n", command);
        continue;
      }
      Peer &peer = peers[command];
      printf("Address: %s\n", peer.getPeerAddress());
      printf("Exists.\n");
      fflush(stdout);
      const char *ip = peer.getPeerAddress();
      uint16_t port = peer.getPortNumber();
      Client *client = clients.get(command, username, ip, port);

      printf("Image file: ");
      scanf("%s", buf1);
      char *imagePath = buf1;
      if (!File::exists(imagePath)) {
        printf("Image %s does not exist!\n", imagePath);
        continue;
      }
      printf("Number of views: ");
      scanf("%s", buf2);
      numberOfViews = atoi(buf2);
      char extraData[256];
      memset(extraData, 0, 256);
      sprintf(extraData, "%s;%d;%d;", appId, numberOfViews, numberOfViews);
      char *rsa = (char *) peer.getRSA();
      char *stegKey = (char *) peer.getStegKey();
      printf("Sending %s\nViews: %s\nStegkey: %s\n", imagePath, extraData, stegKey);
      client->setPeerRSA(rsa);

      char userFilePath[PATH_MAX];
      char fileName[PATH_MAX];
      char encryptedFilePath[PATH_MAX];
      char finalFilePath[PATH_MAX];
      char fileId[PATH_MAX];
      char originalHash[64];
      char coverFile[] = "cover.jpg";

      Crypto::md5HashFile(imagePath, originalHash);
      memset(userFilePath, 0, PATH_MAX);
      memset(fileName, 0, PATH_MAX);
      memset(encryptedFilePath, 0, PATH_MAX);
      memset(finalFilePath, 0, PATH_MAX);
      memset(fileId, 0, PATH_MAX);
      sprintf(userFilePath, "storage/%s/sent", peer.getId());
      File::joinPaths(encryptedFilePath, userFilePath, originalHash);
      strcat(fileId, peer.getId());
      strcat(fileId, "-");

      int rc = Steganography::encryptImage(userFilePath, imagePath, coverFile, originalHash, extraData, stegKey);
      (void) rc;

      strcat(fileId, originalHash);
      File::joinPaths(finalFilePath, userFilePath, fileId);
      File::rename(encryptedFilePath, finalFilePath);

      client->sendTempFile(finalFilePath, fileId);

      client->fetchResults(buf1);

      FileState operationResult = (FileState) atoi(buf1);

      printf("Opeartion result: %d\n", (int) operationResult);

    } else if (strcmp(command, "flush-temp") == 0) {
      printf("Target User: ");
      scanf("%s", command);
      printf("Image ID: ");
      scanf("%s", buf1);
      printf("New number of views: ");
      scanf("%s", buf2);
    } else if (strcmp(command, "view-count") == 0) {
      char imagePath[PATH_MAX], imageDir[PATH_MAX],
           imageName[PATH_MAX], decryptionPath[PATH_MAX],
           decryptionName[PATH_MAX];

      memset(imageDir, 0, PATH_MAX);
      memset(imagePath, 0, PATH_MAX);
      memset(imageName, 0, PATH_MAX);
      memset(decryptionName, 0, PATH_MAX);
      memset(decryptionPath, 0, PATH_MAX);

      strcpy(imageDir, "storage/");
      printf("Sender ID: ");
      scanf("%s", command);
      strcat(imageDir, command);
      strcpy(imagePath, imageDir);
      strcat(imagePath, "/recv");
      printf("Image ID: ");
      scanf("%s", buf1);
      strcat(imagePath, "/");
      strcat(imageName, appId);
      strcat(imageName, "-");
      strcat(imageName, buf1);
      strcat(imagePath, imageName);
      printf("Image directory: %s\n", imageDir);
      printf("Image path: %s\n", imagePath);
      if (!File::exists(imagePath)) {
        fprintf(stderr, "Image does not exist!\n");
        continue;
      }
      int rc = Steganography::getImageData(imageDir, imagePath, buf2, 2048, stegKey);
      printf("Image data: %s\n", buf2);
      if(rc) {
        fprintf(stderr, "Failed to decrypt the image.\n");
        continue;
      }
      int currentViewCount;
      int totalViewCount;
      char owner[256];
      if(sscanf(buf2, "%[^;]%*c%d;%d;", owner, &totalViewCount, &currentViewCount) != 3) {
        fprintf(stderr, "Invalid image!\n");
        File::remove(decryptionPath);
        continue;
      }
      printf("Total count: %d\nCurrent count: %d\n", totalViewCount, currentViewCount);
    } else if (strcmp(command, "client-status") == 0) {
      printf("User ID: ");
      scanf("%s", command);
      if(peers.find(command) == peers.end()) {
        printf("Peer %s does not exist in your map!\n", command);
        continue;
      }
      Peer &peer = peers[command];
      printf("Address: %s\n", peer.getPeerAddress());
      printf("Exists.\n");
      fflush(stdout);
      const char *ip = peer.getPeerAddress();
      uint16_t port = peer.getPortNumber();
      Client *client = clients.get(command, username, ip, port);
      client->pingServer();
      client->fetchResults(buf1);
      if(strcmp(buf1, "1") == 0)
        printf("Online.\n");
      else
        printf("Offline.\n");
    } else if (strcmp(command, "decrypt-image") == 0) {
      char imagePath[PATH_MAX], imageDir[PATH_MAX],
           imageName[PATH_MAX], decryptionPath[PATH_MAX],
           decryptionName[PATH_MAX];

      memset(imageDir, 0, PATH_MAX);
      memset(imagePath, 0, PATH_MAX);
      memset(imageName, 0, PATH_MAX);
      memset(decryptionName, 0, PATH_MAX);
      memset(decryptionPath, 0, PATH_MAX);

      strcpy(imageDir, "storage/");
      printf("Sender ID: ");
      scanf("%s", command);
      strcat(imageDir, command);
      strcpy(imagePath, imageDir);
      strcat(imagePath, "/recv");
      printf("Image ID: ");
      scanf("%s", buf1);
      strcat(imagePath, "/");
      strcat(imageName, appId);
      strcat(imageName, "-");
      strcat(imageName, buf1);
      strcat(imagePath, imageName);
      strcpy(decryptionName, imageName);
      strcat(decryptionName, ".decr");
      strcpy(decryptionPath, imageDir);
      strcat(decryptionPath, "/");
      strcat(decryptionPath, decryptionName);
      printf("Image directory: %s\n", imageDir);
      printf("Image path: %s\n", imagePath);
      if (!File::exists(imagePath)) {
        fprintf(stderr, "Image does not exist!\n");
        continue;
      }

      char updatedCount[32];
      int decryptRC = Steganography::decryptImage(imageDir, imagePath, decryptionName, buf2, 1024, stegKey);
      if(decryptRC) {
        fprintf(stderr, "Failed to decrypt the image.\n");
        continue;
      }
      int currentViewCount;
      int totalViewCount;
      char owner[256];
      if(sscanf(buf2, "%[^;]%*c%d;%d;", owner, &totalViewCount, &currentViewCount) != 3) {
        fprintf(stderr, "Invalid image!\n");
        File::remove(decryptionPath);
        continue;
      }
      if (currentViewCount == 0) {
        printf("You can no longer view this image.\n");
        File::remove(imagePath);
        File::remove(decryptionPath);
        continue;
      }
      printf("Current view count: %d\n", currentViewCount);
      currentViewCount--;
      sprintf(updatedCount, "%s;%d;%d;", owner, totalViewCount, currentViewCount);
      int updateRC = Steganography::updateImageData(imageDir, imagePath, updatedCount, stegKey);
      if(updateRC) {
        fprintf(stderr, "Failed to update the image!\n");
        File::remove(decryptionPath);
        continue;
      }
      //UI TO-DO Load the image in the memory.
      if (currentViewCount == 0) {
        printf("You can no longer view this image.\n");
        File::remove(imagePath);
        File::remove(decryptionPath);
        continue;
      }
      File::remove(decryptionPath);
    } else if (strcmp(command, "ls") == 0) {
      printf("Peersmap: \n");
      for(PeersMap::iterator it = peers.begin(); it != peers.end(); ++it) {
        Peer &peer = it->second;
        printf("%s:\n", it->first);
        printf("Peer(%s):\nUsername: %s\n IP: %s\nPort: %u\n", peer.getId(), peer.getUsername(), peer.getPeerAddress(), peer.getPortNumber());
        fflush(stdout);
      }
    } else if (strcmp(command, "lsr") == 0) {
      printf("RSAs: \n");
      for(PeersMap::iterator it = peers.begin(); it != peers.end(); ++it) {
        Peer &peer = it->second;
        printf("Peer(%s):\nUsername: %s\n RSA: %s\n", peer.getId(), peer.getUsername(), peer.getRSA());
      }
    } else if (strcmp(command, "lsk") == 0) {
      printf("Steganography keys: \n");
      for(PeersMap::iterator it = peers.begin(); it != peers.end(); ++it) {
        Peer &peer = it->second;
        printf("Peer(%s):\nUsername: %s\n Steganography key: %s\n", peer.getId(), peer.getUsername(), peer.getStegKey());
      }
    } else if (strcmp(command, "stat") == 0) {
      printf("Status: ");
      if (heartBeat->isConnected())
        printf("Connected.\n");
      else if (heartBeat->isConnecting())
        printf("Connecting..\n");
      else
        printf("Disconnected.\n");
    } else {
      printf("Unkown command: %s\n", command);
    }
  }
  printf("Command: %s\n", command);
  printf("Exit!\n");
  pthread_mutex_destroy(&globalMutex);
  delete heartBeat;
  delete server;
  return 0;
}
