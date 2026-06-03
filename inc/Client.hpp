/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcarpio-mamaratr <rcarpio-mamaratr@stud    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 11:20:40 by mamaratr          #+#    #+#             */
/*   Updated: 2026/06/03 12:06:15 by rcarpio-mam      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "utils.hpp"

class Client
{
	private:
		int _socket;
		std::string _nickname;
		std::string _username;
		bool _isRegistered;
		std::string _buffer;

	public:
		Client();
		~Client();
};

#endif