/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcarpio- <rcarpio-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 11:21:04 by mamaratr          #+#    #+#             */
/*   Updated: 2026/06/25 16:53:37 by rcarpio-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include "utils.hpp"
#include "Client.hpp"

class Channel
{
	private:
		std::string _name;
		std::string _topic;
		std::string _password;
		int 		_numberUsers;
		std::vector<Client*> _members;
		std::vector<Client*> _operators;
		std::vector<Client*> _invitedUsers;
		bool _inviteOnly;
		bool _topicRestricted;
		bool _passwordRequired;
		int _maxUsers;

	public:
		Channel(const std::string &name);
		~Channel();

		void addClient(Client* client);
		void removeClient(Client* client);
		bool hasClient(Client* client) const;
		void broadcast(const std::string& message, Client* sender) const;
		bool isOperator(Client* client) const;
		void addOperator(Client* client);
		void removeOperator(Client* client);
		bool checkJoinRules(const std::string& password) const;
		void setMode(char mode, bool add, const std::string& arg);
		std::string getModeString() const;
		void addInvitedUser(Client* client);
		bool isInvited(Client* client) const;
		int  clientCount() const;
		void removeInvited(Client *client);

		// Getters
		std::string getName() const { return _name; }
		std::string getTopic() const { return _topic; }
		std::string getPassword() const { return _password; }
		int getNumberUsers() const { return _numberUsers; }
		int getMaxUsers() const { return _maxUsers; }
		bool getInviteOnly() const { return _inviteOnly; }
		bool getTopicRestricted() const { return _topicRestricted; }
		bool getPasswordRequired() const { return _passwordRequired; }
		const std::vector<Client*>& getMembers() const { return _members; }
		const std::vector<Client*>& getOperators() const { return _operators; }
		const std::vector<Client*>& getInvitedUsers() const { return _invitedUsers; }
		std::string getNickList() const;
		
		// Setters
		void setName(const std::string& name) { _name = name; }
		void setTopic(const std::string& topic) { _topic = topic; }
		void setPassword(const std::string& password) { _password = password; }
		void setMaxUsers(int maxUsers) { _maxUsers = maxUsers; }
		void setInviteOnly(bool inviteOnly) { _inviteOnly = inviteOnly; }
		void setTopicRestricted(bool topicRestricted) { _topicRestricted = topicRestricted; }
		void setPasswordRequired(bool passwordRequired) { _passwordRequired = passwordRequired; }
};

#endif