#include "cbase.h"
#include "leaderboard_sync.h"
#include "steam/isteamhttp.h"   // NEW

#ifdef CLIENT_DLL

CLeaderboardSync g_LeaderboardSync;

CLeaderboardSync::CLeaderboardSync()
{
    m_iPendingScore = 0;
    m_szPendingStatName[0] = 0;
    m_szPendingLeaderboardName[0] = 0;
    m_bRequestPending = false;
}

void CLeaderboardSync::PushStatToLeaderboard(const char* pStatName, const char* pLeaderboardName)
{
    if (!steamapicontext || !steamapicontext->SteamUserStats())
        return;

    int statVal = 0;
    if (!steamapicontext->SteamUserStats()->GetStat(pStatName, &statVal))
        return;

    // avoid spamming multiple FindLeaderboard calls at once
    if (m_bRequestPending)
        return;

    m_iPendingScore = statVal;
    V_strncpy(m_szPendingStatName, pStatName, sizeof(m_szPendingStatName));
    V_strncpy(m_szPendingLeaderboardName, pLeaderboardName, sizeof(m_szPendingLeaderboardName));

    // still call FindLeaderboard so our log shows if Steam knows about it
    SteamAPICall_t hCall = steamapicontext->SteamUserStats()->FindLeaderboard(pLeaderboardName);
    m_CallResultFind.Set(hCall, this, &CLeaderboardSync::OnFindLeaderboard);
    m_bRequestPending = true;

    Msg("[LB] requested leaderboard '%s' for stat '%s' = %d\n",
        pLeaderboardName, pStatName, statVal);

    // IMPORTANT: send to your trusted PHP right away
    SendToBackend(pLeaderboardName, statVal);
}

void CLeaderboardSync::OnFindLeaderboard(LeaderboardFindResult_t* pResult, bool bIOFailure)
{
    m_bRequestPending = false;

    if (bIOFailure || !pResult->m_bLeaderboardFound)
    {
        Msg("[LB] failed to find leaderboard '%s'\n", m_szPendingLeaderboardName);
        return;
    }

    // we are NOT doing client-side UploadLeaderboardScore anymore
    Msg("[LB] found leaderboard '%s' -> skipping direct Steam upload (trusted backend only)\n",
        m_szPendingLeaderboardName);
}

void CLeaderboardSync::OnScoreUploaded(LeaderboardScoreUploaded_t* /*pResult*/, bool /*bIOFailure*/)
{
    // no-op now
}

void CLeaderboardSync::SendToBackend(const char* pLeaderboardName, int score)
{
    // your actual PHP file (you added index.php — that’s good)
    const char* pszUrl = "http://drn0.site.nfoservers.com/hub/drn0/steam/leaderboard/index.php";

    if (!steamapicontext || !steamapicontext->SteamHTTP() || !steamapicontext->SteamUser() || !steamapicontext->SteamUtils())
        return;

    CSteamID steamID = steamapicontext->SteamUser()->GetSteamID();
    uint32 appID = steamapicontext->SteamUtils()->GetAppID();

    char json[512];
    V_snprintf(
        json, sizeof(json),
        "{ \"steamid\": \"%llu\", \"appid\": %u, \"leaderboard\": \"%s\", \"score\": %d }",
        (unsigned long long)steamID.ConvertToUint64(),   // cast so %llu is happy
        appID,
        pLeaderboardName,
        score
    );

    HTTPRequestHandle hReq = steamapicontext->SteamHTTP()->CreateHTTPRequest(k_EHTTPMethodPOST, pszUrl);
    if (hReq == INVALID_HTTPREQUEST_HANDLE)
        return;

    steamapicontext->SteamHTTP()->SetHTTPRequestHeaderValue(hReq, "Content-Type", "application/json");
    steamapicontext->SteamHTTP()->SetHTTPRequestRawPostBody(
        hReq,
        "application/json",
        (uint8*)json,
        V_strlen(json)
    );

    // fire and forget — we don’t read the response here
    steamapicontext->SteamHTTP()->SendHTTPRequest(hReq, nullptr);

    Msg("[LB] posted to backend for '%s' score=%d\n", pLeaderboardName, score);
}

#endif // CLIENT_DLL