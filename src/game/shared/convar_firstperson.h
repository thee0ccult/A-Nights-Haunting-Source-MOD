#include "cbase.h"
static ConVar cl_legs("cl_legs", "1", FCVAR_CHEAT, "Enable or disable player leg rendering", true, 0, true, 1);
static ConVar cl_legs_origin_shift("cl_legs_origin_shift", "-17", FCVAR_CHEAT, "Amount in game units to shift the player model relative to the direction the player is facing");
static ConVar cl_legs_clip_height("cl_legs_clip_height", "0", FCVAR_CHEAT, "Amount in game units of the player model to render up to [0 = disable]", true, 0, false, 0);