#include "player.h"

#include <iostream>
#include <ctime>

#include "httplib.h"

std::string Player::stringify() const {
  // TODO: strcat could be a better option, but names are short enough
  // that it should not matter for now.
  return "username: " + username + ", uuid: " + uuid + ", server_id: " + std::to_string(server_id);
}

void WorkerSubmitFunc(ThreadsafeQueue<Player>* queue) {
  std::cout << "Starting data posting thread\n";

  while (true) {
    Player player = queue->Dequeue();

    if (player == Player()) return;

    // submit HTML request.
    httplib::Client cli("http://137.184.17.37:80");
    httplib::Params params;
    params.emplace("username", player.username);
    params.emplace("uuid", player.uuid);
    params.emplace("connection_duration", std::to_string(player.secs_connected));

    if (auto res = cli.Post("/submit_data", params)) {
      if (res->status != 200) {
        std::cerr << "Encountered server error posting data for " << player.stringify() << "\n";
      }
    } else {
      std::cerr << "Encountered error posting data for " << player.stringify() << "\n";
    }
  }
  std::cout << "Killing data posting thread\n";
}

PlayerManager::PlayerManager() : connected_players_(), disconnected_players_(),
data_poster_(WorkerSubmitFunc, &disconnected_players_) {}

PlayerManager::~PlayerManager() {
  disconnected_players_.Kill();
  data_poster_.join();
}

void PlayerManager::AddPlayer(std::string username, std::string uuid, uint8_t server_id) {
  Player new_player {username, uuid, server_id, system_clock::now()};

  // Player already connected, this is a duplicate packet
  if (connected_players_.find(username) != connected_players_.end()) return;

  // Add player to list and log the connection.
  connected_players_[username] = (new_player);
  std::cout << "Player connected: " << new_player.stringify() << "\n";
}

void PlayerManager::DisconnectPlayer(std::string username) {
  // Ignore duplicate packets, since the player is already removed
  if (connected_players_.find(username) == connected_players_.end()) return;

  Player player = connected_players_[username];
  int seconds_connected = system_clock::duration(system_clock::now() - player.connect_time).count()
      * system_clock::period::num / system_clock::period::den;

  // Remove player from connected players list and log it.
  connected_players_.erase(username);
  std::cout << "Player disconnected: " << player.stringify() << " after " << seconds_connected << " secs.\n";

  player.secs_connected = seconds_connected;
  // Send data to server.
  disconnected_players_.Enqueue(player);
}

void PlayerManager::DisconnectAllPlayers() {
  for (const auto& pair : connected_players_) {
    DisconnectPlayer(pair.first);
  }
}