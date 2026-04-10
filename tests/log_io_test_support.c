#include "portab.h"

#include "ngircd.h"
#include "conn.h"
#include "conf.h"
#include "client.h"
#include "module_iface.h"
#include "proc.h"

time_t NGIRCd_Start = 0;
char NGIRCd_StartStr[64] = "test";
char NGIRCd_Version[126] = "test-version";
char NGIRCd_VersionAddition[126] = "test";
bool NGIRCd_Debug = false;
bool NGIRCd_Passive = false;
bool NGIRCd_SignalQuit = false;
bool NGIRCd_SignalRestart = false;
char NGIRCd_DebugLevel[2] = "0";
char NGIRCd_ConfFile[FNAME_LEN];
char NGIRCd_ProtoID[COMMAND_LEN] = "test";

char Conf_ServerName[CLIENT_ID_LEN];
char Conf_ServerInfo[CLIENT_INFO_LEN];
char Conf_ServerPwd[CLIENT_PASS_LEN];
char Conf_ServerAdmin1[CLIENT_INFO_LEN];
char Conf_ServerAdmin2[CLIENT_INFO_LEN];
char Conf_ServerAdminMail[CLIENT_INFO_LEN];
char Conf_Network[CLIENT_INFO_LEN];
array Conf_Motd;
array Conf_Helptext;
array Conf_ListenPorts;
char *Conf_ListenAddress;
uid_t Conf_UID;
gid_t Conf_GID;
char Conf_Chroot[FNAME_LEN];
char Conf_PidFile[FNAME_LEN];
int Conf_PingTimeout;
int Conf_PongTimeout;
int Conf_ConnectRetry;
array Conf_Opers;
CONF_SERVER Conf_Server[MAX_SERVERS];
array Conf_Channels;
char Conf_AllowedChannelTypes[8];
bool Conf_OperCanMode;
bool Conf_OperChanPAutoOp;
bool Conf_OperServerMode;
bool Conf_AllowRemoteOper;
char Conf_CloakHost[CLIENT_ID_LEN];
char Conf_CloakHostModeX[CLIENT_ID_LEN];
char Conf_CloakHostSalt[CLIENT_ID_LEN];
bool Conf_CloakUserToNick;
bool Conf_DNS;
bool Conf_Ident;
bool Conf_MorePrivacy;
bool Conf_NoticeBeforeRegistration;
bool Conf_PAM;
bool Conf_PAMIsOptional;
char Conf_PAMServiceName[MAX_PAM_SERVICE_NAME_LEN];
bool Conf_ScrubCTCP;
char Conf_DefaultChannelModes[CHANNEL_MODE_LEN];
char Conf_DefaultUserModes[CLIENT_MODE_LEN];
bool Conf_ConnectIPv6;
bool Conf_ConnectIPv4;
int Conf_IdleTimeout;
int Conf_MaxConnections;
int Conf_MaxJoins;
int Conf_MaxConnectionsIP;
unsigned int Conf_MaxNickLength;
int Conf_MaxListSize;
time_t Conf_MaxPenaltyTime;
bool Conf_AuthPing;
int Conf_SyslogFacility;
char Conf_WebircPwd[CLIENT_PASS_LEN];

CLIENT *Client_First(void) { return NULL; }
CLIENT *Client_Next(CLIENT *c) { (void)c; return NULL; }
CONN_ID Client_Conn(CLIENT *Client) { (void)Client; return NONE; }
char *Client_ID(CLIENT *Client) { (void)Client; return (char *)"test"; }
bool Client_HasMode(CLIENT *Client, char Mode) { (void)Client; (void)Mode; return false; }
CLIENT *Client_ThisServer(void) { return NULL; }
void Client_SetInfo(CLIENT *Client, const char *Info) { (void)Client; (void)Info; }
void Conn_Close(CONN_ID Idx, const char *Reason, const char *Msg, bool InformServer)
{ (void)Idx; (void)Reason; (void)Msg; (void)InformServer; }
bool Channel_IsValidName(const char *name) { (void)name; return true; }
bool MatchCaseInsensitiveList(const char *Pattern, const char *List, const char *Delim)
{ (void)Pattern; (void)List; (void)Delim; return false; }
void Channel_LogServer(const char *msg) { (void)msg; }
bool IRC_WriteStrClient(CLIENT *Client, const char *Format, ...) { (void)Client; (void)Format; return true; }
bool IRC_WriteStrClientPrefix(CLIENT *Client, CLIENT *Prefix, const char *Format, ...) { (void)Client; (void)Prefix; (void)Format; return true; }
long Conn_CountAccepted(void) { return 0; }

void Proc_InitStruct(PROC_STAT *proc) { if (proc) { proc->pid = 0; proc->pipe_fd = -1; } }
pid_t Proc_Fork(PROC_STAT *proc, int *pipefds, void (*cbfunc)(int, short), int timeout)
{ (void)proc; (void)pipefds; (void)cbfunc; (void)timeout; return -1; }
void Proc_GenericSignalHandler(int Signal) { (void)Signal; }
size_t Proc_Read(PROC_STAT *proc, void *buffer, size_t buflen)
{ (void)proc; (void)buffer; (void)buflen; return 0; }
void Proc_Close(PROC_STAT *proc) { if (proc) Proc_InitStruct(proc); }

bool Signal_NotifySvcMgr_Possible(void) { return false; }
void Signal_NotifySvcMgr(const char *message) { (void)message; }

