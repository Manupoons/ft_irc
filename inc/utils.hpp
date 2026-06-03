/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamaratr <mamaratr@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 11:37:52 by mamaratr          #+#    #+#             */
/*   Updated: 2026/06/03 11:54:08 by mamaratr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

#include <iostream>
#include <sstream>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <fcntl.h>

#include <poll.h>

#include <csignal>

#include <cstdlib>

#include <cstring>
#include <string>

#include <iomanip>
#include <map>
#include <utility>
#include <vector>

#define PARROT \
	"            .cc;.  ...  .;c.               \n" \
	"         .,,cc:cc:lxxxl:ccc:;,.            \n" \
	"        .lo;...lKKklllookl..cO;            \n" \
	"      .cl;.,;'.okl;...'.;,..';:.           \n" \
	"     .:o;;dkx,.ll..,cc::,..,'.;:,.         \n" \
	"     co..lKKKkokl.':lllo;''ol..;dl.        \n" \
	"   .,c;.,xKKKKKKo.':llll;.'oOxo,.cl,.      \n" \
	"   cNo..lKKKKKKKo'';llll;;okKKKl..oNc      \n" \
	"   cNo..lKKKKKKKko;':c:,'lKKKKKo'.oNc      \n" \
	"   cNo..lKKKKKKKKKl.....'dKKKKKxc,l0:      \n" \
	"   .c:'.lKKKKKKKKKk;....oKKKKKKo'.oNc      \n" \
	"     ,:.,oxOKKKKKKKOxxxxOKKKKKKxc,;ol:.    \n" \
	"     ;c..'':oookKKKKKKKKKKKKKKKKKk:.'clc.  \n" \
	"   ,dl'.,oxo;'';oxOKKKKKKKKKKKKKKKOxxl::;,,.\n" \
	"  .dOc..lKKKkoooookKKKKKKKKKKKKKKKKKKKxl,;ol.\n" \
	"  cx,';okKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKl..;lc.\n" \
	"  co..:dddddddddddddddddddddddddddddddddl:;''::.  \n" \
	"  co...........................................\"   \n"


#endif