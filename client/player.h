#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <string>
#include <unordered_map>
#include <chrono>
#include <thread>

#include "threadsafe_queue.h"

using namespace std::chrono;

/* Defines a player and all relevant information
 * we need to track
 */
struct Player {
  std::string username;
  std::string uuid;
  // Every time a player connects to a server they are assigned a unique integer id
  // which identifies them as the sender of a message, the person killed/killing, etc.
  uint8_t server_id;
  system_clock::time_point connect_time; // timestamp of player's first connection

  int secs_connected = 0; // Set once player disconnects.

  std::string stringify() const;

  bool operator==(const Player& other) {
    return this->stringify() == other.stringify();
  }
  
  bool operator!=(const Player & other) {
    return !(*this == other);
   }
};

/* Manages a list of all currently connected players, ensures no player is
 * added twice, and ultimately reports data to the database when a player
 * eventually disconnects.
 */
class PlayerManager {
public:
  PlayerManager();

  ~PlayerManager();

  void AddPlayer(std::string username, std::string uuid, uint8_t server_id);

  void DisconnectPlayer(std::string username);

  void DisconnectAllPlayers();

private:
  // Map username to player object.  A bit of data redundancy, but that's ok.
  std::unordered_map<std::string, Player> connected_players_;

  // A queue of player data for players who have disconnected and already
  // been removed from the connected_players_ map.  This queue is solely
  // used for sending their data to the server.
  ThreadsafeQueue<Player> disconnected_players_;

  // A thread which consumes the disconnected player queue and sends the relevant
  // data to the server via post request.
  std::thread data_poster_;
};

#endif