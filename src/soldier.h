/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#ifndef __S__SOLDIER_H
#define __S__SOLDIER_H

#include "worker.h"

// Constants used to launch attacks
#define WEAKEST   0
#define STRONGEST 1

class Editor_Game_Base;
class AttackController;
class RGBColor;

#define HP_FRAMECOLOR RGBColor(255,255,255)

struct Soldier_Descr : public Worker_Descr {
	Soldier_Descr(const Tribe_Descr &, const std::string & soldier_name);
	virtual ~Soldier_Descr(void);

   virtual Worker_Type get_worker_type(void) const { return Worker_Descr::SOLDIER; }

   virtual void load_graphics(void);

	uint get_max_hp_level          () const {return m_max_hp_level;}
	uint get_max_attack_level      () const {return m_max_attack_level;}
	uint get_max_defense_level     () const {return m_max_defense_level;}
	uint get_max_evade_level       () const {return m_max_evade_level;}

	uint get_min_hp                () const {return m_min_hp;}
	uint get_max_hp                () const {return m_max_hp;}
	uint get_min_attack            () const {return m_min_attack;}
	uint get_max_attack            () const {return m_max_attack;}
	uint get_defense               () const {return m_defense;}
	uint get_evade                 () const {return m_evade;}

	uint get_hp_incr_per_level     () const {return m_hp_incr;}
	uint get_attack_incr_per_level () const {return m_attack_incr;}
	uint get_defense_incr_per_level() const {return m_defense_incr;}
	uint get_evade_incr_per_level  () const {return m_evade_incr;}

	uint get_hp_level_pic     (const uint level) const
	{assert(level <= m_max_hp_level);      return m_hp_pics     [level];}
	uint get_attack_level_pic (const uint level) const
	{assert(level <= m_max_attack_level);  return m_attack_pics [level];}
	uint get_defense_level_pic(const uint level) const
	{assert(level <= m_max_defense_level); return m_defense_pics[level];}
	uint get_evade_level_pic  (const uint level) const
	{assert(level <= m_max_evade_level);   return m_evade_pics  [level];}



	uint get_rand_anim(const char * const name) const;
protected:
	virtual Bob * create_object() const;
	virtual void parse(const char *directory, Profile *prof, const EncodeData *encdata);

   // Start values
   uint m_min_hp;
   uint m_max_hp;
   uint m_min_attack;
   uint m_max_attack;
   uint m_defense;
   uint m_evade;

   // per level increases
   uint m_hp_incr;
   uint m_attack_incr;
   uint m_defense_incr;
   uint m_evade_incr;

   // max levels
   uint m_max_hp_level;
   uint m_max_attack_level;
   uint m_max_defense_level;
   uint m_max_evade_level;

   // Level pictures
   std::vector<uint> m_hp_pics;
   std::vector<uint> m_attack_pics;
   std::vector<uint> m_evade_pics;
   std::vector<uint> m_defense_pics;
   std::vector<std::string> m_hp_pics_fn;
   std::vector<std::string> m_attack_pics_fn;
   std::vector<std::string> m_evade_pics_fn;
   std::vector<std::string> m_defense_pics_fn;
};

class Soldier : public Worker {
   friend class Widelands_Map_Bobdata_Data_Packet; // Writes this to disk
	MO_DESCR(Soldier_Descr);

public:
	Soldier(const Soldier_Descr &);
	virtual ~Soldier();

   virtual void init(Editor_Game_Base*);

	void set_level
		(const uint hp, const uint attack, const uint defense, const uint evade);
	void set_hp_level(const uint);
	void set_attack_level(const uint);
	void set_defense_level(const uint);
	void set_evade_level(const uint);
	uint get_level (const tAttribute at);
	uint get_hp_level     () const {return m_hp_level;}
	uint get_attack_level () const {return m_attack_level;}
	uint get_defense_level() const {return m_defense_level;}
	uint get_evade_level  () const throw () {return m_evade_level;}

	virtual Worker_Descr::Worker_Type get_worker_type() const throw ()
	{return descr().get_worker_type();}

   /// Draw this soldier
	virtual void draw
		(const Editor_Game_Base &, RenderTarget &, const Point) const;

   // Information function from description
	uint get_max_hp_level () const throw ()
	{return descr().get_max_hp_level();}
	uint get_max_attack_level () const throw ()
	{return descr().get_max_attack_level();}
	uint get_max_defense_level() const throw ()
	{return descr().get_max_defense_level();}
	uint get_max_evade_level  () const throw ()
	{return descr().get_max_evade_level();}

   // information functions
	uint get_current_hitpoints() const {return m_hp_current;}
	uint get_max_hitpoints    () const {return m_hp_max;}
	uint get_min_attack       () const {return m_min_attack;}
	uint get_max_attack       () const {return m_max_attack;}
	uint get_defense          () const {return m_defense;}
	uint get_evade            () const {return m_evade;}

   // get pictures
	uint get_hp_level_pic     () const
	{return descr().get_hp_level_pic     (m_hp_level);}
	uint get_attack_level_pic () const
	{return descr().get_attack_level_pic (m_attack_level);}
	uint get_defense_level_pic() const
	{return descr().get_defense_level_pic(m_defense_level);}
	uint get_evade_level_pic  () const
	{return descr().get_evade_level_pic  (m_evade_level);}

   /// Sets a random animation of desired type and start playing it
	void start_animation
		(Editor_Game_Base*, const char * const animname, const uint time);

	/// Heal quantity of hit points instantly
	void heal (const uint);

   /// Damage quantity of hit points
	void damage (const uint);

	///  This are used to control Requests (only called by Warehouse)
	bool is_marked () const throw () {return m_marked;}
	void mark (const bool b) {m_marked = b;}
public: // Worker-specific redefinitions
	virtual void start_task_gowarehouse();

   void startTaskMoveToBattle(Game*, Flag*, Coords);
   void startTaskMoveHome(Game*);

   void log_general_info(Editor_Game_Base* egbase);

   inline void set_attack_ctrl(AttackController* ctrl) { m_attack_ctrl = ctrl; };

private:
   void moveToBattleUpdate (Game*, State*);
   void moveToBattleSignal (Game*, State*);

   void moveHomeUpdate (Game*, State*);
   void moveHomeSignal (Game*, State*);
protected:
   static Task taskMoveToBattle;
   static Task taskMoveHome;

private:
   // Private data
   uint m_hp_current;
   uint m_hp_max;
   uint m_min_attack;
   uint m_max_attack;
   uint m_defense;
   uint m_evade;

   uint m_hp_level;
   uint m_attack_level;
   uint m_defense_level;
   uint m_evade_level;

	bool m_marked;

   AttackController* m_attack_ctrl;
};


#endif // __S__SOLDIER_H
