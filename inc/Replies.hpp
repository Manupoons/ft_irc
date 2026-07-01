/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Replies.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamaratr <mamaratr@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/05 09:23:40 by mamaratr          #+#    #+#             */
/*   Updated: 2026/07/01 09:31:41 by mamaratr         ###   ########.fr       */
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

#define RPL_TOPIC(srv, nick, ch, topic) \
	":" + srv + " 332 " + nick + " " + ch + " :" + topic + "\r\n"

#define RPL_NOTOPIC(srv, nick, ch) \
	":" + srv + " 331 " + nick + " " + ch + " :No topic is set\r\n"

#define RPL_NAMREPLY(srv, nick, ch, nicks) \
	":" + srv + " 353 " + nick + " = " + ch + " :" + nicks + "\r\n"

#define RPL_ENDOFNAMES(srv, nick, ch) \
	":" + srv + " 366 " + nick + " " + ch + " :End of /NAMES list\r\n"

#define RPL_CHANNELMODEIS(srv, nick, ch, modes) \
	":" + srv + " 324 " + nick + " " + ch + " " + modes + "\r\n"

#define ERR_NOSUCHCHANNEL(srv, nick, ch) \
	":" + srv + " 403 " + nick + " " + ch + " :No such channel\r\n"

#define ERR_NOTONCHANNEL(srv, nick, ch) \
	":" + srv + " 442 " + nick + " " + ch + " :You're not on that channel\r\n"

#define ERR_USERONCHANNEL(srv, nick, target, ch) \
	":" + srv + " 443 " + nick + " " + target + " " + ch + " :is already on channel\r\n"

#define ERR_BADCHANNELKEY(srv, nick, ch) \
	":" + srv + " 475 " + nick + " " + ch + " :Cannot join channel (+k)\r\n"

#define ERR_CHANNELISFULL(srv, nick, ch) \
	":" + srv + " 471 " + nick + " " + ch + " :Cannot join channel (+l)\r\n"

#define ERR_INVITEONLYCHAN(srv, nick, ch) \
	":" + srv + " 473 " + nick + " " + ch + " :Cannot join channel (+i)\r\n"

#define ERR_CHANOPRIVSNEEDED(srv, nick, ch) \
	":" + srv + " 482 " + nick + " " + ch + " :You're not channel operator\r\n"

#define ERR_USERNOTINCHANNEL(srv, nick, target, ch) \
	":" + srv + " 441 " + nick + " " + target + " " + ch + " :They aren't on that channel\r\n"

//── PRIVMSG ──

#define ERR_NOSUCHNICK(srv, nick, target) \
	":" + srv + " 401 " + nick + " " + target + " :No such nick/channel\r\n"

#define ERR_NORECIPIENT(srv, nick) \
	":" + srv + " 411 " + nick + " :No recipient given\r\n"

#define ERR_NOTEXTTOSEND(srv, nick) \
	":" + srv + " 412 " + nick + " :No text to send\r\n"

//── MODE ──

#define ERR_UNKNOWNMODE(srv, nick, c, ch) \
	":" + srv + " 472 " + nick + " " + std::string(1, c) + " :is unknown mode char to me for " + ch + "\r\n"

#define ERR_UMODEUNKNOWNFLAG(srv, nick) \
	":" + srv + " 501 " + nick + " :Unknown MODE flag\r\n"

//── KICK / INVITE / TOPIC ──

#define RPL_INVITING(srv, nick, target, ch) \
	":" + srv + " 341 " + nick + " " + target + " " + ch + "\r\n"

//── WHO ──

#define RPL_WHOREPLY(srv, req, ch, user, host, s, nick, flags, real) \
	":" + srv + " 352 " + req + " " + ch + " " + user + " " + host + " " + s + " " + nick + " " + flags + " :0 " + real + "\r\n"
	
#define RPL_ENDOFWHO(srv, nick, mask) \
	":" + srv + " 315 " + nick + " " + mask + " :End of /WHO list\r\n"

//── GENERIC ──

#define ERR_CMDNOTFOUND(srv, cmd) \
	":" + srv + " 421 * " + cmd + " :Unknown command\r\n"

#define RPL_PARROT(srv, nick) \
	":" + srv + " NOTICE " + nick + " :                    .cccccccc.                \r\n" + \
	":" + srv + " NOTICE " + nick + " :              .,,,;;cc:cccccc:;;,.            \r\n" + \
	":" + srv + " NOTICE " + nick + " :            .cdxo;..,::cccc::,..;l.           \r\n" + \
	":" + srv + " NOTICE " + nick + " :           ,do:,,:c:coxxdllll:;,';:,.         \r\n" + \
	":" + srv + " NOTICE " + nick + " :         .cl;.,oxxc'.,cc,.';;;'...oNc         \r\n" + \
	":" + srv + " NOTICE " + nick + " :         ;Oc..cxxxc'.,c;..;lll;...cO;         \r\n" + \
	":" + srv + " NOTICE " + nick + " :       .;;',:ldxxxdoldxc..;lll:'...'c,        \r\n" + \
	":" + srv + " NOTICE " + nick + " :       ;c..cxxxxkxxkxxxc'.;lll:'','.cdc.      \r\n" + \
	":" + srv + " NOTICE " + nick + " :     .c;.;odxxxxxxxxxxxd;.,cll;.,l:.'dNc      \r\n" + \
	":" + srv + " NOTICE " + nick + " :    .:,''ccoxkxxkxxxxxxx:..,:;'.:xc..oNc      \r\n" + \
	":" + srv + " NOTICE " + nick + " :  .lc,.'lc':dxxxkxxxxxxxol,...',lx:..dNc      \r\n" + \
	":" + srv + " NOTICE " + nick + " : .:,',coxoc;;ccccoxxxxxxxxo:::oxxo,.cdc.      \r\n" + \
	":" + srv + " NOTICE " + nick + " :.;':;.'oxxxxxc''''';cccoxxxxxxxxxxxc..oc      \r\n" + \
	":" + srv + " NOTICE " + nick + " :,do:'..,:llllll:;;;;;;,..,;:lllllllll;..oc    \r\n" + \
	":" + srv + " NOTICE " + nick + " :cNo.....................................oc    \r\n"

#endif