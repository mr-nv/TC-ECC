//==================================================================================
#ifndef _CHOOKING_
#define _CHOOKING_
//==================================================================================
#include "engine/wrect.h"
#include "engine/cl_dll.h"
#include "engine/cdll_int.h"
#include "engine/const.h"
#include "engine/progdefs.h"
#include "engine/eiface.h"
#include "engine/edict.h"
#include "engine/studio_event.h"
#include "engine/entity_types.h"
#include "engine/r_efx.h"
#include "engine/pmtrace.h"
#include "common/ref_params.h"
#include "common/screenfade.h"
#include "common/event_api.h"
#include "common/com_model.h"
#include "misc/parsemsg.h"
#include "misc/sprites.h"
#include "misc/r_studioint.h"
#include "engine/triangleapi.h"
#include "engine/pm_defs.h"
#include "engine/studio.h"
#include "msghook.h"
//==================================================================================
typedef int					( *INITIALIZE_FUNCTION )					( struct cl_enginefuncs_s*, int );
typedef void				( *HUD_INIT_FUNCTION )						( void );
typedef int					( *HUD_VIDINIT_FUNCTION )					( void );
typedef int					( *HUD_REDRAW_FUNCTION )					( float, int );
typedef int					( *HUD_UPDATECLIENTDATA_FUNCTION )			( struct client_data_s*, float );
typedef void				( *HUD_RESET_FUNCTION )						( void );
typedef void				( *HUD_CLIENTMOVE_FUNCTION )				( struct playermove_s *ppmove, qboolean server );
typedef void				( *HUD_CLIENTMOVEINIT_FUNCTION )			( struct playermove_s *ppmove );
typedef char				( *HUD_TEXTURETYPE_FUNCTION )				( char *name );
typedef void				( *HUD_IN_ACTIVATEMOUSE_FUNCTION )			( void );
typedef void				( *HUD_IN_DEACTIVATEMOUSE_FUNCTION )		( void );
typedef void				( *HUD_IN_MOUSEEVENT_FUNCTION )				( int mstate );
typedef void				( *HUD_IN_CLEARSTATES_FUNCTION )			( void );
typedef void				( *HUD_IN_ACCUMULATE_FUNCTION )				( void );
typedef void				( *HUD_CL_CREATEMOVE_FUNCTION )				( float frametime, struct usercmd_s *cmd, int active );
typedef int					( *HUD_CL_ISTHIRDPERSON_FUNCTION )			( void );
typedef void				( *HUD_CL_GETCAMERAOFFSETS_FUNCTION)		( float *offs );
typedef struct kbutton_s *	( *HUD_KB_FIND_FUNCTION )					( const char *name );
typedef void				( *HUD_CAMTHINK_FUNCTION )					( void );
typedef void				( *HUD_V_CALCREFDEF_FUNCTION )				( struct ref_params_s *pparams );
typedef int					( *HUD_ADDENTITY_FUNCTION )					( int type, struct cl_entity_s *ent, const char *modelname );
typedef void				( *HUD_CREATEENTITIES_FUNCTION )			( void );
typedef void				( *HUD_DRAWNORMALTRIS_FUNCTION )			( void );
typedef void				( *HUD_DRAWTRANSTRIS_FUNCTION )				( void );
typedef void				( *HUD_STUDIOEVENT_FUNCTION )				( const struct mstudioevent_s *event, const struct cl_entity_s *entity );
typedef void				( *HUD_POSTRUNCMD_FUNCTION )				( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed );
typedef void				( *HUD_SHUTDOWN_FUNCTION )					( void );
typedef void				( *HUD_TXFERLOCALOVERRIDES_FUNCTION )		( struct entity_state_s *state, const struct clientdata_s *client );
typedef void				( *HUD_PROCESSPLAYERSTATE_FUNCTION )		( struct entity_state_s *dst, const struct entity_state_s *src );	
typedef void				( *HUD_TXFERPREDICTIONDATA_FUNCTION )		( struct entity_state_s *ps, const struct entity_state_s *pps, struct clientdata_s *pcd, const struct clientdata_s *ppcd, struct weapon_data_s *wd, const struct weapon_data_s *pwd );
typedef void				( *HUD_DEMOREAD_FUNCTION )					( int size, unsigned char *buffer );
typedef int					( *HUD_CONNECTIONLESS_FUNCTION )			( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size );
typedef int					( *HUD_GETHULLBOUNDS_FUNCTION )				( int hullnumber, float *mins, float *maxs );
typedef void				( *HUD_FRAME_FUNCTION )						( double );
typedef int					( *HUD_KEY_EVENT_FUNCTION )					( int eventcode, int keynum, const char *pszCurrentBinding );
typedef void				( *HUD_TEMPENTUPDATE_FUNCTION )				( double frametime, double client_time, double cl_gravity, struct tempent_s **ppTempEntFree, struct tempent_s **ppTempEntActive, 	int ( *Callback_AddVisibleEntity )( struct cl_entity_s *pEntity ),	void ( *Callback_TempEntPlaySound )( struct tempent_s *pTemp, float damp ) );
typedef struct cl_entity_s *( *HUD_GETUSERENTITY_FUNCTION )				( int index );
typedef void				( *HUD_VOICESTATUS_FUNCTION )				( int entindex, qboolean bTalking );
typedef void				( *HUD_DIRECTORMESSAGE_FUNCTION )			( int iSize, void *pbuf );
typedef int					( *HUD_STUDIO_INTERFACE_FUNCTION )			( int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio );
typedef void				( *HUD_CHATINPUTPOSITION_FUNCTION )			( int *x, int *y );
typedef int					( *HUD_GETPLAYERTEAM_FUNCTION )				( int iplayer );
typedef void				( *CLIENTFACTORY )							( );
//==================================================================================
// Client API  structure. Holds all functions exported by client.
//==================================================================================
typedef struct _CLIENT_
{
	INITIALIZE_FUNCTION								Initialize;
	HUD_INIT_FUNCTION								HUD_Init;
	HUD_VIDINIT_FUNCTION							HUD_VidInit;
	HUD_REDRAW_FUNCTION								HUD_Redraw;
	HUD_UPDATECLIENTDATA_FUNCTION					HUD_UpdateClientData;
	HUD_RESET_FUNCTION								HUD_Reset;
	HUD_CLIENTMOVE_FUNCTION							HUD_PlayerMove;
	HUD_CLIENTMOVEINIT_FUNCTION						HUD_PlayerMoveInit;
	HUD_TEXTURETYPE_FUNCTION						HUD_PlayerMoveTexture;
	HUD_IN_ACTIVATEMOUSE_FUNCTION					IN_ActivateMouse;
	HUD_IN_DEACTIVATEMOUSE_FUNCTION					IN_DeactivateMouse;
	HUD_IN_MOUSEEVENT_FUNCTION						IN_MouseEvent;
	HUD_IN_CLEARSTATES_FUNCTION						IN_ClearStates;
	HUD_IN_ACCUMULATE_FUNCTION						IN_Accumulate;
	HUD_CL_CREATEMOVE_FUNCTION						CL_CreateMove;
	HUD_CL_ISTHIRDPERSON_FUNCTION					CL_IsThirdPerson;
	HUD_CL_GETCAMERAOFFSETS_FUNCTION				CL_CameraOffset;
	HUD_KB_FIND_FUNCTION							KB_Find;
	HUD_CAMTHINK_FUNCTION							CAM_Think;
	HUD_V_CALCREFDEF_FUNCTION						V_CalcRefdef;
	HUD_ADDENTITY_FUNCTION							HUD_AddEntity;
	HUD_CREATEENTITIES_FUNCTION						HUD_CreateEntities;
	HUD_DRAWNORMALTRIS_FUNCTION						HUD_DrawNormalTriangles;
	HUD_DRAWTRANSTRIS_FUNCTION						HUD_DrawTransparentTriangles;
	HUD_STUDIOEVENT_FUNCTION						HUD_StudioEvent;
	HUD_POSTRUNCMD_FUNCTION							HUD_PostRunCmd;
	HUD_SHUTDOWN_FUNCTION							HUD_Shutdown;
	HUD_TXFERLOCALOVERRIDES_FUNCTION				HUD_TxferLocalOverrides;
	HUD_PROCESSPLAYERSTATE_FUNCTION					HUD_ProcessPlayerState;
	HUD_TXFERPREDICTIONDATA_FUNCTION				HUD_TxferPredictionData;
	HUD_DEMOREAD_FUNCTION							Demo_ReadBuffer;
	HUD_CONNECTIONLESS_FUNCTION						HUD_ConnectionlessPacket;
	HUD_GETHULLBOUNDS_FUNCTION						HUD_GetHullBounds;
	HUD_FRAME_FUNCTION								HUD_Frame;
	HUD_KEY_EVENT_FUNCTION							HUD_Key_Event;
	HUD_TEMPENTUPDATE_FUNCTION						HUD_TempEntUpdate;
	HUD_GETUSERENTITY_FUNCTION						HUD_GetUserEntity;
	HUD_VOICESTATUS_FUNCTION						HUD_VoiceStatus;
	HUD_DIRECTORMESSAGE_FUNCTION					HUD_DirectorMessage;
	HUD_STUDIO_INTERFACE_FUNCTION					HUD_GetStudioModelInterface;
	HUD_CHATINPUTPOSITION_FUNCTION					HUD_ChatInputPosition;
	HUD_GETPLAYERTEAM_FUNCTION						HUD_GetPlayerTeam;
	CLIENTFACTORY									ClientFactory;
} CLIENT, *PCLIENT;
//==================================================================================
extern CLIENT gClient;
extern bool bClientActive;
extern bool ActivateEngine( void );
extern bool ActivateClient( void );
extern bool bEngineActive;
//==================================================================================
#endif
//==================================================================================