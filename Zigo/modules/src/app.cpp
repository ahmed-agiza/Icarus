#include "server.h"
#include "client_manager.h"
#include "heartbeat.h"
#include "peer.h"
#include "steganography.h"
#include <map>
using std::map;


pthread_mutex_t globalMutex;
bool heartBeatIsDisconnected = true;

void *heartBeatDisconnected(Thread *, void*) {
  pthread_mutex_lock(&globalMutex);
  heartBeatIsDisconnected = false;
  printf("Heart beat is disconnected!\n");
  pthread_mutex_unlock(&globalMutex);
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
  char rsa[2048];
  (void)rsa;

  ClientManager clients;

  PeersMap peers;

  HeartBeat *heartBeat = new HeartBeat(username, seederIP, seederPort, serverPort);
  strcpy(appId, heartBeat->getId());
  Server *server = new Server(serverPort);
  server->start();

  heartBeat->addDoneCallback(heartBeatDisconnected);
  printf("Heartbeat start(%d).\n", heartBeat->start());
  printf("App ID: %s\n", appId);

  while (strcmp(command, "quit") != 0) {
    memset(command, 0, 128);
    memset(param, 0, 128);
    memset(buf1, 0, 2048);
    memset(buf2, 0, 2048);

    printf("New command: ");
    scanf("%s", command);

    printf("Command: %s\n", command);
    if (strcmp(command, "reconnect") == 0) {
      printf("Re-connecting to seeding server..\n");
      printf("Heartbeat start(%d).\n", heartBeat->start());
    } else if (strcmp(command, "query") == 0) {
      printf("Query type?\n1-All\n2-Username\n3-ID\n");
      scanf("%s", command);
      if (strcmp(command, "2") == 0) {
        printf("Username: ");
        scanf("%s", buf1);
        heartBeat->queryUsername(buf1);
      } else if (strcmp(command, "3") == 0) {
        printf("ID: ");
        scanf("%s", buf1);
        heartBeat->queryId(buf1);
      } else {
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
          client->queryRSA();
          client->fetchResults(buf1);
          printf("RSA: %s\n", buf1);
          peer.setRSA(buf1);
          client->queryStegKey();
          client->fetchResults(buf2);
          printf("Steg Key: %s\n", buf2);
          peer.setStegKey(buf2);
          executed = true;
          break;
        }

      }
      if (!executed)
        printf("Peer %s does not exist in your map!\n", command);

    } else if (strcmp(command, "send-image") == 0) {
      int numberOfViews = 0;
      printf("Target User: ");
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
          (void)client;
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
          char *rsa = (char *) peer.getRSA();
          char *stegKey = (char *) peer.getStegKey();
          printf("Sending %s\nViews: %d\nStegkey: %s\nRSA: %s\n", imagePath, numberOfViews, stegKey, rsa);

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

          int rc = Steganography::encryptImage(userFilePath, imagePath, coverFile, originalHash, buf2, stegKey);
          (void) rc;

          strcat(fileId, originalHash);
          File::joinPaths(finalFilePath, userFilePath, fileId);
          File::rename(encryptedFilePath, finalFilePath);

          client->sendFile(finalFilePath, fileId);

          client->fetchResults(buf1);

          FileState operationResult = (FileState) atoi(buf1);

          printf("Opeartion result: %d\n", (int) operationResult);


          executed = true;
          break;
        }

      }
      if (!executed)
        printf("Peer %s does not exist in your map!\n", command);

    } else if (strcmp(command, "update-image") == 0) {
      printf("Target User: ");
      scanf("%s", command);
      printf("Image ID: ");
      scanf("%s", buf1);
      printf("New number of views: ");
      scanf("%s", buf2);
    } else if (strcmp(command, "open-file") == 0) {
      printf("Owner: ");
      scanf("%s", command);
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
    }else {
      printf("Unkown command: %s\n", command);
    }
  }
  printf("Exit!\n");
  pthread_mutex_destroy(&globalMutex);
  delete heartBeat;
  delete server;
  return 0;
}
