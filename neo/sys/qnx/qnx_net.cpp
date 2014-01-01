/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013 Vincent Simonetti

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
#pragma hdrstop
#include "../../idlib/precompiled.h"

#include <bps/netstatus.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

static bool usingSocks = false;

/*
================================================================================================

	Network CVars

================================================================================================
*/

idCVar net_socksServer( "net_socksServer", "", CVAR_ARCHIVE, "" );
idCVar net_socksPort( "net_socksPort", "1080", CVAR_ARCHIVE | CVAR_INTEGER, "" );
idCVar net_socksUsername( "net_socksUsername", "", CVAR_ARCHIVE, "" );
idCVar net_socksPassword( "net_socksPassword", "", CVAR_ARCHIVE, "" );

idCVar net_ip( "net_ip", "localhost", 0, "local IP address" );

static struct sockaddr_in	socksRelayAddr;

static int		ip_socket;
static int		socks_socket;
static char		socksBuf[4096];

typedef struct {
	unsigned long ip;
	unsigned long mask;
	char addr[16];
} net_interface;

#define 		MAX_INTERFACES	32
int				num_interfaces = 0;
net_interface	netint[MAX_INTERFACES];

/*
================================================================================================

	Free Functions

================================================================================================
*/

/*
========================
NET_ErrorString
========================
*/
const char *NET_ErrorString() {
	int		code;

	code = errno;
	switch( code ) {
	case EACCES: return "EACCES";
	case EAFNOSUPPORT: return "EAFNOSUPPORT";
	case EINTR: return "EINTR";
	case EMFILE: return "EMFILE";
	case ENFILE: return "ENFILE";
	case ENOBUFS: return "ENOBUFS";
	case ENOMEM: return "ENOMEM";
	case EPROTONOSUPPORT: return "EPROTONOSUPPORT";
	case EBADF: return "EBADF";
	case EDOM: return "EDOM";
	case EFAULT: return "EFAULT";
	case EINVAL: return "EINVAL";
	case ENOPROTOOPT: return "ENOPROTOOPT";
	case EAGAIN: return "EAGAIN";
	case ECONNRESET: return "ECONNRESET";
	case EDESTADDRREQ: return "EDESTADDRREQ";
	case EIO: return "EIO";
	case EMSGSIZE: return "EMSGSIZE";
	case ENETDOWN: return "ENETDOWN";
	case ENETUNREACH: return "ENETUNREACH";
	case ENOTCONN: return "ENOTCONN";
	case ENOTSOCK: return "ENOTSOCK";
	case EOPNOTSUPP: return "EOPNOTSUPP";
	case EPIPE: return "EPIPE";
	//case EWOULDBLOCK: return "EWOULDBLOCK"; //Same value as EAGAIN
	case EHOSTUNREACH: return "EHOSTUNREACH";
	case EISCONN: return "EISCONN";
	case ELOOP: return "ELOOP";
	case ENAMETOOLONG: return "ENAMETOOLONG";
	case ENOENT: return "ENOENT";
	case ENOTDIR: return "ENOTDIR";
	case ENOTTY: return "ENOTTY";
	case EADDRINUSE: return "EADDRINUSE";
	case EADDRNOTAVAIL: return "EADDRNOTAVAIL";
	default: return "NO ERROR";
	}
}

/*
========================
Net_NetadrToSockadr
========================
*/
void Net_NetadrToSockadr( const netadr_t *a, sockaddr_in *s ) {
	memset( s, 0, sizeof(*s) );

	if ( a->type == NA_BROADCAST ) {
		s->sin_family = AF_INET;
		s->sin_addr.s_addr = INADDR_BROADCAST;
	} else if ( a->type == NA_IP || a->type == NA_LOOPBACK ) {
		s->sin_family = AF_INET;
		s->sin_addr.s_addr = *(int *)a->ip;
	}

	s->sin_port = htons( (short)a->port );
}

/*
========================
Net_SockadrToNetadr
========================
*/
void Net_SockadrToNetadr( sockaddr_in *s, netadr_t *a ) {
	unsigned int ip;
	if ( s->sin_family == AF_INET ) {
		ip = s->sin_addr.s_addr;
		*(unsigned int *)a->ip = ip;
		a->port = htons( s->sin_port );
		// we store in network order, that loopback test is host order..
		ip = ntohl( ip );
		if ( ip == INADDR_LOOPBACK ) {
			a->type = NA_LOOPBACK;
		} else {
			a->type = NA_IP;
		}
	}
}

/*
========================
Net_ExtractPort
========================
*/
static bool Net_ExtractPort( const char *src, char *buf, int bufsize, int *port ) {
	char *p;
	strncpy( buf, src, bufsize );
	p = buf; p += Min( bufsize - 1, idStr::Length( src ) ); *p = '\0';
	p = strchr( buf, ':' );
	if ( !p ) {
		return false;
	}
	*p = '\0';
	*port = strtol( p+1, NULL, 10 );
	if ( errno == ERANGE ) {
		return false;
	}
	return true;
}

/*
========================
Net_StringToSockaddr
========================
*/
static bool Net_StringToSockaddr( const char *s, sockaddr_in *sadr, bool doDNSResolve ) {
	struct hostent	*h;
	char buf[256];
	int port;

	memset( sadr, 0, sizeof( *sadr ) );

	sadr->sin_family = AF_INET;
	sadr->sin_port = 0;

	if( s[0] >= '0' && s[0] <= '9' ) {
		unsigned long ret = inet_addr(s);
		if ( ret != INADDR_NONE ) {
			*(int *)&sadr->sin_addr = ret;
		} else {
			// check for port
			if ( !Net_ExtractPort( s, buf, sizeof( buf ), &port ) ) {
				return false;
			}
			ret = inet_addr( buf );
			if ( ret == INADDR_NONE ) {
				return false;
			}
			*(int *)&sadr->sin_addr = ret;
			sadr->sin_port = htons( port );
		}
	} else if ( doDNSResolve ) {
		// try to remove the port first, otherwise the DNS gets confused into multiple timeouts
		// failed or not failed, buf is expected to contain the appropriate host to resolve
		if ( Net_ExtractPort( s, buf, sizeof( buf ), &port ) ) {
			sadr->sin_port = htons( port );
		}
		h = gethostbyname( buf );
		if ( h == 0 ) {
			return false;
		}
		*(int *)&sadr->sin_addr = *(int *)h->h_addr_list[0];
	}

	return true;
}

/*
========================
NET_IPSocket
========================
*/
int NET_IPSocket( const char *net_interface, int port, netadr_t *bound_to ) {
	int					newsocket;
	sockaddr_in			address;
	unsigned long		_true = 1;
	int					i = 1;
	int					err;

	if ( port != PORT_ANY ) {
		if( net_interface ) {
			idLib::Printf( "Opening IP socket: %s:%i\n", net_interface, port );
		} else {
			idLib::Printf( "Opening IP socket: localhost:%i\n", port );
		}
	}

	if( ( newsocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) == -1 ) {
		err = errno;
		if( err != EAFNOSUPPORT ) {
			idLib::Printf( "WARNING: UDP_OpenSocket: socket: %s\n", NET_ErrorString() );
		}
		return 0;
	}

	// make it non-blocking
	if( ioctl_socket( newsocket, FIONBIO, &_true ) == -1 ) {
		idLib::Printf( "WARNING: UDP_OpenSocket: ioctl FIONBIO: %s\n", NET_ErrorString() );
		close( newsocket );
		return 0;
	}

	// make it broadcast capable
	if( setsockopt( newsocket, SOL_SOCKET, SO_BROADCAST, (char *)&i, sizeof(i) ) == -1 ) {
		idLib::Printf( "WARNING: UDP_OpenSocket: setsockopt SO_BROADCAST: %s\n", NET_ErrorString() );
		close( newsocket );
		return 0;
	}

	if( !net_interface || !net_interface[0] || !idStr::Icmp( net_interface, "localhost" ) ) {
		address.sin_addr.s_addr = INADDR_ANY;
	}
	else {
		Net_StringToSockaddr( net_interface, &address, true );
	}

	if( port == PORT_ANY ) {
		address.sin_port = 0;
	}
	else {
		address.sin_port = htons( (short)port );
	}

	address.sin_family = AF_INET;

	if( bind( newsocket, (const sockaddr *)&address, sizeof(address) ) == -1 ) {
		idLib::Printf( "WARNING: UDP_OpenSocket: bind: %s\n", NET_ErrorString() );
		close( newsocket );
		return 0;
	}

	// if the port was PORT_ANY, we need to query again to know the real port we got bound to
	// ( this used to be in idUDP::InitForPort )
	if ( bound_to ) {
		socklen_t len = sizeof( address );
		getsockname( newsocket, (sockaddr *)&address, &len );
		Net_SockadrToNetadr( &address, bound_to );
	}

	return newsocket;
}

/*
========================
NET_OpenSocks
========================
*/
void NET_OpenSocks( int port ) {
	sockaddr_in			address;
	struct hostent		*h;
	int					len;
	bool				rfc1929;
	unsigned char		buf[64];

	usingSocks = false;

	idLib::Printf( "Opening connection to SOCKS server.\n" );

	if ( ( socks_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ) == -1 ) {
		idLib::Printf( "WARNING: NET_OpenSocks: socket: %s\n", NET_ErrorString() );
		return;
	}

	h = gethostbyname( net_socksServer.GetString() );
	if ( h == NULL ) {
		idLib::Printf( "WARNING: NET_OpenSocks: gethostbyname: %s\n", NET_ErrorString() );
		return;
	}
	if ( h->h_addrtype != AF_INET ) {
		idLib::Printf( "WARNING: NET_OpenSocks: gethostbyname: address type was not AF_INET\n" );
		return;
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = *(int *)h->h_addr_list[0];
	address.sin_port = htons( (short)net_socksPort.GetInteger() );

	if ( connect( socks_socket, (sockaddr *)&address, sizeof( address ) ) == -1 ) {
		idLib::Printf( "NET_OpenSocks: connect: %s\n", NET_ErrorString() );
		return;
	}

	// send socks authentication handshake
	if ( *net_socksUsername.GetString() || *net_socksPassword.GetString() ) {
		rfc1929 = true;
	}
	else {
		rfc1929 = false;
	}

	buf[0] = 5;		// SOCKS version
	// method count
	if ( rfc1929 ) {
		buf[1] = 2;
		len = 4;
	}
	else {
		buf[1] = 1;
		len = 3;
	}
	buf[2] = 0;		// method #1 - method id #00: no authentication
	if ( rfc1929 ) {
		buf[2] = 2;		// method #2 - method id #02: username/password
	}
	if ( send( socks_socket, (const void *)buf, len, 0 ) == -1 ) {
		idLib::Printf( "NET_OpenSocks: send: %s\n", NET_ErrorString() );
		return;
	}

	// get the response
	len = recv( socks_socket, (void *)buf, 64, 0 );
	if ( len == -1 ) {
		idLib::Printf( "NET_OpenSocks: recv: %s\n", NET_ErrorString() );
		return;
	}
	if ( len != 2 || buf[0] != 5 ) {
		idLib::Printf( "NET_OpenSocks: bad response\n" );
		return;
	}
	switch( buf[1] ) {
	case 0:	// no authentication
		break;
	case 2: // username/password authentication
		break;
	default:
		idLib::Printf( "NET_OpenSocks: request denied\n" );
		return;
	}

	// do username/password authentication if needed
	if ( buf[1] == 2 ) {
		int		ulen;
		int		plen;

		// build the request
		ulen = idStr::Length( net_socksUsername.GetString() );
		plen = idStr::Length( net_socksPassword.GetString() );

		buf[0] = 1;		// username/password authentication version
		buf[1] = ulen;
		if ( ulen ) {
			memcpy( &buf[2], net_socksUsername.GetString(), ulen );
		}
		buf[2 + ulen] = plen;
		if ( plen ) {
			memcpy( &buf[3 + ulen], net_socksPassword.GetString(), plen );
		}

		// send it
		if ( send( socks_socket, (const void *)buf, 3 + ulen + plen, 0 ) == -1 ) {
			idLib::Printf( "NET_OpenSocks: send: %s\n", NET_ErrorString() );
			return;
		}

		// get the response
		len = recv( socks_socket, (void *)buf, 64, 0 );
		if ( len == -1 ) {
			idLib::Printf( "NET_OpenSocks: recv: %s\n", NET_ErrorString() );
			return;
		}
		if ( len != 2 || buf[0] != 1 ) {
			idLib::Printf( "NET_OpenSocks: bad response\n" );
			return;
		}
		if ( buf[1] != 0 ) {
			idLib::Printf( "NET_OpenSocks: authentication failed\n" );
			return;
		}
	}

	// send the UDP associate request
	buf[0] = 5;		// SOCKS version
	buf[1] = 3;		// command: UDP associate
	buf[2] = 0;		// reserved
	buf[3] = 1;		// address type: IPV4
	*(int *)&buf[4] = INADDR_ANY;
	*(short *)&buf[8] = htons( (short)port );		// port
	if ( send( socks_socket, (const void *)buf, 10, 0 ) == -1 ) {
		idLib::Printf( "NET_OpenSocks: send: %s\n", NET_ErrorString() );
		return;
	}

	// get the response
	len = recv( socks_socket, (void *)buf, 64, 0 );
	if( len == -1 ) {
		idLib::Printf( "NET_OpenSocks: recv: %s\n", NET_ErrorString() );
		return;
	}
	if( len < 2 || buf[0] != 5 ) {
		idLib::Printf( "NET_OpenSocks: bad response\n" );
		return;
	}
	// check completion code
	if( buf[1] != 0 ) {
		idLib::Printf( "NET_OpenSocks: request denied: %i\n", buf[1] );
		return;
	}
	if( buf[3] != 1 ) {
		idLib::Printf( "NET_OpenSocks: relay address is not IPV4: %i\n", buf[3] );
		return;
	}
	socksRelayAddr.sin_family = AF_INET;
	socksRelayAddr.sin_addr.s_addr = *(int *)&buf[4];
	socksRelayAddr.sin_port = *(short *)&buf[8];
	memset( socksRelayAddr.sin_zero, 0, sizeof( socksRelayAddr.sin_zero ) );

	usingSocks = true;
}

/*
========================
Net_WaitForData
========================
*/
bool Net_WaitForData( int netSocket, int timeout ) {
	int					ret;
	fd_set				set;
	struct timeval		tv;

	if ( !netSocket ) {
		return false;
	}

	if ( timeout < 0 ) {
		return true;
	}

	FD_ZERO( &set );
	FD_SET( netSocket, &set );

	tv.tv_sec = 0;
	tv.tv_usec = timeout * 1000;

	ret = select( netSocket + 1, &set, NULL, NULL, &tv );

	if ( ret == -1 ) {
		idLib::Printf( "Net_WaitForData select(): %s\n", strerror( errno ) );
		return false;
	}

	// timeout with no data
	if ( ret == 0 ) {
		return false;
	}

	return true;
}

/*
========================
Net_GetUDPPacket
========================
*/
bool Net_GetUDPPacket( int netSocket, netadr_t &net_from, char *data, int &size, int maxSize ) {
	int 			ret;
	sockaddr_in		from;
	socklen_t		fromlen;
	int				err;

	if ( !netSocket ) {
		return false;
	}

	fromlen = sizeof(from);
	ret = recvfrom( netSocket, ( void * )data, maxSize, 0, (sockaddr *)&from, &fromlen );
	if ( ret == -1 ) {
		err = errno;

		if ( err == EWOULDBLOCK || err == ECONNRESET ) {
			return false;
		}
		char	buf[1024];
		sprintf( buf, "Net_GetUDPPacket: %s\n", NET_ErrorString() );
		idLib::Printf( buf );
		return false;
	}

	if ( netSocket == ip_socket ) {
		memset( from.sin_zero, 0, sizeof( from.sin_zero ) );
	}

	if ( usingSocks && netSocket == ip_socket && memcmp( &from, &socksRelayAddr, fromlen ) == 0 ) {
		if ( ret < 10 || data[0] != 0 || data[1] != 0 || data[2] != 0 || data[3] != 1 ) {
			return false;
		}
		net_from.type = NA_IP;
		net_from.ip[0] = data[4];
		net_from.ip[1] = data[5];
		net_from.ip[2] = data[6];
		net_from.ip[3] = data[7];
		net_from.port = *(short *)&data[8];
		memmove( data, &data[10], ret - 10 );
	} else {
		Net_SockadrToNetadr( &from, &net_from );
	}

	if ( ret > maxSize ) {
		char	buf[1024];
		sprintf( buf, "Net_GetUDPPacket: oversize packet from %s\n", Sys_NetAdrToString( net_from ) );
		idLib::Printf( buf );
		return false;
	}

	size = ret;

	return true;
}

/*
========================
Net_SendUDPPacket
========================
*/
void Net_SendUDPPacket( int netSocket, int length, const void *data, const netadr_t to ) {
	int				ret;
	sockaddr_in		addr;

	if ( !netSocket ) {
		return;
	}

	Net_NetadrToSockadr( &to, &addr );

	if ( usingSocks && to.type == NA_IP ) {
		socksBuf[0] = 0;	// reserved
		socksBuf[1] = 0;
		socksBuf[2] = 0;	// fragment (not fragmented)
		socksBuf[3] = 1;	// address type: IPV4
		*(int *)&socksBuf[4] = addr.sin_addr.s_addr;
		*(short *)&socksBuf[8] = addr.sin_port;
		memcpy( &socksBuf[10], data, length );
		ret = sendto( netSocket, (const void *)socksBuf, length+10, 0, (sockaddr *)&socksRelayAddr, sizeof(socksRelayAddr) );
	} else {
		ret = sendto( netSocket, (const void *)data, length, 0, (sockaddr *)&addr, sizeof(addr) );
	}
	if ( ret == -1 ) {
		int err = errno;

		// some PPP links do not allow broadcasts and return an error
		if ( ( err == EADDRNOTAVAIL ) && ( to.type == NA_BROADCAST ) ) {
			return;
		}

		// NOTE: EWOULDBLOCK used to be silently ignored,
		// but that means the packet will be dropped so I don't feel it's a good thing to ignore
		idLib::Printf( "UDP sendto error - packet dropped: %s\n", NET_ErrorString() );
	}
}

/*
========================
Net_InterfaceType
========================
*/
const char* Net_InterfaceType( netstatus_interface_type_t type ) {
	const char* typeStr;

	switch ( type ) {
	case NETSTATUS_INTERFACE_TYPE_WIRED:
		typeStr = "Wired";
		break;

	case NETSTATUS_INTERFACE_TYPE_WIFI:
		typeStr = "Wi-Fi";
		break;

	case NETSTATUS_INTERFACE_TYPE_BLUETOOTH_DUN:
		typeStr = "Bluetooth dial-up";
		break;

	case NETSTATUS_INTERFACE_TYPE_USB:
		typeStr = "USB";
		break;

	case NETSTATUS_INTERFACE_TYPE_VPN:
		typeStr = "VPN";
		break;

	case NETSTATUS_INTERFACE_TYPE_BB:
		typeStr = "BlackBerry";
		break;

	case NETSTATUS_INTERFACE_TYPE_CELLULAR:
		typeStr = "Cellular";
		break;

#if BBNDK_VERSION_AT_LEAST(10, 2, 0)
	case NETSTATUS_INTERFACE_TYPE_P2P:
		typeStr = "P2P";
		break;
#endif

	case NETSTATUS_INTERFACE_TYPE_UNKNOWN:
	default:
		typeStr = "Unknown";
		break;
	}

	return typeStr;
}

/*
========================
Sys_InitNetworking
========================
*/
void Sys_InitNetworking() {
	netstatus_interface_list_t	list;

	if ( netstatus_get_interfaces( &list ) != BPS_SUCCESS ) {
		idLib::FatalError( "Sys_InitNetworking: Couldn't retrieve network list from netstatus_get_interfaces." );
	}

	bool foundloopback;
	netstatus_interface_details_t* details;

	num_interfaces = 0;
	foundloopback = false;

	for ( int i = 0; i < list.num_interfaces; i++ ) {
		if ( netstatus_get_interface_details( list.interfaces[i], &details ) != BPS_SUCCESS ) {
			idLib::Printf( "could not get details on %s - skipped\n", list.interfaces[i] );
			continue;
		}

		idLib::Printf( "Found interface: %s of type %s - \n", netstatus_interface_get_name( details ), Net_InterfaceType( netstatus_interface_get_type( details ) ) );

		int ipCount = netstatus_interface_get_num_ip_addresses( details );

		for ( int k = 0; k < ipCount; k++ ) {
			const char* ipAddress = netstatus_interface_get_ip_address( details, k );
			const char* ipMask = netstatus_interface_get_ip_address_netmask( details, k );

			//Skip IPv6 since inet_addr doesn't like it
			if( idStr::FindChar( ipAddress, ':' ) != -1 ) {
				idLib::Printf( "     %s IPv6 address - skipped\n", ipAddress );
				continue;
			}

			unsigned long ip_a, ip_m;
			if( !idStr::Icmp( "127.0.0.1", ipAddress ) ) {
				foundloopback = true;
			}
			ip_a = ntohl( inet_addr( ipAddress ) );
			ip_m = ntohl( inet_addr( ipMask ) );
			//skip null netmasks
			if( !ip_m ) {
				idLib::Printf( "     %s NULL netmask - skipped\n", ipAddress );
				continue;
			}
			idLib::Printf( "     %s/%s\n", ipAddress, ipMask );
			netint[num_interfaces].ip = ip_a;
			netint[num_interfaces].mask = ip_m;
			idStr::Copynz( netint[num_interfaces].addr, ipAddress, sizeof( netint[num_interfaces].addr ) );
			num_interfaces++;
			if( num_interfaces >= MAX_INTERFACES ) {
				idLib::Printf( "Sys_InitNetworking: MAX_INTERFACES(%d) hit.\n", MAX_INTERFACES );
				netstatus_free_interface_details( &details );
				netstatus_free_interfaces( &list );
				return;
			}
		}

		netstatus_free_interface_details( &details );
	}

	netstatus_free_interfaces( &list );

	if( !foundloopback && num_interfaces < MAX_INTERFACES ) {
		idLib::Printf( "Sys_InitNetworking: adding loopback interface\n" );
		netint[num_interfaces].ip = ntohl( inet_addr( "127.0.0.1" ) );
		netint[num_interfaces].mask = ntohl( inet_addr( "255.0.0.0" ) );
		num_interfaces++;
	}
}

/*
========================
Sys_ShutdownNetworking
========================
*/
void Sys_ShutdownNetworking() {
}

/*
========================
Sys_StringToNetAdr
========================
*/
bool Sys_StringToNetAdr( const char *s, netadr_t *a, bool doDNSResolve ) {
	sockaddr_in sadr;

	if ( !Net_StringToSockaddr( s, &sadr, doDNSResolve ) ) {
		return false;
	}

	Net_SockadrToNetadr( &sadr, a );
	return true;
}

/*
========================
Sys_NetAdrToString
========================
*/
const char *Sys_NetAdrToString( const netadr_t a ) {
	static int index = 0;
	static char buf[ 4 ][ 64 ];	// flip/flop
	char *s;

	s = buf[index];
	index = (index + 1) & 3;

	if ( a.type == NA_LOOPBACK ) {
		if ( a.port ) {
			idStr::snPrintf( s, 64, "localhost:%i", a.port );
		} else {
			idStr::snPrintf( s, 64, "localhost" );
		}
	} else if ( a.type == NA_IP ) {
		idStr::snPrintf( s, 64, "%i.%i.%i.%i:%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3], a.port );
	}
	return s;
}

/*
========================
Sys_IsLANAddress
========================
*/
bool Sys_IsLANAddress( const netadr_t adr ) {
	if ( adr.type == NA_LOOPBACK ) {
		return true;
	}

	if ( adr.type != NA_IP ) {
		return false;
	}

	if ( num_interfaces ) {
		int i;
		unsigned long *p_ip;
		unsigned long ip;
		p_ip = (unsigned long *)&adr.ip[0];
		ip = ntohl( *p_ip );

		for( i = 0; i < num_interfaces; i++ ) {
			if( ( netint[i].ip & netint[i].mask ) == ( ip & netint[i].mask ) ) {
				return true;
			}
		}
	}
	return false;
}

/*
========================
Sys_CompareNetAdrBase

Compares without the port.
========================
*/
bool Sys_CompareNetAdrBase( const netadr_t a, const netadr_t b ) {
	if ( a.type != b.type ) {
		return false;
	}

	if ( a.type == NA_LOOPBACK ) {
		if ( a.port == b.port ) {
			return true;
		}

		return false;
	}

	if ( a.type == NA_IP ) {
		if ( a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1] && a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3] ) {
			return true;
		}
		return false;
	}

	idLib::Printf( "Sys_CompareNetAdrBase: bad address type\n" );
	return false;
}

/*
========================
Sys_GetLocalIPCount
========================
*/
int	Sys_GetLocalIPCount() {
	return num_interfaces;
}

/*
========================
Sys_GetLocalIP
========================
*/
const char * Sys_GetLocalIP( int i ) {
	if ( ( i < 0 ) || ( i >= num_interfaces ) ) {
		return NULL;
	}
	return netint[i].addr;
}

/*
================================================================================================

	idUDP

================================================================================================
*/

/*
========================
idUDP::idUDP
========================
*/
idUDP::idUDP() {
	netSocket = 0;
	memset( &bound_to, 0, sizeof( bound_to ) );
	silent = false;
	packetsRead = 0;
	bytesRead = 0;
	packetsWritten = 0;
	bytesWritten = 0;
}

/*
========================
idUDP::~idUDP
========================
*/
idUDP::~idUDP() {
	Close();
}

/*
========================
idUDP::InitForPort
========================
*/
bool idUDP::InitForPort( int portNumber ) {
	netSocket = NET_IPSocket( net_ip.GetString(), portNumber, &bound_to );
	if ( netSocket <= 0 ) {
		netSocket = 0;
		memset( &bound_to, 0, sizeof( bound_to ) );
		return false;
	}

	return true;
}

/*
========================
idUDP::Close
========================
*/
void idUDP::Close() {
	if ( netSocket ) {
		close( netSocket );
		netSocket = 0;
		memset( &bound_to, 0, sizeof( bound_to ) );
	}
}

/*
========================
idUDP::GetPacket
========================
*/
bool idUDP::GetPacket( netadr_t &from, void *data, int &size, int maxSize ) {
	bool ret;

	while ( 1 ) {

		ret = Net_GetUDPPacket( netSocket, from, (char *)data, size, maxSize );
		if ( !ret ) {
			break;
		}

		packetsRead++;
		bytesRead += size;

		break;
	}

	return ret;
}

/*
========================
idUDP::GetPacketBlocking
========================
*/
bool idUDP::GetPacketBlocking( netadr_t &from, void *data, int &size, int maxSize, int timeout ) {

	if ( !Net_WaitForData( netSocket, timeout ) ) {
		return false;
	}

	if ( GetPacket( from, data, size, maxSize ) ) {
		return true;
	}

	return false;
}

/*
========================
idUDP::SendPacket
========================
*/
void idUDP::SendPacket( const netadr_t to, const void *data, int size ) {
	if ( to.type == NA_BAD ) {
		idLib::Warning( "idUDP::SendPacket: bad address type NA_BAD - ignored" );
		return;
	}

	packetsWritten++;
	bytesWritten += size;

	if ( silent ) {
		return;
	}

	Net_SendUDPPacket( netSocket, size, data, to );
}
