#include "devilution.h"
#include "stubs.h"
#include "dvlnet/abstract_net.h"

namespace dvl {

static net::abstract_net* dvlnet_inst;

/**
 * Initialize a connection provider.
 *
 * @brief Called by engine for single, called by ui for multi
 * @param provider BNET, IPXN, MODM, SCBL or UDPN
 * @param fileinfo Ignore
 */
int SNetInitializeProvider(
	unsigned long            provider,    // type of network connection. game sets this to 'loopback' when playing single player.
	struct _SNETPROGRAMDATA *client_info, // client information.
	struct _SNETPLAYERDATA  *user_info,   // user information.
	struct _SNETUIDATA      *ui_info,     // user interface information.
	struct _SNETVERSIONDATA *fileinfo     // file information.
)
{
	dvlnet_inst = net::abstract_net::make_net(provider);
	return ui_info->selectnamecallback(client_info, user_info, ui_info, fileinfo, provider, NULL, 0, NULL, 0, NULL);
}

// Destroy/free the allocated network resources.
BOOL SNetDestroy()
{
	delete dvlnet_inst;
	return true;
}

BOOL SNetSendServerChatCommand(const char *command)
{
	DUMMY();
	return true;
}

// Send a message directly to a player over the network,
// although the game never sends a direct message to one of the players.
// It only broadcasts messages to all players.
BOOL SNetSendMessage(
	int          playerID,  // player ID.
	void        *data,      // message data to send.
	unsigned int databytes) // length of message data in bytes.
{
	return dvlnet_inst->SNetSendMessage(playerID, data, databytes);
}

// Receive a direct message from a player on the network.
BOOL SNetReceiveMessage(
	int *senderplayerid, // outputs the ID of the player that sent the message.
	char **data,         // outputs the data sent as an array of bytes.
	int *databytes       // outputs the number of bytes sent by the player.
)
{
	if (!dvlnet_inst->SNetReceiveMessage(senderplayerid, data, databytes)) {
		SErrSetLastError(STORM_ERROR_NO_MESSAGES_WAITING);
		return false;
	}
	return true;
}

// Send a turn to all connected players over the network.
BOOL SNetSendTurn(
	char        *data,     // the array of bytes to send.
	unsigned int databytes // the length of the array to send in bytes.
)
{
	return dvlnet_inst->SNetSendTurn(data, databytes);
}

// Receive turns from all connected players over the network.
BOOL SNetReceiveTurns(
	int    a1,				 // ??
	int    arraysize,        // number of pointers in the array.
	char  *arraydata[],      // the array itself.
	DWORD *arraydatabytes,   // the length of the array in bytes that the pointer points to.
	DWORD *arrayplayerstatus // array container the status of the players.
)
{
	if (a1 != 0)
		UNIMPLEMENTED();
	if (arraysize != MAX_PLRS)
		UNIMPLEMENTED();

	if (!dvlnet_inst->SNetReceiveTurns(arraydata, arraydatabytes, arrayplayerstatus)) {
		SErrSetLastError(STORM_ERROR_NO_MESSAGES_WAITING);
		return false;
	}
	return true;
}

// Get number of turns from the owner that is waiting to be received by your client.
BOOL SNetGetOwnerTurnsWaiting(DWORD *turns)
{
	// Is this the mirror image of SNetGetTurnsInTransit?
	return dvlnet_inst->SNetGetOwnerTurnsWaiting(turns);
}

// Get number of turns currently in transit by your client.
BOOL SNetGetTurnsInTransit(int *turns)
{
	return dvlnet_inst->SNetGetTurnsInTransit(turns);
}

// Get provider capabilities.
int SNetGetProviderCaps(struct _SNETCAPS *caps)
{
	caps->size 					= 0;        // engine writes only ?!?
	caps->flags 				= 0;        // unused
	caps->maxmessagesize 		= 512;      // capped to 512; underflow if < 24
	caps->maxqueuesize 			= 0;        // unused
	caps->maxplayers 			= MAX_PLRS; // capped to 4
	caps->bytessec 				= 1000000;  // bytes per second?
	caps->latencyms 			= 0;        // unused
	caps->defaultturnssec 		= 10;       // default turn per second?
	caps->defaultturnsintransit = 1; 	    // maximum acceptable number of turns in queue?
	return 1;
}

// Register event handler.
BOOL SNetRegisterEventHandler(
	int         evtype, // event type.
	SEVTHANDLER func    // event handler (callback).
)
{
	return dvlnet_inst->SNetRegisterEventHandler(*reinterpret_cast<event_type*>(&evtype), func);
}

// Unregister event handler.
BOOL SNetUnregisterEventHandler(
	int         evtype, // event type.
	SEVTHANDLER func    // event handler (callback).
)
{
	return dvlnet_inst->SNetUnregisterEventHandler(*reinterpret_cast<event_type*>(&evtype), func);
}

// Get game information.
BOOL SNetGetGameInfo(int type, void *dst, unsigned int length, unsigned int *byteswritten)
{
	DUMMY();
	return true;
}

// Called by engine for single player, called by UI for multi player.
BOOL SNetCreateGame(
	const char *pszGameName,
	const char *pszGamePassword,   // game password set by game creator.
	const char *pszGameStatString,
    DWORD 		dwGameType,
    char 	   *GameTemplateData,  // points to the game template data in bytes.
    int 		GameTemplateSize,  // size of the game template data in bytes.
    int 		playerCount,
    char 	   *creatorName,
    char 	   *a11,
    int        *playerID           // outputs a generated player ID for the user that created the game.
)
{
	if (GameTemplateSize != 8)
		ABORT();

	net::buffer_t game_init_info(GameTemplateData, GameTemplateData + GameTemplateSize);
	dvlnet_inst->setup_gameinfo(std::move(game_init_info));

	char addrstr[129] = "0.0.0.0";
	SRegLoadString("dvlnet", "bindaddr", 0, addrstr, 128);
	return (*playerID = dvlnet_inst->create(addrstr, pszGamePassword)) != -1;
}

// Join an existing game.
BOOL SNetJoinGame(
	int   id,
	char *pszGameName,     // used as IP address.
	char *pszGamePassword, // game password.
	char *playerName,      // name of the player that is joining the game.
	char *userStats,
	int  *playerID         // outputs your player ID.
)
{
	return (*playerID = dvlnet_inst->join(pszGameName, pszGamePassword)) != -1; // -1 indicates error.
}

// Leave a joined game.
BOOL SNetLeaveGame(int type)
{
	return dvlnet_inst->SNetLeaveGame(type);
}

BOOLEAN SNetSetBasePlayer(int) // engine calls this only once with argument 1.
{
	return true;
}

// Drop/disconnect a player from an established game.
BOOL SNetDropPlayer(
	int   playerid, // player ID to drop.
	DWORD flags
)
{
	return dvlnet_inst->SNetDropPlayer(playerid, flags);
}

// since we never signal STORM_ERROR_REQUIRES_UPGRADE the engine will not call this function.
BOOL SNetPerformUpgrade(DWORD *upgradestatus)
{
	UNIMPLEMENTED();
}

}
