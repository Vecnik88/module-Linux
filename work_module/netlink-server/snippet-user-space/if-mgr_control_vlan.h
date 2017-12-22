#ifndef IF_MNG_CONTROL_VLAN_H
#define IF_MNG_CONTROL_VLAN_H

#include <nl-server/nl-server.h>

#include "if-mgr.h"

int if_mgr_create_connect_to_kernel_space();
int if_mgr_vlan_carrier_change(ifmgr_t * ifmgr, struct vlan_carrier vlan_opt);

#endif	/* IF_MNG_CONTROL_VLAN_H */
