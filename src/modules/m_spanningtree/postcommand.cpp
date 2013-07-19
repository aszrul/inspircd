/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2009 Daniel De Graaf <danieldg@inspircd.org>
 *   Copyright (C) 2007 Craig Edwards <craigedwards@brainbox.cc>
 *
 * This file is part of InspIRCd.  InspIRCd is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "inspircd.h"

#include "main.h"
#include "utils.h"
#include "treeserver.h"

void ModuleSpanningTree::OnPostCommand(Command* command, const std::vector<std::string>& parameters, LocalUser* user, CmdResult result, const std::string& original_line)
{
	if (result == CMD_SUCCESS)
		Utils->RouteCommand(NULL, command, parameters, user);
}

void SpanningTreeUtilities::RouteCommand(TreeServer* origin, CommandBase* thiscmd, const parameterlist& parameters, User* user)
{
	const std::string& command = thiscmd->name;
	RouteDescriptor routing = thiscmd->GetRouting(user, parameters);

	std::string sent_cmd = command;
	parameterlist params;

	if (routing.type == ROUTE_TYPE_LOCALONLY)
	{
		return;
	}
	else if (routing.type == ROUTE_TYPE_OPT_BCAST)
	{
		params.push_back("*");
		params.push_back(command);
		sent_cmd = "ENCAP";
	}
	else if (routing.type == ROUTE_TYPE_OPT_UCAST)
	{
		TreeServer* sdest = FindServer(routing.serverdest);
		if (!sdest)
		{
			ServerInstance->Logs->Log(MODNAME, LOG_DEFAULT, "Trying to route ENCAP to nonexistant server %s",
				routing.serverdest.c_str());
			return;
		}
		params.push_back(sdest->GetID());
		params.push_back(command);
		sent_cmd = "ENCAP";
	}
	else
	{
		Module* srcmodule = thiscmd->creator;
		Version ver = srcmodule->GetVersion();

		if (!(ver.Flags & (VF_COMMON | VF_CORE)) && srcmodule != Creator)
		{
			ServerInstance->Logs->Log(MODNAME, LOG_DEFAULT, "Routed command %s from non-VF_COMMON module %s",
				command.c_str(), srcmodule->ModuleSourceFile.c_str());
			return;
		}
	}

	std::string output_text = CommandParser::TranslateUIDs(thiscmd->translation, parameters, true, thiscmd);

	params.push_back(output_text);

	if (routing.type == ROUTE_TYPE_MESSAGE)
	{
		char pfx = 0;
		std::string dest = routing.serverdest;
		if (ServerInstance->Modes->FindPrefix(dest[0]))
		{
			pfx = dest[0];
			dest = dest.substr(1);
		}
		if (dest[0] == '#')
		{
			Channel* c = ServerInstance->FindChan(dest);
			if (!c)
				return;
			// TODO OnBuildExemptList hook was here
			CUList exempts;
			SendChannelMessage(user->uuid, c, parameters[1], pfx, exempts, sent_cmd.c_str(), origin ? origin->GetSocket() : NULL);
		}
		else if (dest[0] == '$')
		{
			DoOneToAllButSender(user->uuid, sent_cmd, params, origin);
		}
		else
		{
			// user target?
			User* d = ServerInstance->FindNick(dest);
			if (!d)
				return;
			TreeServer* tsd = BestRouteTo(d->server);
			if (tsd == origin)
				// huh? no routing stuff around in a circle, please.
				return;
			DoOneToOne(user->uuid, sent_cmd, params, d->server);
		}
	}
	else if (routing.type == ROUTE_TYPE_BROADCAST || routing.type == ROUTE_TYPE_OPT_BCAST)
	{
		DoOneToAllButSender(user->uuid, sent_cmd, params, origin);
	}
	else if (routing.type == ROUTE_TYPE_UNICAST || routing.type == ROUTE_TYPE_OPT_UCAST)
	{
		if (origin && routing.serverdest == origin->GetName())
			return;
		DoOneToOne(user->uuid, sent_cmd, params, routing.serverdest);
	}
}
