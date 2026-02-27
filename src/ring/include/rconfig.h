/* Custom Configuration File (Could be modified when embedding Ring in other projects) */

/*
 * MRE (MediaTek Runtime Environment) overrides
 * These are defined here instead of ring.h so updating Ring doesn't lose them.
 * rconfig.h is included before all other checks in ring.h, so these take effect
 * via the existing #ifndef / #if guards.
 */
#ifdef RING_VM_MRE
	#define RING_LIMITEDENV 1
	#define RING_LIMITEDSYS 1
	#define RING_VM_OS 0
	#define RING_VM_REFMETA 0
	#define RING_VM_INFO 0
#endif