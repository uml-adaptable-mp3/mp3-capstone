#ifndef MMCCOMMANDS_H
#define MMCCOMMANDS_H

#define         MMC_GO_IDLE_STATE   0
#define         MMC_SEND_OP_COND   1
#define         MMC_SEND_IF_COND   8
#define         MMC_SEND_CSD   9
#define         MMC_SEND_CID   10
#define         MMC_STOP_TRANSMISSION 12
#define         MMC_SEND_STATUS   13
#define         MMC_SET_BLOCKLEN   16
#define         MMC_READ_SINGLE_BLOCK   17
#define         MMC_READ_MULTIPLE_BLOCK 18
#define         MMC_WRITE_BLOCK   24
#define         MMC_PROGRAM_CSD   27
#define         MMC_SET_WRITE_PROT   28
#define         MMC_CLR_WRITE_PROT   29
#define         MMC_SEND_WRITE_PROT   30
#define         MMC_TAG_SECTOR_START   32
#define         MMC_TAG_SECTOR_END   33
#define         MMC_UNTAG_SECTOR   34
#define         MMC_TAG_ERASE_GROUP_START   35
#define         MMC_TAG_ERARE_GROUP_END   36
#define         MMC_UNTAG_ERASE_GROUP   37
#define         MMC_ERASE   38
#define         MMC_READ_OCR     58
#define         MMC_CRC_ON_OFF   59
#define         MMC_R1_BUSY   0x80
#define         MMC_R1_PARAMETER   0x40
#define         MMC_R1_ADDRESS   0x20
#define         MMC_R1_ERASE_SEQ   0x10
#define         MMC_R1_COM_CRC   0x08
#define         MMC_R1_ILLEGAL_COM   0x04
#define         MMC_R1_ERASE_RESET   0x02
#define         MMC_R1_IDLE_STATE   0x01
#define         MMC_STARTBLOCK_READ   0xFE
#define         MMC_STARTBLOCK_WRITE   0xFE
#define         MMC_STARTBLOCK_MWRITE   0xFC
#define         MMC_STOPTRAN_WRITE   0xFD
#define         MMC_DE_MASK   0x1F
#define         MMC_DE_ERROR   0x01
#define         MMC_DE_CC_ERROR   0x02
#define         MMC_DE_ECC_FAIL   0x04
#define         MMC_DE_OUT_OF_RANGE   0x04
#define         MMC_DE_CARD_LOCKED   0x04
#define         MMC_DR_MASK   0x1F
#define         MMC_DR_ACCEPT   0x05
#define         MMC_DR_REJECT_CRC   0x0B
#define         MMC_DR_REJECT_WRITE_ERROR   0x0D

#endif