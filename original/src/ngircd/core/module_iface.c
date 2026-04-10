#include "portab.h"

#include "module_iface.h"

static const NgPlatformOps *PlatformOps;
static const NgLogOps *LogOps;
static const NgIoOps *IoOps;
static const NgResolverOps *ResolverOps;
static const NgSignalOps *SignalOps;

GLOBAL void
NgPlatform_SetOps(const NgPlatformOps *Ops)
{
	PlatformOps = Ops;
}

GLOBAL const NgPlatformOps *
NgPlatform_GetOps(void)
{
	return PlatformOps;
}

GLOBAL void
NgLog_SetOps(const NgLogOps *Ops)
{
	LogOps = Ops;
}

GLOBAL const NgLogOps *
NgLog_GetOps(void)
{
	return LogOps;
}

GLOBAL void
NgIo_SetOps(const NgIoOps *Ops)
{
	IoOps = Ops;
}

GLOBAL const NgIoOps *
NgIo_GetOps(void)
{
	return IoOps;
}

GLOBAL void
NgResolver_SetOps(const NgResolverOps *Ops)
{
	ResolverOps = Ops;
}

GLOBAL const NgResolverOps *
NgResolver_GetOps(void)
{
	return ResolverOps;
}

GLOBAL void
NgSignal_SetOps(const NgSignalOps *Ops)
{
	SignalOps = Ops;
}

GLOBAL const NgSignalOps *
NgSignal_GetOps(void)
{
	return SignalOps;
}
