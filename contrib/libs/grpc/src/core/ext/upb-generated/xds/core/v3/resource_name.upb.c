/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     xds/core/v3/resource_name.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#include <stddef.h>
#include "upb/msg.h"
#include "xds/core/v3/resource_name.upb.h"
#include "udpa/annotations/status.upb.h"
#include "xds/core/v3/context_params.upb.h"
#include "validate/validate.upb.h"

#include "upb/port_def.inc"

static const upb_msglayout *const xds_core_v3_ResourceName_submsgs[1] = {
  &xds_core_v3_ContextParams_msginit,
};

static const upb_msglayout_field xds_core_v3_ResourceName__fields[4] = {
  {1, UPB_SIZE(4, 8), 0, 0, 9, 1},
  {2, UPB_SIZE(12, 24), 0, 0, 9, 1},
  {3, UPB_SIZE(20, 40), 0, 0, 9, 1},
  {4, UPB_SIZE(28, 56), 1, 0, 11, 1},
};

const upb_msglayout xds_core_v3_ResourceName_msginit = {
  &xds_core_v3_ResourceName_submsgs[0],
  &xds_core_v3_ResourceName__fields[0],
  UPB_SIZE(32, 64), 4, false, 255,
};

#include "upb/port_undef.inc"

