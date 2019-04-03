#ifndef VS_NAND_H
#define VS_NAND_H

/** Nand Flash Opcode: */
#define NAND_OP_ 0x

/** Nand Flash Opcode: Read Signature*/
#define NAND_OP_READ_SIGNATURE 0x90

/** Nand Flash Opcode: Read Signature*/
#define NAND_OP_READ_STATUS 0x70

/** Nand Flash Opcode: Read A*/
#define NAND_OP_READ_A 0x00

/** Nand Flash Opcode: Read C*/
#define NAND_OP_READ_C 0x50

/** Nand Flash Opcode: Commit read address */
#define NAND_OP_COMMIT_DATA_ADDRESS 0x30

/** Nand Flash Opcode: Prepare to Program*/
#define NAND_OP_PREPARE_TO_PROGRAM 0x80

/** Nand Flash Opcode: Execute Programming*/
#define NAND_OP_PERFORM_PROGRAM 0x10

#ifdef ASM

/*asm-only definitions*/

#else/*ASM*/

#include <vstypes.h>
#include <stdlib.h>
#include <physical.h>

struct FsNandPhys {
  /** Physical basic structure */
  struct FsPhysical p;
  /* Custom fields follow */
  u_int16 nandType;
  u_int16 waitns; /* wait in ns */
  u_int16 fifoAddr; /* VS1005: default address to use in FIFO */
};

/** Pull Nand Flash Chip Select Low */
#define NandSelect() PERIP(GPIO0_CLEAR_MASK) = GPIO0_CS1

/** Pull Nand Flash Chip Select High */
#define NandDeselect() PERIP(GPIO0_SET_MASK) = GPIO0_CS1

#define NAND_IS_LARGE_PAGE  (fsNandPhys.nandType & 0x0001)

void NandPutCommand(register __a0 u_int16 command);
void NandPutAddressOctet(register __a0 u_int16 address);
void XpFifoRead(u_int16 wcnt, u_int16 addr, u_int16 *dbuf);
void XpFifoWrite(u_int16 wcnt, u_int16 addr, u_int16 * dbuf);
void NandGetOctets(register __c0 s_int16 length, register __i2 u_int16 *buf);
void NandPutOctets(register __c0 s_int16 length, register __i2 u_int16 *buf);
void NandPutBlockAddress(register __c u_int32 addr);
void NandPutDataAddress(register __c u_int32 addr);
void NandPutLargePageSpareAddress(register __c u_int32 addr);
u_int16 NandGetStatus(void);
s_int16 NandPutPage(u_int32 addr, u_int16 *buf, u_int16 *spare);
void NandSetWaits(register __a0 u_int16 waitns);


/** Erase the block that starts from page "block". */
s_int16 FsPhNandErase(struct FsPhysical *p, s_int32 block);

/** Creates a physical layer. */
struct FsPhysical *FsPhNandCreate(u_int16 param);

/** Free resources allocated by FsPhNandCreate and release HW */
s_int16 FsPhNandDelete(struct FsPhysical *p);

/** Free hardware bus for possible other devices */
s_int16 FsPhNandFreeBus(struct FsPhysical *p);

/** Reinitialize bus */
s_int16 FsPhNandReinitialize(struct FsPhysical *p);

/** Read pages. */
s_int16 FsPhNandRead(struct FsPhysical *p, s_int32 firstPage,
		     u_int16 pages, u_int16 *data, u_int16 *meta);

/** Write pages */
s_int16 FsPhNandWrite(struct FsPhysical *p, s_int32 firstPage,
		      u_int16 pages, u_int16 *data, u_int16 *meta);

s_int16 FsPhNullFail();   /*< intentionally incomplete prototype */
s_int16 FsPhNullOk();   /*< intentionally incomplete prototype */

/** Support: Count bits */
s_int16 NandCountBits(register __a u_int32 val);
s_int16 NandMingle(register __a u_int32 val);
void NandWaitIdle(void);
s_int16 LargePageWrite(s_int32 page, u_int16 *data, u_int16 *meta);
s_int16 LargePageRead(s_int32 page, u_int16 *data, u_int16 *meta);

#endif/*elseASM*/
#endif
