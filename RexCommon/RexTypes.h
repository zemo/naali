// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_RexTypes_h
#define incl_RexTypes_h

#include "CoreStableHeaders.h"
#include "boost/cstdint.hpp"

///\todo Move all these to core framework headers.
using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;
using boost::uint64_t;
using boost::int8_t;
using boost::int16_t;
using boost::int32_t;
using boost::int64_t;

namespace RexTypes
{   

typedef Core::Vector3D<float> Vector3;
typedef Core::Vector3D<double> Vector3d;

///\todo Is this needed? Only one message uses Vector4 (CameraConstraint).
struct Vector4
{
    void Null() { x = y = z = w = 0.f; }
	float x;
	float y;
	float z;
	float w;
};

// Avatar control flags
const int CONTROL_AT_POS_INDEX = 0;
const int CONTROL_AT_NEG_INDEX = 1;
const int CONTROL_LEFT_POS_INDEX = 2;
const int CONTROL_LEFT_NEG_INDEX = 3;
const int CONTROL_UP_POS_INDEX = 4;
const int CONTROL_UP_NEG_INDEX = 5;
const int CONTROL_PITCH_POS_INDEX = 6;
const int CONTROL_PITCH_NEG_INDEX = 7;
const int CONTROL_YAW_POS_INDEX = 8;
const int CONTROL_YAW_NEG_INDEX = 9;
const int CONTROL_FAST_AT_INDEX = 10;
const int CONTROL_FAST_LEFT_INDEX = 11;
const int CONTROL_FAST_UP_INDEX = 12;
const int CONTROL_FLY_INDEX = 13;
const int CONTROL_STOP_INDEX = 14;
const int CONTROL_FINISH_ANIM_INDEX = 15;
const int CONTROL_STAND_UP_INDEX = 16;
const int CONTROL_SIT_ON_GROUND_INDEX = 17;
const int CONTROL_MOUSELOOK_INDEX = 18;
const int CONTROL_NUDGE_AT_POS_INDEX = 19;
const int CONTROL_NUDGE_AT_NEG_INDEX = 20;
const int CONTROL_NUDGE_LEFT_POS_INDEX = 21;
const int CONTROL_NUDGE_LEFT_NEG_INDEX = 22;
const int CONTROL_NUDGE_UP_POS_INDEX = 23;
const int CONTROL_NUDGE_UP_NEG_INDEX = 24;
const int CONTROL_TURN_LEFT_INDEX = 25;
const int CONTROL_TURN_RIGHT_INDEX = 26;
const int CONTROL_AWAY_INDEX = 27;
const int CONTROL_LBUTTON_DOWN_INDEX = 28;
const int CONTROL_LBUTTON_UP_INDEX = 29;
const int CONTROL_ML_LBUTTON_DOWN_INDEX = 30;
const int CONTROL_ML_LBUTTON_UP_INDEX = 31;
const int TOTAL_CONTROLS = 32;

enum ControlFlags
{
    /// Empty flag
    NONE = 0,
    /// Move Forward
    AGENT_CONTROL_AT_POS = 0x1 << CONTROL_AT_POS_INDEX,
    /// Move Backward
    AGENT_CONTROL_AT_NEG = 0x1 << CONTROL_AT_NEG_INDEX,
    /// Move Left
    AGENT_CONTROL_LEFT_POS = 0x1 << CONTROL_LEFT_POS_INDEX,
    /// Move Right
    AGENT_CONTROL_LEFT_NEG = 0x1 << CONTROL_LEFT_NEG_INDEX,
    /// Not Flying: Jump/Flying: Move Up
    AGENT_CONTROL_UP_POS = 0x1 << CONTROL_UP_POS_INDEX,
    /// Not Flying: Croutch/Flying: Move Down
    AGENT_CONTROL_UP_NEG = 0x1 << CONTROL_UP_NEG_INDEX,
    /// Unused
    AGENT_CONTROL_PITCH_POS = 0x1 << CONTROL_PITCH_POS_INDEX,
    /// Unused
    AGENT_CONTROL_PITCH_NEG = 0x1 << CONTROL_PITCH_NEG_INDEX,
    /// Unused
    AGENT_CONTROL_YAW_POS = 0x1 << CONTROL_YAW_POS_INDEX,
    /// Unused
    AGENT_CONTROL_YAW_NEG = 0x1 << CONTROL_YAW_NEG_INDEX,
    /// ORed with AGENT_CONTROL_AT_* if the keyboard is being used
    AGENT_CONTROL_FAST_AT = 0x1 << CONTROL_FAST_AT_INDEX,
    /// ORed with AGENT_CONTROL_LEFT_* if the keyboard is being used
    AGENT_CONTROL_FAST_LEFT = 0x1 << CONTROL_FAST_LEFT_INDEX,
    /// ORed with AGENT_CONTROL_UP_* if the keyboard is being used
    AGENT_CONTROL_FAST_UP = 0x1 << CONTROL_FAST_UP_INDEX,
    /// Fly
    AGENT_CONTROL_FLY = 0x1 << CONTROL_FLY_INDEX,
    /// 
    AGENT_CONTROL_STOP = 0x1 << CONTROL_STOP_INDEX,
    /// Finish our current animation
    AGENT_CONTROL_FINISH_ANIM = 0x1 << CONTROL_FINISH_ANIM_INDEX,
    /// Stand up from the ground or a prim seat
    AGENT_CONTROL_STAND_UP = 0x1 << CONTROL_STAND_UP_INDEX,
    /// Sit on the ground at our current location
    AGENT_CONTROL_SIT_ON_GROUND = 0x1 << CONTROL_SIT_ON_GROUND_INDEX,
    /// Whether mouselook is currently enabled
    AGENT_CONTROL_MOUSELOOK = 0x1 << CONTROL_MOUSELOOK_INDEX,
    /// Legacy, used if a key was pressed for less than a certain amount of time
    AGENT_CONTROL_NUDGE_AT_POS = 0x1 << CONTROL_NUDGE_AT_POS_INDEX,
    /// Legacy, used if a key was pressed for less than a certain amount of time
    AGENT_CONTROL_NUDGE_AT_NEG = 0x1 << CONTROL_NUDGE_AT_NEG_INDEX,
    /// Legacy, used if a key was pressed for less than a certain amount of time
    AGENT_CONTROL_NUDGE_LEFT_POS = 0x1 << CONTROL_NUDGE_LEFT_POS_INDEX,
    /// Legacy, used if a key was pressed for less than a certain amount of time
    AGENT_CONTROL_NUDGE_LEFT_NEG = 0x1 << CONTROL_NUDGE_LEFT_NEG_INDEX,
    /// Legacy, used if a key was pressed for less than a certain amount of time
    AGENT_CONTROL_NUDGE_UP_POS = 0x1 << CONTROL_NUDGE_UP_POS_INDEX,
    /// Legacy, used if a key was pressed for less than a certain amount of time
    AGENT_CONTROL_NUDGE_UP_NEG = 0x1 << CONTROL_NUDGE_UP_NEG_INDEX,
    /// 
    AGENT_CONTROL_TURN_LEFT = 0x1 << CONTROL_TURN_LEFT_INDEX,
    /// 
    AGENT_CONTROL_TURN_RIGHT = 0x1 << CONTROL_TURN_RIGHT_INDEX,
    /// Set when the avatar is idled or set to away. Note that the away animation is 
    /// activated separately from setting this flag
    AGENT_CONTROL_AWAY = 0x1 << CONTROL_AWAY_INDEX,
    /// 
    AGENT_CONTROL_LBUTTON_DOWN = 0x1 << CONTROL_LBUTTON_DOWN_INDEX,
    /// 
    AGENT_CONTROL_LBUTTON_UP = 0x1 << CONTROL_LBUTTON_UP_INDEX,
    /// 
    AGENT_CONTROL_ML_LBUTTON_DOWN = 0x1 << CONTROL_ML_LBUTTON_DOWN_INDEX,
    /// 
    AGENT_CONTROL_ML_LBUTTON_UP = 0x1 << CONTROL_ML_LBUTTON_UP_INDEX
};

}

#endif
