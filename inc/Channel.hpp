/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcarpio-mamaratr <rcarpio-mamaratr@stud    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 11:21:04 by mamaratr          #+#    #+#             */
/*   Updated: 2026/06/11 14:16:41 by rcarpio-mam      ###   ########.fr       */
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
		Channel();
		~Channel();

		void addClient(Client* client);
		void removeClient(Client* client);
		bool isMember(Client* client) const;
		void broadcast(const std::string& message, Client* sender=nullptr) const;
		bool isOperator(Client* client) const;
		void addOperator(Client* client);
		void removeOperator(Client* client);
		bool checkJoinRules(const std::string& password) const;
		void setMode(char mode, bool add, const std::string& arg);
		void addInvitedUser(Client* client);

		// Getters
		std::string getName() const { return _name; }
		std::string getTopic() const { return _topic; }
		std::string getPassword() const { return _password; }
		int getNumberUsers() const { return _numberUsers; }
		int getMaxUsers() const { return _maxUsers; }
		bool isInviteOnly() const { return _inviteOnly; }
		bool isTopicRestricted() const { return _topicRestricted; }
		bool isPasswordRequired() const { return _passwordRequired; }
		const std::vector<Client*>& getMembers() const { return _members; }
		const std::vector<Client*>& getOperators() const { return _operators; }
		const std::vector<Client*>& getInvitedUsers() const { return _invitedUsers; }
		
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