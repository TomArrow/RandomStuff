/******************************************************************************

Welcome to GDB Online.
GDB online is an online compiler and debugger tool for C, C++, Python, Java, PHP, Ruby, Perl,
C#, OCaml, VB, Swift, Pascal, Fortran, Haskell, Objective-C, Assembly, HTML, CSS, JS, SQLite, Prolog.
Code, Compile, Run and Debug online from anywhere in world.

*******************************************************************************/
#include <stdio.h>
#include <exception>
#include <stdexcept>
#include <string.h>
#include <array>
typedef unsigned char 		byte;

#define	GENTITYNUM_BITS		10		// don't need to send any more
#define	MAX_GENTITIES		(1<<GENTITYNUM_BITS)
typedef enum { qfalse, qtrue }	qboolean;
typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];
typedef enum {
	TR_STATIONARY,
	TR_INTERPOLATE,				// non-parametric, but interpolate between snapshots
	TR_LINEAR,
	TR_LINEAR_STOP,
	TR_SINE,					// value = base + sin( time / duration ) * delta
	TR_GRAVITY
} trType_t;

typedef struct {
	trType_t	trType;
	int		trTime;
	int		trDuration;			// if non 0, trTime + trDuration = stop time
	vec3_t	trBase;
	vec3_t	trDelta;			// velocity, etc
} trajectory_t;
typedef struct entityState_s {
	int		number;			// entity index
	int		eType;			// entityType_t
	int		eFlags;

	trajectory_t	pos;	// for calculating position
	trajectory_t	apos;	// for calculating angles

	int		time;
	int		time2;

	vec3_t	origin;
	vec3_t	origin2;

	vec3_t	angles;
	vec3_t	angles2;

	//rww - these were originally because we shared g2 info client and server side. Now they
	//just get used as generic values everywhere.
	int		bolt1;
	int		bolt2;

	//rww - this is necessary for determining player visibility during a jedi mindtrick
	int		trickedentindex; //0-15
	int		trickedentindex2; //16-32
	int		trickedentindex3; //33-48
	int		trickedentindex4; //49-64

	float	speed;

	int		fireflag;

	int		genericenemyindex;

	int		activeForcePass;

	int		emplacedOwner;

	int		otherEntityNum;	// shotgun sources, etc
	int		otherEntityNum2;

	int		groundEntityNum;	// -1 = in air

	int		constantLight;	// r + (g<<8) + (b<<16) + (intensity<<24)
	int		loopSound;		// constantly loop this sound

	int		modelGhoul2;
	int		g2radius;
	int		modelindex;
	int		modelindex2;
	int		clientNum;		// 0 to (MAX_CLIENTS - 1), for players and corpses
	int		frame;

	qboolean	saberInFlight;
	int			saberEntityNum;
	int			saberMove;
	int			forcePowersActive;

	qboolean	isJediMaster;

	int		solid;			// for client side prediction, trap_linkentity sets this properly

	int		event;			// impulse events -- muzzle flashes, footsteps, etc
	int		eventParm;

	// so crosshair knows what it's looking at
	int			owner;
	int			teamowner;
	qboolean	shouldtarget;

	// for players
	int		powerups;		// bit flags
	int		weapon;			// determines weapon and flash model, etc
	int		legsAnim;		// mask off ANIM_TOGGLEBIT
	int		torsoAnim;		// mask off ANIM_TOGGLEBIT

	int		forceFrame;		//if non-zero, force the anim frame

	int		generic1;


	// JKA-specific:
	int		eFlags2;		// EF2_??? used much less frequently
	qboolean	loopIsSoundset; //qtrue if the loopSound index is actually a soundset index
	int		soundSetIndex;
	int			saberHolstered;//sent in only only 2 bits - should be 0, 1 or 2
	qboolean	isPortalEnt; //this needs to be seperate for all entities I guess, which is why I couldn't reuse another value.
	qboolean	legsFlip; //set to opposite when the same anim needs restarting, sent over in only 1 bit. Cleaner and makes porting easier than having that god forsaken ANIM_TOGGLEBIT.
	qboolean	torsoFlip;
	int		heldByClient; //can only be a client index - this client should be holding onto my arm using IK stuff.
	int		ragAttach; //attach to ent while ragging
	int		iModelScale; //rww - transfer a percentage of the normal scale in a single int instead of 3 x-y-z scale values
	int		brokenLimbs;
	int		boltToPlayer; //set to index of a real client+1 to bolt the ent to that client. Must be a real client, NOT an NPC.
	//for looking at an entity's origin (NPCs and players)
	qboolean	hasLookTarget;
	int			lookTarget;
	int			customRGBA[4];
	//I didn't want to do this, but I.. have no choice. However, we aren't setting this for all ents or anything,
	//only ones we want health knowledge about on cgame (like siege objective breakables) -rww
	int			health;
	int			maxhealth; //so I know how to draw the stupid health bar
	//NPC-SPECIFIC FIELDS
	//------------------------------------------------------------
	int		npcSaber1;
	int		npcSaber2;
	//index values for each type of sound, gets the folder the sounds
	//are in. I wish there were a better way to do this,
	int		csSounds_Std;
	int		csSounds_Combat;
	int		csSounds_Extra;
	int		csSounds_Jedi;
	int		surfacesOn; //a bitflag of corresponding surfaces from a lookup table. These surfaces will be forced on.
	int		surfacesOff; //same as above, but forced off instead.
	//Allow up to 4 PCJ lookup values to be stored here.
	//The resolve to configstrings which contain the name of the
	//desired bone.
	int		boneIndex1;
	int		boneIndex2;
	int		boneIndex3;
	int		boneIndex4;
	//packed with x, y, z orientations for bone angles
	int		boneOrient;
	//I.. feel bad for doing this, but NPCs really just need to
	//be able to control this sort of thing from the server sometimes.
	//At least it's at the end so this stuff is never going to get sent
	//over for anything that isn't an NPC.
	vec3_t	boneAngles1; //angles of boneIndex1
	vec3_t	boneAngles2; //angles of boneIndex2
	vec3_t	boneAngles3; //angles of boneIndex3
	vec3_t	boneAngles4; //angles of boneIndex4
	int		NPC_class; //we need to see what it is on the client for a few effects.
	//If non-0, this is the index of the vehicle a player/NPC is riding.
	int		m_iVehicleNum;
	//rww - spare values specifically for use by mod authors.
	//See netf_overrides.txt if you want to increase the send
	//amount of any of these above 1 bit.
	int			userInt1;
	int			userInt2;
	int			userInt3;
	float		userFloat1;
	float		userFloat2;
	float		userFloat3;
	vec3_t		userVec1;
	vec3_t		userVec2;


	// RTCW (according to uber demo tools)
	int dl_intensity;  /* used for coronas */
	int eventSequence; /* pmove generated events */
	int events[4];
	int eventParms[4];
	int density;       /* for particle effects */
	/* to pass along additional information for damage effects for players */
	/* also used for cursorhints for non-player entities */
	int dmgFlags;
	int onFireStart;
	int onFireEnd;
	int aiChar;
	int teamNum;
	int effect1Time;
	int effect2Time;
	int effect3Time;
	int aiState;
	int animMovetype;  /* clients can't derive movetype of other clients for anim scripting system */


	// Quake Live (according to uber demo tools)
	int pos_gravity;  /* part of idEntityStateBase::pos trajectory */
	int apos_gravity; /* part of idEntityStateBase::apos trajectory */
	int jumpTime;
	int doubleJumped; /* qboolean */
	//int health; // Already exists through jka
	int armor;
	int location;


	// JK2 SP Stuff
	int		modelindex3;
	int		legsAnimTimer;	// don't change low priority animations on legs until this runs out
	int		torsoAnimTimer;	// don't change low priority animations on torso until this runs out
	int		scale;			//Scale players
	qboolean	saberActive;
	int		vehicleModel;	// For overriding your playermodel with a drivable vehicle
	vec3_t	modelScale;		// used to scale models in any axis
	//Ghoul2 stuff for jk2sp:
	int		radius;			// used for culling all the ghoul models attached to this ent NOTE - this is automatically scaled by Ghoul2 if/when you scale the model. This is a 100% size value
	int		boltInfo;		// info used for bolting entities to Ghoul2 models - NOT used for bolting ghoul2 models to themselves, more for stuff like bolting effects to ghoul2 models




} entityState_t;

typedef struct {
	const char* name{};
	size_t		offset{};
	int		bits{};		// 0 = float
} netField_t;


// using the stringizing operator to save typing...
//#define	NETF(x) (size_t)&((entityState_t*)0)->x
//#define	NETF(x) #x,((size_t)&reinterpret_cast<char const volatile&>((((entityState_t*)0)->x)))
#define myoffsetof(s,m) ((size_t)&reinterpret_cast<char const volatile&>((((s*)0)->m)))
#define	NETF(x) #x,myoffsetof(entityState_t,x)
//#define	NETF(x) #x,2

//constexpr size_t test = (size_t)&((entityState_t*)0)->eFlags;
template<typename T, auto MPtr>
static constexpr size_t offsetof_ptr()
{
	return ((::size_t) & reinterpret_cast<char const volatile&>((((T*)0)->*MPtr)));
}

constexpr size_t test = offsetof(entityState_t,eFlags);

static const constexpr netField_t	entityStateFieldsJKA[] = // Apply to both DM_25 and DM_26 from what I can tell
{
{ NETF(pos.trTime), 32 },
{ NETF(pos.trBase[1]), 0 },
{ NETF(pos.trBase[0]), 0 },
{ NETF(apos.trBase[1]), 0 },
{ NETF(pos.trBase[2]), 0 },
{ NETF(apos.trBase[0]), 0 },
{ NETF(pos.trDelta[0]), 0 },
{ NETF(pos.trDelta[1]), 0 },
{ NETF(eType), 8 },
{ NETF(angles[1]), 0 },
{ NETF(pos.trDelta[2]), 0 },
{ NETF(origin[0]), 0 },
{ NETF(origin[1]), 0 },
{ NETF(origin[2]), 0 },
// does this need to be 8 bits?
{ NETF(weapon), 8 },
{ NETF(apos.trType), 8 },
// changed from 12 to 16
{ NETF(legsAnim), 16 },			// Maximum number of animation sequences is 2048.  Top bit is reserved for the togglebit
// suspicious
{ NETF(torsoAnim), 16 },		// Maximum number of animation sequences is 2048.  Top bit is reserved for the togglebit
// large use beyond GENTITYNUM_BITS - should use generic1 insead
{ NETF(genericenemyindex), 32 }, //Do not change to GENTITYNUM_BITS, used as a time offset for seeker
{ NETF(eFlags), 32 },
{ NETF(pos.trDuration), 32 },
// might be able to reduce
{ NETF(teamowner), 8 },
{ NETF(groundEntityNum), GENTITYNUM_BITS },
{ NETF(pos.trType), 8 },
{ NETF(angles[2]), 0 },
{ NETF(angles[0]), 0 },
{ NETF(solid), 24 },
// flag states barely used - could be moved elsewhere
{ NETF(fireflag), 2 },
{ NETF(event), 10 },			// There is a maximum of 256 events (8 bits transmission, 2 high bits for uniqueness)
// used mostly for players and npcs - appears to be static / never changing
{ NETF(customRGBA[3]), 8 }, //0-255
// used mostly for players and npcs - appears to be static / never changing
{ NETF(customRGBA[0]), 8 }, //0-255
// only used in fx system (which rick did) and chunks
{ NETF(speed), 0 },
// why are npc's clientnum's that big?
{ NETF(clientNum), GENTITYNUM_BITS }, //with npc's clientnum can be > MAX_CLIENTS so use entnum bits now instead.
{ NETF(apos.trBase[2]), 0 },
{ NETF(apos.trTime), 32 },
// used mostly for players and npcs - appears to be static / never changing
{ NETF(customRGBA[1]), 8 }, //0-255
// used mostly for players and npcs - appears to be static / never changing
{ NETF(customRGBA[2]), 8 }, //0-255
// multiple meanings
{ NETF(saberEntityNum), GENTITYNUM_BITS },
// could probably just eliminate and assume a big number
{ NETF(g2radius), 8 },
{ NETF(otherEntityNum2), GENTITYNUM_BITS },
// used all over the place
{ NETF(owner), GENTITYNUM_BITS },
{ NETF(modelindex2), 8 },
// why was this changed from 0 to 8 ?
{ NETF(eventParm), 8 },
// unknown about size?
{ NETF(saberMove), 8 },
{ NETF(apos.trDelta[1]), 0 },
{ NETF(boneAngles1[1]), 0 },
// why raised from 8 to -16?
{ NETF(modelindex), -16 },
// barely used, could probably be replaced
{ NETF(emplacedOwner), 32 }, //As above, also used as a time value (for electricity render time)
{ NETF(apos.trDelta[0]), 0 },
{ NETF(apos.trDelta[2]), 0 },
// shouldn't these be better off as flags?  otherwise, they may consume more bits this way
{ NETF(torsoFlip), 1 },
{ NETF(angles2[1]), 0 },
// used mostly in saber and npc
{ NETF(lookTarget), GENTITYNUM_BITS },
{ NETF(origin2[2]), 0 },
// randomly used, not sure why this was used instead of svc_noclient
//	if (cent->currentState.modelGhoul2 == 127)
//	{ //not ready to be drawn or initialized..
//		return;
//	}
{ NETF(modelGhoul2), 8 },
{ NETF(loopSound), 8 },
{ NETF(origin2[0]), 0 },
// multiple purpose bit flag
{ NETF(shouldtarget), 1 },
// widely used, does not appear that they have to be 16 bits
{ NETF(trickedentindex), 16 }, //See note in PSF
{ NETF(otherEntityNum), GENTITYNUM_BITS },
{ NETF(origin2[1]), 0 },
{ NETF(time2), 32 },
{ NETF(legsFlip), 1 },
// fully used
{ NETF(bolt2), GENTITYNUM_BITS },
{ NETF(constantLight), 32 },
{ NETF(time), 32 },
// why doesn't lookTarget just indicate this?
{ NETF(hasLookTarget), 1 },
{ NETF(boneAngles1[2]), 0 },
// used for both force pass and an emplaced gun - gun is just a flag indicator
{ NETF(activeForcePass), 6 },
// used to indicate health
{ NETF(health), 10 }, //if something's health exceeds 1024, then.. too bad!
// appears to have multiple means, could be eliminated by indicating a sound set differently
{ NETF(loopIsSoundset), 1 },
{ NETF(saberHolstered), 2 },
//NPC-SPECIFIC:
// both are used for NPCs sabers, though limited
{ NETF(npcSaber1), 9 },
{ NETF(maxhealth), 10 },
{ NETF(trickedentindex2), 16 },
// appear to only be 18 powers?
{ NETF(forcePowersActive), 32 },
// used, doesn't appear to be flexible
{ NETF(iModelScale), 10 }, //0-1024 (guess it's gotta be increased if we want larger allowable scale.. but 1024% is pretty big)
// full bits used
{ NETF(powerups), 16 },
// can this be reduced?
{ NETF(soundSetIndex), 8 }, //rww - if MAX_AMBIENT_SETS is changed from 256, REMEMBER TO CHANGE THIS
// looks like this can be reduced to 4? (ship parts = 4, people parts = 2)
{ NETF(brokenLimbs), 8 }, //up to 8 limbs at once (not that that many are used)
{ NETF(csSounds_Std), 8 }, //soundindex must be 8 unless max sounds is changed
// used extensively
{ NETF(saberInFlight), 1 },
{ NETF(angles2[0]), 0 },
{ NETF(frame), 16 },
{ NETF(angles2[2]), 0 },
// why not use torsoAnim and set a flag to do the same thing as forceFrame (saberLockFrame)
{ NETF(forceFrame), 16 }, //if you have over 65536 frames, then this will explode. Of course if you have that many things then lots of things will probably explode.
{ NETF(generic1), 8 },
// do we really need 4 indexes?
{ NETF(boneIndex1), 6 }, //up to 64 bones can be accessed by this indexing method
// only 54 classes, could cut down 2 bits
{ NETF(NPC_class), 8 },
{ NETF(apos.trDuration), 32 },
// there appears to be only 2 different version of parms passed - a flag would better be suited
{ NETF(boneOrient), 9 }, //3 bits per orientation dir
// this looks to be a single bit flag
{ NETF(bolt1), 8 },
{ NETF(trickedentindex3), 16 },
// in use for vehicles
{ NETF(m_iVehicleNum), GENTITYNUM_BITS }, // 10 bits fits all possible entity nums (2^10 = 1024). - AReis
{ NETF(trickedentindex4), 16 },
// but why is there an opposite state of surfaces field?
{ NETF(surfacesOff), 32 },
{ NETF(eFlags2), 10 },
// should be bit field
{ NETF(isJediMaster), 1 },
// should be bit field
{ NETF(isPortalEnt), 1 },
// possible multiple definitions
{ NETF(heldByClient), 6 },
// this does not appear to be used in any production or non-cheat fashion - REMOVE
{ NETF(ragAttach), GENTITYNUM_BITS },
// used only in one spot for seige
{ NETF(boltToPlayer), 6 },
{ NETF(npcSaber2), 9 },
{ NETF(csSounds_Combat), 8 },
{ NETF(csSounds_Extra), 8 },
{ NETF(csSounds_Jedi), 8 },
// used only for surfaces on NPCs
{ NETF(surfacesOn), 32 }, //allow up to 32 surfaces in the bitflag
{ NETF(boneIndex2), 6 },
{ NETF(boneIndex3), 6 },
{ NETF(boneIndex4), 6 },
{ NETF(boneAngles1[0]), 0 },
{ NETF(boneAngles2[0]), 0 },
{ NETF(boneAngles2[1]), 0 },
{ NETF(boneAngles2[2]), 0 },
{ NETF(boneAngles3[0]), 0 },
{ NETF(boneAngles3[1]), 0 },
{ NETF(boneAngles3[2]), 0 },
{ NETF(boneAngles4[0]), 0 },
{ NETF(boneAngles4[1]), 0 },
{ NETF(boneAngles4[2]), 0 },

//rww - for use by mod authors only
// These don't exist on Xbox btw:
{ NETF(userInt1), 1 },
{ NETF(userInt2), 1 },
{ NETF(userInt3), 1 },
{ NETF(userFloat1), 1 },
{ NETF(userFloat2), 1 },
{ NETF(userFloat3), 1 },
{ NETF(userVec1[0]), 1 },
{ NETF(userVec1[1]), 1 },
{ NETF(userVec1[2]), 1 },
{ NETF(userVec2[0]), 1 },
{ NETF(userVec2[1]), 1 },
{ NETF(userVec2[2]), 1 }
};

constexpr bool is_digit(char c) {
    return c <= '9' && c >= '0';
}

constexpr int stoi_impl(const char* str, int value = 0) {
    return *str ?
            is_digit(*str) ?
                stoi_impl(str + 1, (*str - '0') + value * 10)
                : throw "compile-time-error: not a digit"
            : value;
}

constexpr int stoi(const char* str) {
    return stoi_impl(str);
}

/*
static constexpr void MSG_CheckNETFPSFOverridesConstExpr(const char* overrideFile, int len, netField_t* netFields, int numFields)
{
	//char overrideFile[4096]{ };
	char entryName[4096]{ };
	char bits[4096]{ };
	int ibits=0;
	int i = 0;
	int j=0;

	//Now parse through. Lines beginning with ; are disabled.
	while (overrideFile[i])
	{
		if (overrideFile[i] == ';')
		{ //parse to end of the line
			while (overrideFile[i] != '\n')
			{
				i++;
			}
		}

		if (overrideFile[i] != ';' &&
			overrideFile[i] != '\n' &&
			overrideFile[i] != '\r')
		{ //on a valid char I guess, parse it
			j = 0;

			while (overrideFile[i] && overrideFile[i] != ',')
			{
				entryName[j] = overrideFile[i];
				j++;
				i++;
			}
			entryName[j] = 0;

			if (!overrideFile[i])
			{ //just give up, this shouldn't happen
				throw std::logic_error("ERROR: Parsing error for overrides");
				return;
			}

			while (overrideFile[i] == ',' || overrideFile[i] == ' ')
			{ //parse to the start of the value
				i++;
			}

			j = 0;
			while (overrideFile[i] != '\n' && overrideFile[i] != '\r')
			{ //now read the value in
				bits[j] = overrideFile[i];
				j++;
				i++;
			}
			bits[j] = 0;

			if (bits[0])
			{
				if (!strcmp(bits, "GENTITYNUM_BITS"))
				{ //special case
					ibits = GENTITYNUM_BITS;
				}
				else
				{
					ibits = stoi(bits);
				}

				j = 0;

				//Now go through all the fields and see if we can find a match
				while (j < numFields)
				{

					if (!strcmp(netFields[j].name, entryName))
					{ //found it, set the bits
						netFields[j].bits = ibits;
						break;
					}
					j++;
				}

				if (j == numFields)
				{ //failed to find the value
					//throw std::logic_error("ERROR: Override parsing: Value is not valid.");
					throw std::logic_error("test");
					//Com_Printf("WARNING: Value '%s' from %s is not valid\n", entryName, fileName);
				}
			}
			else
			{ //also should not happen
				throw std::logic_error("ERROR: Override parsing: Parsing error.");
				return;
			}
		}

		i++;
	}
}*/



constexpr auto myArray{ []() constexpr {
	constexpr size_t size = sizeof(entityStateFieldsJKA) / sizeof(entityStateFieldsJKA[0]);
	std::array<netField_t, size> newNetFields{};
	for (int i = 0; i < size; i++) {
		newNetFields[i] = entityStateFieldsJKA[i];
	}
	constexpr const char* overrides = 
		"userInt1, 32\n"
		"userInt2, 32\n"
		"userInt3, 32\n"
		"userFloat1, 0\n"
		"userFloat2, 0\n";
	//MSG_CheckNETFPSFOverridesConstExpr(overrides,sizeof(overrides), newNetFields.data(),size);
	return newNetFields;
}() };






int main()
{
    
    printf("Hello World");

    return 0;
}
