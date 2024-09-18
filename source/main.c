/*
    This is a simple example in C of using the rich presence API asynchronously.
*/

#define _CRT_SECURE_NO_WARNINGS /* thanks Microsoft */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "discord_rpc.h"

static const char* APPLICATION_ID = "1286034210838155325";
static int64_t StartTime;
static int SendPresence = 1;


void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        printf("Closing connections...\n");
        Discord_Shutdown();
        exit(0);
    }
}

static int prompt(char* line, size_t size)
{
    int res;
    char* nl;
    printf("\n> ");
    fflush(stdout);
    res = fgets(line, size, stdin) ? 1 : 0;
    line[size - 1] = 0;
    nl = strchr(line, '\n');
    if (nl) {
        *nl = 0;
    }
    return res;
}

static void updateDiscordPresence()
{
    if (SendPresence) {
        char buffer[256];
        DiscordRichPresence discordPresence;
        memset(&discordPresence, 0, sizeof(discordPresence));
        sprintf(buffer, "Jamezeboyy L count: 99");
        discordPresence.details = buffer;
        discordPresence.startTimestamp = StartTime;
        discordPresence.endTimestamp = time(0) + 5 * 60;
        discordPresence.smallImageKey = "mcmichaellarge";
        discordPresence.partyId = "party1234";
        discordPresence.partySize = INT_MAX;
        discordPresence.partyMax = INT_MAX;
        discordPresence.partyPrivacy = DISCORD_PARTY_PUBLIC;
        discordPresence.matchSecret = "xyzzy";
        discordPresence.joinSecret = "join";
        discordPresence.spectateSecret = "look";
        discordPresence.instance = 0;
        Discord_UpdatePresence(&discordPresence);
    }
    else {
        Discord_ClearPresence();
    }
}

static void handleDiscordReady(const DiscordUser* connectedUser)
{
    printf("\nDiscord: connected to user %s#%s - %s\n",
           connectedUser->username,
           connectedUser->discriminator,
           connectedUser->userId);
}

static void handleDiscordDisconnected(int errcode, const char* message)
{
    printf("\nDiscord: disconnected (%d: %s)\n", errcode, message);
}

static void handleDiscordError(int errcode, const char* message)
{
    printf("\nDiscord: error (%d: %s)\n", errcode, message);
}

static void handleDiscordJoin(const char* secret)
{
    printf("\nDiscord: join (%s)\n", secret);
}

static void handleDiscordSpectate(const char* secret)
{
    printf("\nDiscord: spectate (%s)\n", secret);
}

static void handleDiscordJoinRequest(const DiscordUser* request)
{
    int response = -1;
    char yn[4];
    printf("\nDiscord: join request from %s#%s - %s\n",
           request->username,
           request->discriminator,
           request->userId);
    do {
        printf("Accept? (y/n)");
        if (!prompt(yn, sizeof(yn))) {
            break;
        }

        if (!yn[0]) {
            continue;
        }

        if (yn[0] == 'y') {
            response = DISCORD_REPLY_YES;
            break;
        }

        if (yn[0] == 'n') {
            response = DISCORD_REPLY_NO;
            break;
        }
    } while (1);
    if (response != -1) {
        Discord_Respond(request->userId, response);
    }
}

static void discordInit()
{
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    handlers.ready = handleDiscordReady;
    handlers.disconnected = handleDiscordDisconnected;
    handlers.errored = handleDiscordError;
    handlers.joinGame = handleDiscordJoin;
    handlers.spectateGame = handleDiscordSpectate;
    handlers.joinRequest = handleDiscordJoinRequest;
    Discord_Initialize(APPLICATION_ID, &handlers, 1, NULL);
}


int main(void)
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    printf("Initializing discord RPC...\n");
    discordInit();

    updateDiscordPresence();
    printf("Presence updated.\n");

    while(1)
    {

    }

    Discord_Shutdown();
    return 0;
}
