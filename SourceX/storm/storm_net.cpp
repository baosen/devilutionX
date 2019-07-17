#include <memory>

#include "devilution.h"
#include "stubs.h"
#include "dvlnet/abstract_net.h"

namespace dvl {

static std::unique_ptr<net::abstract_net> dvlnet_inst;

BOOL SNetSendServerChatCommand(const char *command)
{
	DUMMY();
	return true;
}

// Send a message directly to a player over the network.
BOOL SNetSendMessage(int playerID, void *data, unsigned int databytes)
{
	return dvlnet_inst->SNetSendMessage(playerID, data, databytes);
}

// Receive a direct message from a player on the network.
BOOL SNetReceiveMessage(int *senderplayerid, char **data, int *databytes)
{
	if (!dvlnet_inst->SNetReceiveMessage(senderplayerid, data, databytes)) {
		SErrSetLastError(STORM_ERROR_NO_MESSAGES_WAITING);
		return false;
	}
	return true;
}

// Send a turn to all connected players over the network.
BOOL SNetSendTurn(char *data, unsigned int databytes)
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

// Get number of turns waiting for the owner.
BOOL SNetGetOwnerTurnsWaiting(DWORD *turns)
{
	// Is this the mirror image of SNetGetTurnsInTransit?
	return dvlnet_inst->SNetGetOwnerTurnsWaiting(turns);
}

// Get number of turns currently in transit.
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
	caps->bytessec 				= 1000000;  // ?
	caps->latencyms 			= 0;        // unused
	caps->defaultturnssec 		= 10;       // ?
	caps->defaultturnsintransit = 1; 	    // maximum acceptable number of turns in queue?
	return 1;
}

// Register event handler.
BOOL SNetRegisterEventHandler(int evtype, SEVTHANDLER func)
{
	return dvlnet_inst->SNetRegisterEventHandler(*(event_type *)&evtype, func);
}

// Unregister event handler.
BOOL SNetUnregisterEventHandler(int evtype, SEVTHANDLER func)
{
	return dvlnet_inst->SNetUnregisterEventHandler(*(event_type *)&evtype, func);
}

// Get game information.
BOOL SNetGetGameInfo(int type, void *dst, unsigned int length, unsigned int *byteswritten)
{
	DUMMY();
	return true;
}

/**
 * @brief Called by engine for single, called by ui for multi
 * @param provider BNET, IPXN, MODM, SCBL or UDPN
 * @param fileinfo Ignore
 */
int SNetInitializeProvider(
	unsigned long            provider,
	struct _SNETPROGRAMDATA *client_info,
    struct _SNETPLAYERDATA  *user_info,
	struct _SNETUIDATA      *ui_info,
    struct _SNETVERSIONDATA *fileinfo
)
{
	dvlnet_inst = net::abstract_net::make_net(provider);
	return ui_info->selectnamecallback(client_info, user_info, ui_info, fileinfo, provider, NULL, 0, NULL, 0, NULL);
}

// Destroy/free the allocated network resources.
BOOL SNetDestroy()
{
	DUMMY();
	return true;
}

/**
 * @brief Called by engine for single, called by ui for multi
 */
BOOL SNetCreateGame(
	const char *pszGameName,
	const char *pszGamePassword,
	const char *pszGameStatString,
    DWORD 		dwGameType,
    char 	   *GameTemplateData,
    int 		GameTemplateSize,
    int 		playerCount,
    char 	   *creatorName,
    char 	   *a11,
    int        *playerID
)
{
	if (GameTemplateSize != 8)
		ABORT();

	net::buffer_t game_init_info(GameTemplateData, GameTemplateData + GameTemplateSize);
	dvlnet_inst->setup_gameinfo(std::move(game_init_info));

	char addrstr[129] = "0.0.0.0";
	SRegLoadString("dvlnet", "bindaddr", 0, addrstr, 128);
	*playerID = dvlnet_inst->create(addrstr, pszGamePassword);
	return *playerID != -1;
}

BOOL SNetJoinGame(int id, char *pszGameName, char *pszGamePassword, char *playerName, char *userStats, int *playerID)
{
	return (*playerID = dvlnet_inst->join(pszGameName, pszGamePassword)) != -1;
}

BOOL SNetLeaveGame(int type)
{
	return dvlnet_inst->SNetLeaveGame(type);
}

BOOLEAN SNetSetBasePlayer(int) // engine calls this only once with argument 1.
{
	return true;
}

// Drop/disconnect a player from an established game.
BOOL SNetDropPlayer(int playerid, DWORD flags)
{
	return dvlnet_inst->SNetDropPlayer(playerid, flags);
}

/**
 * @brief since we never signal STORM_ERROR_REQUIRES_UPGRADE the engine will not call this function
 */
BOOL SNetPerformUpgrade(DWORD *upgradestatus)
{
	UNIMPLEMENTED();
}

}
