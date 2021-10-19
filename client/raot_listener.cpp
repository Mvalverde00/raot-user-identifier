#define TINS_STATIC
#include <tins/tins.h>
#include <iostream>
#include <unordered_map>
#include <functional>

#include "byte_reader.h"
#include "util.h"
#include "player.h"
#include "pvp.h"

using namespace Tins;
typedef std::function<ByteReader(ByteReader)> handler;

// To fit with new hypothesis that command ids are ints (4 bytes), we cut off
// the command a bit, although it does seem like the full 8 byte sequence
// identifies a player joining...
//byte_array player_join = { 0x3a, 0x3b, 0x07, 0x1e, 0x13, 0xb3, 0x4f, 0x87};
byte_array player_join = { 0x3a, 0x3b, 0x07, 0x1e };

//byte_array player_join = { 0x00, 0x02, 0x64, 0x40 };

// ae 9b 87 3a actually indicates any message sent by the server, such
// as "<player x> is joining/leaving/switching teams/etc."  However
// this is currently the most reliable method I have found to detect
// player disonnect
byte_array player_disconnect = { 0xae, 0x9b, 0x87, 0x3a};

// A message was sent in chat and we received it.
byte_array received_message = { 0xfb, 0x80, 0x1d, 0xb8 };

byte_array server_id_change = { 0x00, 0x02, 0x64, 0x40 };

// A player killing another player.
byte_array kill_event = { 0xa5, 0x23, 0x52, 0x06 };

// A player parrying another player
byte_array parry_event = { 0xe7, 0x30, 0xd2, 0x07 };

PlayerManager connected_player_manager;
uint8_t curr_server_id = 0;

ByteReader on_player_join(ByteReader reader) {

  // Strip off command id
  int cmd_id = reader.ReadInt32();
  reader.ReadInt32();

  // The next 24 bytes are junk, all 0s.
  reader.ReadBytes(24);

  // The next 24 bytes are also mostly junk, although one byte contains
  // the length of the player's name (null-terminated).
  std::vector<byte> unknown = reader.ReadBytes(24);
  uint8_t name_len = unknown[22] - 1;  // Correct for null-termination.

  // The next name_len bytes contain the username.  However, if for some reason
  // the name were to have length 0, the code below might break...
  if (name_len == 0) {
    std::cerr << "Player joined with name consisting of 0 characters, "
              << "segfault possible.\n";
  }
  std::vector<byte> name_bytes = reader.ReadBytes(name_len);
  std::string username(reinterpret_cast<char const*>(name_bytes.data()), name_len);

  // After the username there are two bytes, b"0030" which we also skip
  reader.ReadBytes(2);

  // The next 32 bytes encode the player's UUID as a string
  std::vector<byte> uuid_bytes = reader.ReadBytes(32);
  std::string uuid(reinterpret_cast<char const*>(uuid_bytes.data()), 32);

  // Track the player.
  connected_player_manager.AddPlayer(username, uuid, curr_server_id);

  // Return the remaining bytes
  return reader;
}

ByteReader on_player_disconnect(ByteReader reader) {
  // Strip off command id
  reader.ReadInt32();

  reader.ReadByte();
  uint32_t stuff = reader.ReadPackedUInt32();
  std::string s = reader.ReadString();

  std::cout << stuff << ", " << s << "\n";

  return reader;

  // There is a variable length bit of data before the actual message begins.
  // This is always terminated by a null character.  Strip it out before proceeding.


  // NOT SURE WHAT TO DO HERE??
  //std::string unknown(reinterpret_cast<char const*>(&data[0]));
  //data = byte_array(data.begin() + unknown.length() + 1, data.end());
  //message_len -= (unknown.length() + 1);

  /*
  std::string message(reinterpret_cast<char const*>(&data[0]), message_len);
  // Technically we are capturing _all_ server-sent messages.  such as
  // switching teams, joining, etc.  Skip these
  if (message.find("left the server.") == std::string::npos)
    return byte_array(data.begin() + message_len, data.end());

  std::cout << message << "\n";

  // When a player disconnects, a message of the form "<username> left the server." is
  // broadcasted.  This is all we have to identify the person that left.
  // We can calculate username length as message_len minus the length of the fixed part of the message.
  uint8_t username_len = message_len - 17;
  std::string username(reinterpret_cast<char const*>(&data[0]), username_len);

  connected_player_manager.DisconnectPlayer(username);

  // Return remainder after leaving message.  Might be able to trim a bit more of this out,
  // not quite sure.
  return byte_array(data.begin() + message_len, data.end());
  */
}

ByteReader on_receive_message(ByteReader reader) {
  // Strip off command id
  reader.ReadInt32();

  reader.ReadByte();
  reader.ReadPackedUInt64();
  uint32_t client_id = reader.ReadPackedUInt32();
  std::string s = reader.ReadString();
  bool to_team = reader.ReadByte();

  std::cout << client_id << ": " << s << "\n";
  std::cout << " to team:" << to_team << "\n";
  //std::vector<byte> message_bytes = reader.ReadBytes();
  //bool is_team = reader.ReadByte();

  return reader;

  /*
  // There is a variable length bit of data before the actual message begins.
  // This is always terminated by a null character.  Strip it out before proceeding.
  std::string unknown(reinterpret_cast<char const*>(&data[0]));
  uint8_t server_id = unknown[unknown.size() - 2];
  int message_len = unknown[unknown.size() - 1] - 1;
  data = byte_array(data.begin() + unknown.length() + 1, data.end());

  std::string message(reinterpret_cast<char const*>(&data[0]), message_len);
  bool team_chat = data[message.length()];

  std::string prefix = team_chat ? "(Team)" : "(Global)";
  std::cout << prefix << "  " << int(server_id) << ": " <<  message <<  "\n";

  return byte_array(data.begin() + message_len + 1, data.end());*/

}

ByteReader on_kill_event(ByteReader reader) {
  // Strip command ID
  reader.ReadInt32();

  uint8_t killer_server_id = reader.ReadByte();
  uint8_t unknown1 = reader.ReadByte(); // unknown
  uint8_t killed_server_id = reader.ReadByte();
  uint8_t unknown2 = reader.ReadByte(); // unknown
  int attack = reader.ReadPackedInt32();

  std::string attack_name = attack_names[AttackType(attack)];
  std::cout << int(killer_server_id) << " killed " << int(killed_server_id)
            << " with " << attack_name << "\n";

  return reader;
}

ByteReader on_parry_event(ByteReader reader) {
  // Strip command ID
  reader.ReadInt32();

  uint8_t initiator_server_id = reader.ReadByte();
  reader.ReadByte(); // unknown
  uint8_t parryer_server_id = reader.ReadByte();
  reader.ReadByte(); // unknown
  int attack = reader.ReadPackedInt32();

  std::string attack_name = attack_names[AttackType(attack)];
  std::cout << int(initiator_server_id) << "'s " << attack_name << " was parried by "
            << int(parryer_server_id) << "\n";

  return reader;
}

ByteReader on_server_id_change(ByteReader reader) {
  reader.ReadInt32();

  return reader;
}

ByteReader noop(ByteReader reader) {
  // Strip off 1-byte since command was not found.
  reader.ReadByte();

  return reader;
}


std::unordered_map<byte_array, handler> handlers{
  {player_join, on_player_join},
  {player_disconnect, on_player_disconnect},
  {received_message, on_receive_message},
  {server_id_change, on_server_id_change},
  {kill_event, on_kill_event},
  {parry_event, on_parry_event},
};


int main(int argc, char* argv[]) {
  const auto interface = Tins::NetworkInterface::default_interface();

  Tins::Sniffer sniffer(interface.name());

  sniffer.set_filter("udp port 15937");

  std::cout << "starting\n";
  while (true) {
    PDU* pkt = sniffer.next_packet();

    UDP* udp = pkt->find_pdu<UDP>();
    RawPDU* raw = pkt->find_pdu<RawPDU>();

    if (udp == NULL || raw == NULL) continue;

    byte_array data = raw->payload();

    ByteReader reader(data);

    // A single 0x0d byte means the client is disconnecting from the server.
    // Thus, we cannot collect any more data on players in the lobby,
    // so we must disconnect.
    //if (data.size() == 1 && data[0] == 0x0d) { TEMP
    if (reader.Size() == 1 && reader.PeakByte() == 0x0d) {
      IP* ip = pkt->find_pdu<IP>();
      std::string dst = ip->dst_addr().to_string();
      // If it leads with 192.168, it was sent to us.  This is important
      // since if we are hosting a server, we may receive other people's
      // disconnect requests.
      if (dst.find("192.168") == 0) {
        std::cout << "Client disconnected from server\n";
        connected_player_manager.DisconnectAllPlayers();
        //data = {}; TEMP
      }
    }
    
    while (reader.Size() >= 4) {
      int cmd_id = reader.PeakInt32();
      byte_array key(4);
      std::memcpy(key.data(), &cmd_id, 4);

      handler func = GetWithDefault<byte_array, handler>(handlers, key, noop);
      reader = func(reader);
    }
  }

  return 0;
}