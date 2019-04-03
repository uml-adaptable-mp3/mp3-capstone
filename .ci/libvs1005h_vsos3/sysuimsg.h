#ifndef SYSUIMSG_H
#define SYSUIMSG_H

#include <cyclic.h>

struct SysUiMsg {
  s_int16 balance;
  s_int16 masterVolume;
  u_int16 volumeChanged;
  struct CyclicNode systemUiMessageCyclicNode;
};
extern const struct SysUiMsg __mem_y romSysUiMsg;
extern struct SysUiMsg sysUiMsg;
#define SYS_UI_MSG_FILTER_VALUES 16
extern __mem_y s_int16 sysUiMsgFilterValues[SYS_UI_MSG_FILTER_VALUES];

void SysUiMsgInit(void);

#endif
