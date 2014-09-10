/*
 * Copyright (C) 2012 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */


#include "editor/tools/editor_draw_tool.h"

#include "editor/tools/editor_action_args.h"
#include "editor/tools/editor_history.h"


// TODO(unknown): Saving every action in a list isn't very efficient.
// A long list can take several seconds to undo/redo every action.
// If someone has a better idea how to do this, implement it!
void EditorDrawTool::add_action
(EditorToolAction ac, EditorActionArgs & args)
{
	args.draw_actions.push_back(new EditorToolAction(ac));
}

int32_t
EditorDrawTool::handle_click_impl(Widelands::Map& /* map */,
                                    const Widelands::World& world,
                                    Widelands::NodeAndTriangle<Widelands::Coords> /* center */,
                                    EditorInteractive& /* parent */,
                                    EditorActionArgs& args) {

	for
		(std::list<EditorToolAction *>::iterator i = args.draw_actions.begin();
	        i != args.draw_actions.end();
	        ++i)
	{
		(*i)->tool.handle_click
			(static_cast<EditorTool::ToolIndex>((*i)->i),
				(*i)->map, world, (*i)->center, (*i)->parent, *((*i)->args));
	}
	return args.draw_actions.size();
}

int32_t
EditorDrawTool::handle_undo_impl(Widelands::Map& /* map */,
                                   const Widelands::World& world,
                                   Widelands::NodeAndTriangle<Widelands::Coords> /* center */,
                                   EditorInteractive& /* parent */,
                                   EditorActionArgs& args) {
	for
		(std::list<EditorToolAction *>::reverse_iterator i = args.draw_actions.rbegin();
	        i != args.draw_actions.rend();
	        ++i)
	{
		(*i)->tool.handle_undo
		(static_cast<EditorTool::ToolIndex>((*i)->i),
			(*i)->map, world, (*i)->center, (*i)->parent, *((*i)->args));
	}
	return args.draw_actions.size();
}

EditorActionArgs EditorDrawTool::format_args_impl(EditorInteractive & parent)
{
	return EditorTool::format_args_impl(parent);
}
