// leaderboard_sync.h
#pragma once

#ifdef CLIENT_DLL

#include "steam/steam_api.h"
#include "tier1/strtools.h" // for V_strncpy

// Tiny helper that mirrors an existing Steam stat onto a Steam leaderboard.
class CLeaderboardSync
{
public:
    CLeaderboardSync();

    // pStatName = name of the Steam stat you already set (e.g. "zombie_kills")
    // pLeaderboardName = the leaderboard name we created in Steamworks (e.g. "zombie_kills")
    void PushStatToLeaderboard(const char* pStatName, const char* pLeaderboardName);

private:
    void OnFindLeaderboard(LeaderboardFindResult_t* pResult, bool bIOFailure);
    void OnScoreUploaded(LeaderboardScoreUploaded_t* pResult, bool bIOFailure);

    // NEW: actually talk to your PHP relay
    void SendToBackend(const char* pLeaderboardName, int score);

    CCallResult<CLeaderboardSync, LeaderboardFindResult_t>   m_CallResultFind;
    CCallResult<CLeaderboardSync, LeaderboardScoreUploaded_t> m_CallResultUpload;

    int  m_iPendingScore;
    char m_szPendingStatName[64];
    char m_szPendingLeaderboardName[64];
    bool m_bRequestPending;
};

// one global instance the rest of the client can use
extern CLeaderboardSync g_LeaderboardSync;

#endif // CLIENT_DLL