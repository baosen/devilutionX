#include "dvlnet/abstract_net.h"

#include "stubs.h"
#include "dvlnet/tcp_client.h"
#include "dvlnet/udp_p2p.h"
#include "dvlnet/loopback.h"

namespace dvl {
namespace net {

abstract_net::~abstract_net()
{
}

abstract_net* abstract_net::make_net(provider_t provider)
{
	if (provider == 'TCPN') {
		return new tcp_client;
	} else if (provider == 'UDPN') {
		return new udp_p2p;
	} else if (provider == 'SCBL' || provider == 0) {
		return new loopback;
	} else {
		ABORT();
	}
}

}  // namespace net
}  // namespace dvl
