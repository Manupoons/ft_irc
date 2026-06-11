/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamaratr <mamaratr@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 11:21:04 by mamaratr          #+#    #+#             */
/*   Updated: 2026/06/11 13:18:59 by mamaratr         ###   ########.fr       */
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
};

#endif