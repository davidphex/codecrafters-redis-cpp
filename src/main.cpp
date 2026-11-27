#include <iostream>
#include <string>

#include "CommandRouter.h"
#include "network/Connection.h"
#include "network/Server.h"
#include "storage/StorageEngine.h"
#include "resp/Parser.h"

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  StorageEngine storage_engine;
  CommandRouter command_router(storage_engine);

  Server server(6379,
    // OnDataCallback: parse all complete RESP commands in conn.read_buffer()
    [&](Connection& conn) {
        while (auto cmd = Parser::parseCommand(conn.read_buffer())) {
            // std::nullopt reply means the command (BLPOP) took over
            // responsibility for responding later
            if (auto reply = command_router.execute(cmd.value(), conn.fd())) {
                conn.queue_response(*reply);
            }
        }
    },
    // OnTickCallback: expire blocked BLPOP clients that hit their timeout
    [&]() {
        command_router.tick();
    }
  );

  // Let the router push deferred replies (BLPOP unblocking / timeout) to any connection
  command_router.set_delivery_callback([&](int fd, const std::string& reply) {
      server.send_to_fd(fd, reply);
  });

  // Drop blocked-client entries when a connection goes away,
  // so replies are never queued to a closed or reused fd
  server.set_on_close_callback([&](int fd) {
      command_router.on_client_closed(fd);
  });

  server.run();

  return 0;
}
