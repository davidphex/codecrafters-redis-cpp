#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>
#include <thread>

#include "CommandHandler.h"
#include "Storage.h"
#include "resp/Parser.h"
#include "resp/Serializer.h"

static void* serve_client(const int client_fd) {

  std::cout << "Serving client connection...\n";

  while (true) {

    char buffer[1024];

    read(client_fd, buffer, 1024);

    std::vector<std::string> elements = Parser::parseCommand(*buffer);

    CommandHandler commandHandler;

    commandHandler.addCommand("PING", [client_fd](const std::vector<std::string>& elements) {
      const std::string response = Serializer::serializeSimpleString("PONG");
      send(client_fd, response.data(), response.length(), 0);
    });

    commandHandler.addCommand("ECHO", [client_fd](const std::vector<std::string>& elements) {
      const std::string response = Serializer::serializeBulkString(elements.at(1));
      send(client_fd, response.data(), response.length(), 0);
    });

    commandHandler.addCommand("SET", [client_fd](const std::vector<std::string>& elements) {

      // if it has PX or EX
      if (elements.size() == 5) {
        std::chrono::milliseconds expiresIn {};
        if (elements.at(3) == "EX") {
          // EX specifies the time in seconds
          expiresIn = std::chrono::milliseconds(std::stoi(elements.at(4)) * 1000);
        } else if (elements.at(3) == "PX") {
          // PX specifies the time in milliseconds
          expiresIn = std::chrono::milliseconds(std::stoi(elements.at(4)));
        }
        Storage::set(elements.at(1), elements.at(2), expiresIn);

      } else {
        Storage::set(elements.at(1), elements.at(2));
      }

      const std::string response = Serializer::serializeSimpleString("OK");

      send(client_fd, response.data(), response.length(), 0);
    });

    commandHandler.addCommand("GET", [client_fd](const std::vector<std::string>& elements) {
      const std::optional<std::string> value = Storage::get(elements.at(1));

      if (value) {
        const std::string response = Serializer::serializeBulkString(*value);
        send(client_fd, response.data(), response.length(), 0);
      } else {
        send(client_fd, Serializer::nullBulkString.data(), Serializer::nullBulkString.length(), 0);
      }


    });

    commandHandler.run(elements);

  }

  close(client_fd);
}

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
   std::cerr << "Failed to create server socket\n";
   return 1;
  }
  
  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }
  
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(6379);
  
  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port 6379\n";
    return 1;
  }


  // Maximum queued connections
  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }

  // You can use print statements as follows for debugging, they'll be visible when running tests.
  std::cout << "Logs from your program will appear here!\n";

  while (true) {

    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);
    std::cout << "Waiting for a client to connect...\n";

    // Accept incoming connection and assign it a file descriptor
    int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t*) &client_addr_len);

    if (client_fd < 0) {
      std::cerr << "Client connection failed.\n";
    }

    std::thread client_thread(serve_client, client_fd);

    client_thread.detach();
  }

  // join all threads before closing the main socket



  close(server_fd);

  return 0;
}
