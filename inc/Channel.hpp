/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamaratr <mamaratr@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 11:21:04 by mamaratr          #+#    #+#             */
/*   Updated: 2026/05/29 09:29:46 by mamaratr         ###   ########.fr       */
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
		std::vector<Client*> _members;
		std::vector<Client*> _operators;
		bool _inviteOnly;
		bool _topicRestricted;
		int _maxMembers;

	public:
		Channel();
		//Channel(const Channel &copy);
		//Channel &operator=(const Channel &assign);
		~Channel();
};

#endif