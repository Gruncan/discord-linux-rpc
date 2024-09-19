#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "discord_rpc.h"

static const char* APPLICATION_ID = "1286034210838155325";
static int64_t StartTime;
static int SendPresence = 1;

static int detailsProgress = 0;
static int detailsFlipper = 1;

#define PROGRESS_BAR_WIDTH  20
#define PROGRESS_CHAR       '-'
#define PROGRESS_CHAR_START '<'
#define PROGRESS_CHAR_END   '>'
#define NON_PROGRESS_CHAR   '\32'
#define PROGRESS_INCREMENT  3

void signalHandler(const int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        printf("Closing connections...\n");
        Discord_Shutdown();
        exit(0);
    }
}


static void updateDiscordPresence()
{
    if (SendPresence) {
        char buffer[256];
        for(int i = 1; i <= PROGRESS_BAR_WIDTH; i++) {
            if (i == detailsProgress) {
                buffer[i] = PROGRESS_CHAR_START;
            } else if(i == detailsProgress + 1) {
                buffer[i] = PROGRESS_CHAR;
            } else if (i == detailsProgress + 2) {
                buffer[i] = PROGRESS_CHAR_END;
            } else {
                buffer[i] = NON_PROGRESS_CHAR;
            }
        }
        buffer[0] = '[';
        buffer[PROGRESS_BAR_WIDTH+1] = ']';
        buffer[PROGRESS_BAR_WIDTH+2] = '\0';

        if (detailsProgress == PROGRESS_BAR_WIDTH - 2) {
            detailsFlipper -= 1;
        }else if (detailsProgress == 2 && detailsFlipper != 1) {
            detailsFlipper += 1;
        }
        detailsProgress += detailsFlipper;

        DiscordRichPresence discordPresence;
        memset(&discordPresence, 0, sizeof(discordPresence));
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
    printf("\nDiscord: join (%s)\n", request->username);
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

    while(1) {
        updateDiscordPresence();
        sleep(PROGRESS_INCREMENT);
    }
}
