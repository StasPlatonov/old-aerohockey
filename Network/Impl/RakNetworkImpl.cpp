#include "RakNetworkImpl.h"

#include "BitStream.h"
#include "Rand.h"
#include "RakNetStatistics.h"
#include "MessageIdentifiers.h"

#include "RakAssert.h"
#include "GetTime.h"
#include "Common/Timer.h"
#include "Common/Utils.h"

#if defined(_WIN32)
    #pragma comment(lib, "RakNet_VS2008_LibStatic_Debug_Win32.lib")
    #pragma comment(lib, "ws2_32.lib")
#endif

namespace Network
{
	using namespace RakNet;

	namespace
	{
		enum
		{
			ID_CUSTOM_DATA = ID_USER_PACKET_ENUM,
		};

		unsigned char GetPacketIdentifier(RakNet::Packet *p)
		{
			if (p == 0)
				return 255;

			if ((unsigned char)p->data[0] == ID_TIMESTAMP)
			{
				//RakAssert(p->length > sizeof(RakNet::MessageID) + sizeof(Network::NetTime));
				//return (unsigned char) p->data[sizeof(RakNet::MessageID) + sizeof(Network::NetTime)];
				return (unsigned char)p->data[sizeof(RakNet::MessageID) + sizeof(RakNet::Time)];
			}
			else
			{
				return (unsigned char)p->data[0];
			}
		}

		RakNetPeerFactory g_RakNetFactory;
	}

	RakNetPeer::RakNetPeer(INetPeerHandler& handler)
		: m_Peer(RakNet::RakPeerInterface::GetInstance())
		, m_Handler(handler)
	{
		const RakNet::RakNetGUID rng = m_Peer->GetMyGUID();
		printf("GUID: %s\n", rng.ToString());
		//m_Peer->SetPerConnectionOutgoingBandwidthLimit(8 * 128 * 1024);
	}

	RakNetPeer::~RakNetPeer()
	{
		RakNet::RakPeerInterface::DestroyInstance(m_Peer);
	}
	//-----------------------------------------------------------------------------------

	NetId RakNetPeer::GetId() const
	{
		return m_Peer->GetMyGUID().g;
	}

	unsigned int RakNetPeer::GetConnectionsCount() const
	{
		unsigned short numberOfSystems = 0;
		m_Peer->GetConnectionList(0, &numberOfSystems);
		return numberOfSystems;
	}

	void RakNetPeer::SetMaximumIncomingConnections(unsigned int count)
	{
		m_Peer->SetMaximumIncomingConnections(count);
	}

	void RakNetPeer::SetIncomingPassword(const std::string& password)
	{
		m_Peer->SetIncomingPassword(password.c_str(), password.size());
	}

	void RakNetPeer::Startup(const PeerParameters& params)
	{
		RakNet::SocketDescriptor socketDescriptor;
		socketDescriptor.port = params.m_Port;
		StartupResult b = m_Peer->Startup(params.m_MaxConnections, &socketDescriptor, 1);
		m_Peer->SetOccasionalPing(true);
	}
	//-----------------------------------------------------------------------------------

	int RakNetPeer::GetPing(NetId id) const
	{
		if (id == EmptyNetId)
			return 0;

		RakNet::SystemAddress addrTo = m_Peer->GetSystemAddressFromGuid(RakNetGUID(id));
		return m_Peer->GetAveragePing(addrTo);
	}
	//-----------------------------------------------------------------------------------

	void RakNetPeer::Shutdown()
	{
		m_Peer->Shutdown(0);
	}

	bool RakNetPeer::Connect(const std::string& host, unsigned short port, const std::string& password)
	{
		return (m_Peer->Connect(host.c_str(), port, password.c_str(), password.size(), 0) == RakNet::CONNECTION_ATTEMPT_STARTED);
	}

	void RakNetPeer::Disconnect()
	{
		m_Peer->CloseConnection(m_Peer->GetSystemAddressFromIndex(0), true, 0);
	}
	//-----------------------------------------------------------------------------------


	void RakNetPeer::Kick(NetId id)
	{
		RakNet::SystemAddress addr = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
		if (id != EmptyNetId)
		{
			addr = m_Peer->GetSystemAddressFromGuid(RakNetGUID(id));
		}

		m_Peer->CloseConnection(addr, true);
	}

	void RakNetPeer::Ban(const std::string& ip, unsigned int ms)
	{
		m_Peer->AddToBanList(ip.c_str(), ms);
	}

	void RakNetPeer::Ping(NetId id)
	{
		if (id == EmptyNetId)
			return;

		RakNet::SystemAddress addrTo = m_Peer->GetSystemAddressFromGuid(RakNetGUID(id));
		m_Peer->Ping(addrTo);
	}
	//-----------------------------------------------------------------------------------

	void RakNetPeer::Send(const unsigned char* data, long size, NetId id, bool broadcast, bool timed)
	{
		RakNet::SystemAddress addrTo = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
		if (id != EmptyNetId)
		{
			addrTo = m_Peer->GetSystemAddressFromGuid(RakNetGUID(id));
		}

		BitStream outputBitStream;
		if (timed)
		{
			outputBitStream.Write((unsigned char)ID_TIMESTAMP);
			const Network::NetTime time = Common::Timer::GetInstance().GetTime();
			//printf("Timestamped send: %llu\n", time);
			outputBitStream.Write<Network::NetTime>(time);
		}
		outputBitStream.Write((unsigned char)ID_CUSTOM_DATA);
		outputBitStream.Write((const char*)data, size);
		m_Peer->Send(&outputBitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addrTo, broadcast);
	}
	//-----------------------------------------------------------------------------------

	void RakNetPeer::ProcessPacketInternal(Packet* p)
	{
		unsigned char packetId = GetPacketIdentifier(p);

		const NetId sender = p->guid.g;

		switch (packetId)
		{
		case ID_CONNECTION_REQUEST_ACCEPTED:
			printf("ID_CONNECTION_REQUEST_ACCEPTED from %s[%s]\n", p->systemAddress.ToString(), p->guid.ToString());
			m_Handler.OnAccepted(sender);
			break;
			// print out errors
		case ID_CONNECTION_ATTEMPT_FAILED:
			printf("Error: ID_CONNECTION_ATTEMPT_FAILED\n");
			m_Handler.OnConnectionAttemptFailed(sender);
			break;
		case ID_ALREADY_CONNECTED:
			printf("Error: ID_ALREADY_CONNECTED\n");
			break;
		case ID_CONNECTION_BANNED:
			printf("Error: ID_CONNECTION_BANNED\n");
			break;
		case ID_INVALID_PASSWORD:
			printf("Error: ID_INVALID_PASSWORD\n");
			break;
		case ID_INCOMPATIBLE_PROTOCOL_VERSION:
			printf("Error: ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			printf("Error: ID_NO_FREE_INCOMING_CONNECTIONS\n");
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			printf("ID_DISCONNECTION_NOTIFICATION from %s[%s]\n", p->systemAddress.ToString(), p->guid.ToString());
			m_Handler.OnDisconnected(sender);
			break;
		case ID_CONNECTION_LOST:
			printf("Error: ID_CONNECTION_LOST from %s[%s]\n", p->systemAddress.ToString(), p->guid.ToString());
			m_Handler.OnConnectionLost(sender);
			break;
		case ID_NEW_INCOMING_CONNECTION:
			printf("ID_NEW_INCOMING_CONNECTION from %s[%s]\n", p->systemAddress.ToString(), p->guid.ToString());
			m_Handler.OnNewIncomingConnection(sender);
			break;

		case ID_CONNECTED_PING:
		case ID_UNCONNECTED_PING:
		{
			printf("Ping from SystemAddress %s.\n", p->systemAddress.ToString(true));
			unsigned int dataLength;
			RakNet::TimeMS time;
			RakNet::BitStream bsIn(p->data, p->length, false);
			bsIn.IgnoreBytes(1);
			bsIn.Read(time);
			dataLength = p->length - sizeof(unsigned char) - sizeof(RakNet::TimeMS);
			printf("Time is %i\n", time);
			printf("Ping is %i\n", (unsigned int)(RakNet::GetTimeMS() - time));
			printf("Data is %i bytes long.\n", dataLength);
			break;
		}
		case ID_TIMESTAMP:
			printf("ID_TIMESTAMP\n");
			break;

		case ID_CUSTOM_DATA:
		{
			int offset = sizeof(unsigned char); //skip ID_CUSTOM_DATA
			NetTime time = 0;
			if (p->data[0] == ID_TIMESTAMP)
			{
				BitStream bs(p->data, p->length, false);
				offset += sizeof(unsigned char); //skip ID_TIMESTAMP
				offset += sizeof(Network::NetTime);//skip timestamp
				//memcpy(&time, &p->data[sizeof(unsigned char)], sizeof(Network::NetTime));
				bs.IgnoreBytes(1);
				bs.Read<Network::NetTime>(time);
				//printf("Timestamped receive: %llu\n", time);
			}
			unsigned char* pData = p->data + offset;
			unsigned int size = p->length - offset;
			m_Handler.OnData(sender, pData, size, time);
			break;
		}
		}
	}
	//-----------------------------------------------------------------------------------

	void RakNetPeer::Update()
	{
		while (Packet* packet = m_Peer->Receive())
		{
			ProcessPacketInternal(packet);
			m_Peer->DeallocatePacket(packet);
		}
	}
	//-----------------------------------------------------------------------------------

	std::string RakNetPeer::GetStatistics() const
	{
		std::string result;
		unsigned int numAddr = m_Peer->GetNumberOfAddresses();
		for (unsigned int i = 0; i < numAddr; ++i)
		{
			char text[2048] = { 0 };
			result += "Interface[";
#if defined(WIN32) || defined(WINNT)
			_itoa(i, text, 10);
#else
			sprintf(text, "%d", i);
#endif
			result += text;
			result += "] statistics:\n";
			RakNetStatistics* rssSender = m_Peer->GetStatistics(m_Peer->GetSystemAddressFromIndex(i));
			StatisticsToString(rssSender, text, 3);
			result += std::string(text);
			result += "\n";
		}
		return result;
	}
	//-----------------------------------------------------------------------------------

	INetPeer::Ptr RakNetPeerFactory::CreatePeer(INetPeerHandler& handler)
	{
		INetPeer::Ptr result(new RakNetPeer(handler));
		return result;
	}

	void RakNetPeerFactory::DestroyPeer(INetPeer::Ptr peer)
	{
		delete peer;
	}
	//-----------------------------------------------------------------------------------

	INetPeerFactory& GetRakNetPeerFactory()
	{
		return g_RakNetFactory;
	}
}
