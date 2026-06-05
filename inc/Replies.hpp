/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Replies.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamaratr <mamaratr@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/05 09:23:40 by mamaratr          #+#    #+#             */
/*   Updated: 2026/06/05 10:02:30 by mamaratr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REPLIES_HPP
# define REPLIES_HPP

#include "utils.hpp"

inline std::string _itos(int n)
{
	std::ostringstream oss;
	oss << n;
	return oss.str();
}

//── Welcome sequence ──

#define RPL_WELCOME(srv, nick, user, host, real) \ 
	":" + srv + " 001 " + nick + " :Welcome to the irc network " + nick + "!" + user + "@" + host + "\r\n"

#define RPL_YOURHOST(srv, nick) \
	":" + srv + " 002 " + nick + " :Your host is " + srv + ", running ft_irc\r\n"

#define RPL_CREATED(srv, nick) \
	":" + srv + " 003 " + nick + " :This server was created now\r\n"

#define RPL_MYINFO(srv, nick) \
	":" + srv + " 004 " + nick + " " + srv + " irc o itklo\r\n"

//── MOTD ──
	
#define RPL_MOTDSTART(srv, nick) \
	":" + srv + " 375 " + nick + " :- " + srv + " Message of the day -\r\n"

#define RPL_MOTD(srv, nick, line) \
	":" + srv + " 372 " + nick + " :- " + line + "\r\n"

#define RPL_ENDOFMOTD(srv, nick) \
	":" + srv + " 376 " + nick + " :End of /MOTD command\r\n"

//── NICK / USER ──

#define ERR_NONICKNAMEGIVEN(srv, nick) \
	":" + srv + " 431 " + nick + " :No nickname given\r\n"

#define ERR_ERRORNICKNAME(srv, nick, bad) \
	":" + srv + " 432 " + nick + " " + bad + " :Erroneous nickname\r\n"

#define ERR_NICKNAMEINUSE(srv, nick) \
	":" + srv + " 433 * " + nick + " :Nickname is already in use\r\n"

#define ERR_ALREADYREGISTERED(srv, nick) \
	":" + srv + " 462 " + nick + " :Already registered\r\n"

//── AUTH ──

#define ERR_NOTREGISTERED(srv) \
	":" + srv + " 451 * :You have not registered\r\n"

#define ERR_PASSWDMISMATCH(srv) \
	":" + srv + " 464 * :Password incorrect\r\n"

#define ERR_NEEDMOREPARAMS(srv, nick, cmd) \
	":" + srv + " 461 " + nick + " " + cmd + " :Not enough parameters\r\n"

//── CHANNEL ──


//── PRIVMSG ──


//── MODE ──


//── KICK / INVITE / TOPIC ──


//── WHO ──


//── GENERIC ──

#endif