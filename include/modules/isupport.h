/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2020 Sadie Powell <sadie@witchery.services>
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


#pragma once

namespace ISupport
{
	class EventListener;
	class EventProvider;

	/* A mapping of ISUPPORT tokens to their values. */
	typedef std::map<std::string, std::string, irc::insensitive_swo> TokenMap;
}

class ISupport::EventListener
	: public Events::ModuleEventListener
{
protected:
	EventListener(Module* mod)
		: ModuleEventListener(mod, "event/isupport")
	{
	}

public:
	virtual void OnBuildISupport(TokenMap& tokens) { }
	virtual void OnBuildClassISupport(ConnectClass::Ptr klass, TokenMap& tokens) { }
};

class ISupport::EventProvider final
	: public Events::ModuleEventProvider
{
public:
	EventProvider(Module* mod)
		: Events::ModuleEventProvider(mod, "event/isupport")
	{
	}
};