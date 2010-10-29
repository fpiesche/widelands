/*
 * Copyright (C) 2004-2006, 2008-2009 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef NETWORK_GGZ_H
#define NETWORK_GGZ_H

#ifdef USE_GGZ
#define HAVE_GGZ 1

#define ERRMSG "</p><p font-size=14 font-color=#ff6633 font-weight=bold>ERROR: "

#include "build_info.h"
#include "chat.h"
#include "network_lan_promotion.h"

#include <stdint.h>
#include <string>
#include <vector>

#include <limits>
#include "gamesettings.h"
#include "logic/game.h"

#ifdef WIN32
#include <winsock2.h>
#include <io.h>
#endif
#include <ggz_common.h>

#include "game_server/protocol.h"


/// A simply network player struct
struct Net_Player {
	std::string   table;
	std::string   name;
	GGZPlayerType type;
	char          stats[16];
};

struct Net_Player_Info {
	Net_Player_Info():
		playernum(-1),
		points(0),
		name(),
		tribe(),
		type(playertype_null),
		result(gamestatresult_null),
		team(0)
		{}
	int playernum;
	int points;
	std::string name;
	std::string tribe;
	WLGGZPlayerType type;
	WLGGZGameStatsResult result;
	Widelands::TeamNumber team;
	int report_time;
};

/// A MOTD struct for easier output to the chat panel
struct MOTD {
	std::string formationstr;
	std::vector<std::string> motd;

	MOTD() {}
	MOTD(std::string msg) {
		// if msg is empty -> return
		if (msg.size() < 1)
			return;

		// first char is always \n - so we remove it
		msg = msg.substr(1);
		std::string::size_type j = msg.find('\n');

		// split the message parts to have good looking texts
		for (int32_t i = 0; msg.size(); ++i) {
			if (j == std::string::npos) {
				motd.push_back(msg);
				break;
			}
			if (i == 0 && msg.size() and *msg.begin() == '<')
				formationstr = msg.substr(0, j);
			else
				motd.push_back(msg.substr(0, j));
			msg = msg.substr(j + 1);
			j = msg.find('\n');
		}
	}
};

class ggz_ggzcore;
class ggz_ggzmod;
class ggz_wlmodule;

/**
 * The GGZ implementation
 *
 * It handles all widelands related parts and does the communication with
 * the widelands_server part of ggz. The ggzcore and ggzmod handling is done
 * in \ref ggz_ggzcore and \ref ggz_ggzmod.
 */
struct NetGGZ : public ChatProvider {
	friend class ggz_ggzcore;
	friend class ggz_ggzmod;
	static NetGGZ & ref();
	~NetGGZ();

	/// process ggz data (ggzcore and ggzmod). This must be called on a regular
	/// basis. If timeout is 0 this method return imediately after all pending
	/// data is processed. Else this function wait for incomming data timeout
	/// milliseconds
	int process(int timeout = 0);
	
	/// returns the ip address of the server we joined
	char const * ip();

	/// returns true if ggz is connected to the metaserver
	bool connected();

	/// true if ggz is logged in to the metaserver
	bool logged_in();

	/// only true while logging in
	bool is_connecting();

	void statechange();

	bool updateForTables();
	bool updateForUsers();
	std::vector<Net_Game_Info> const & tables();
	std::vector<Net_Player>    const & users();

	// Include the enums to communicate with the server

	/**
	 * initialize ggz core. Set up internal strucures and set metaserver settings
	 *
	 * @param metaserv Name of the server to connect to
	 * @param nick Nickname of the playerinfo
	 * @param pwd Password of the player. Only needed if registered is true
	 * @param registered If true log in as registered player else log in as a
	 *                   guest
	 */
	bool initcore
		(const char * metaserver, const char * nick,
		 const char * pwd, bool registered);
	/// deinit ggz completely. Disconnect from server
	void deinit();

	/// Modify ggz table state to playing
	void send_game_playing();
	/// Modify ggz table state to done
	void send_game_done();

	/// Set information about the players from @ref GameSettings structure. This
	/// is transmitted later by @ref send_game_info()
	void set_players(GameSettings&);

	/// Set map name and size. This is transmitted later by @ref send_game_info()
	void set_map(std::string, int, int);

	/// Send infortmation about the game to the metaserver.
	void send_game_info();

	/**
	 * Report the result of a plyer to ggz. After a result was reported for all
	 * player the results will be transmitted to the metaserver.
	 * NOTE: This is called from the win condition script. I think here is a
	 *       possible bug. I a game is resumed from savegame and one player was
	 *       defeated earlier (and results reported) the results of this player
	 *       wont't be reported again and so game results will never be
	 *       transmitted to metaserver.                timowi, 26.08.2010
	 */
	void report_result
		(int32_t player, Widelands::TeamNumber team, int32_t points,
		 bool win, int32_t gametime,
		 const Widelands::Game::General_Stats_vector & resultvec);

	/// join a open table (a open game)
	void join(char const * tablename);
	
	/// launch a new table (open a game)
	void launch();

	/// functions for local server setup
	uint32_t max_players();
	/// sets the maximum number of players that may be in the game
	void set_local_maxplayers(uint32_t mp) {
		//std::cout << "set seats: "<< mp << std::endl;
		tableseats = mp;
	}

	/// sets the servername shown in the games list
	void set_local_servername(std::string const & name) {
		servername = name.empty() ? "WL-Default" : name;
		servername += " (";
		servername += build_id();
		servername += ')';
	}

	/// ChatProvider: sends a message via GGZnetwork.
	void send(std::string const &);

	/// ChatProvider: adds the message to the message list and calls parent.
	void receive(ChatMessage const & msg) {
		messages.push_back(msg);
		ChatProvider::send(msg);
	}

	/// ChatProvider: returns the list of chatmessages.
	std::vector<ChatMessage> const & getMessages() const {
		return messages;
	}

	/// Called when a message is received via GGZnetwork.
	void recievedGGZChat(void const * cbdata);

	/// Adds a GGZchatmessage in selected format to the list of chatmessages.
	void formatedGGZChat
		(std::string const &, std::string const &,
		 bool system = false, std::string recipient = std::string());

	bool set_spectator(bool spec);

	bool ggz_mode();

	bool is_host() { return false; }
	
	std::string playername();

	ggz_ggzcore & core() {
		if (m_ggzcore)
			return *m_ggzcore;
		throw wexception ("Accessed ggz core object but it does not exist");
	}
	ggz_ggzmod & ggzmod() {
		if (m_ggzmod)
			return *m_ggzmod;
		throw wexception ("Accessed ggzmod object but it does not exist");
	}
	ggz_wlmodule & wlmodule() {
		if (m_wlmodule)
			return *m_wlmodule;
		throw wexception ("Accessed ggz wlmodule object but it does not exist");
	}
	
private:
	NetGGZ();
	NetGGZ(const NetGGZ &)
		{ throw wexception("Tried to copy NetGGZ class. This is not allowd"); }
	NetGGZ & operator= (const NetGGZ & old)
		{ throw wexception("Tried to copy NetGGZ class. This is not allowd"); }

	/// Transmit game statistics to metaserver. This is called from \ref
	/// report_result() when all player have a result.
	void send_game_statistics
		(int32_t gametime,
		 const Widelands::Game::General_Stats_vector & resultvec);


	/// true if ggz is used
	// bool use_ggz;

	char * server_ip_addr;

	std::string username;
	std::string servername;
	uint32_t tableseats;
	/// If true widelands was started 
	bool m_started_from_ggzclient;

	std::vector<Net_Player_Info> playerinfo;
	MOTD motd;

	/// stores information for send_game_info()
	//@{
	std::string mapname;
	int map_w, map_h;
	WLGGZGameType win_condition;
	//@}

	/// The chat messages
	std::vector<ChatMessage> messages;

	ggz_ggzcore * m_ggzcore;
	ggz_ggzmod * m_ggzmod;
	ggz_wlmodule * m_wlmodule;
};

#endif //USE_GGZ

#endif //NETWORK_GGZ_H
