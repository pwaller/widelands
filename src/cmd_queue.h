/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#ifndef __S__CMD_QUEUE_H
#define __S__CMD_QUEUE_H

// Define here all the possible users
#define SENDER_MAPOBJECT 0
#define SENDER_PLAYER1 1 // those are just place holder, a player can send commands with
#define SENDER_PLAYER2 2 // it's player number 
#define SENDER_PLAYER3 3 
#define SENDER_PLAYER4 4 
#define SENDER_PLAYER5 5 
#define SENDER_PLAYER6 6 
#define SENDER_PLAYER7 7
#define SENDER_PLAYER8 8

// ---------------------- BEGINN OF CMDS ----------------------------------
enum {
   UNUSED = 0,
   CMD_ACT		// arg1 = serialnum
};
// ---------------------- END    OF CMDS ----------------------------------

// 
// This is finally the command queue. It is fully widelands specific,
// it needs to know nearly all modules.
//
class Game;

class Cmd_Queue {
	//
	// This struct defines the commands, which are possible
	//
	// [I must've accidently deleted a comment about different access rights
	//  here; either way, filtering commands at the network level should
	//  really be enough]
	//
	struct Cmd {
		int time; // scheduled time of execution
		char sender;
		int cmd;
		int arg1;
		int arg2;
		void *arg3; // pointer to malloc()ed memory
	};
	struct CmdCompare {
	public:
		bool operator() (const Cmd& c1, const Cmd& c2) {
			return c1.time > c2.time;
		}
	};
	typedef std::priority_queue<Cmd, vector<Cmd>, CmdCompare> queue_t;
	
   public:
      Cmd_Queue(Game *g);
      ~Cmd_Queue(void);

		void queue(int time, char sender, int cmd, int arg1=0, int arg2=0, void *arg3=0);
      int run_queue(int interval);
   
		inline int get_time() { return m_time; }
		   
   private:
		void exec_cmd(const Cmd *c);
		
      Game *m_game;
		queue_t m_cmds;
		int m_time;	
};

#endif // __S__CMD_QUEUE_H 
