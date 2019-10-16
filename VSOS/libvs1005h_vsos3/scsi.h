/// \file scsi.h Common SCSI definitions

#ifndef SCSI_H
#define SCSI_H

typedef enum {
  SCSI_UNINITIALIZED = -1,
  SCSI_READY_FOR_COMMAND = 0,
  SCSI_DATA_TO_HOST = 1,
  SCSI_TRANSMITTING = 2,
  SCSI_DATA_FROM_HOST = 3,
  SCSI_SEND_STATUS = 4,
  SCSI_INVALID_CBW = 5
} SCSIStageEnum;

typedef enum {
  SCSI_OK = 0,
  SCSI_REQUEST_ERROR = 1,
  SCSI_PHASE_ERROR = 2
} SCSIStatusEnum;

typedef struct scsicdb6variant1 {
  u_int16 length__opcode;
  u_int16 flags__pageCode;
  u_int16 allocationLength; //OK BIG-ENDIAN! :)
  u_int16 control__null;
} ScsiInquiryCdb;

typedef struct scsicdb6variant2 {
  u_int16 length__opcode;
  u_int16 flags__pageCode;
  u_int16 res__allocationLength;
  u_int16 control__null;
} ScsiModeSense6Cdb;

typedef struct scsicdb6variant3 {
  u_int16 length__opcode;
  u_int16 flags__res;
  u_int16 res__allocationLength;
  u_int16 control__null;
} ScsiRequestSenseCdb;

typedef struct scsicdb10variant1 {
  u_int16 length__opcode;
  u_int16 res__lbab3;
  u_int16 lbab2__lbab1;
  u_int16 lbab0__res;
  u_int16 wLength; //OK BIG-ENDIAN :)
  u_int16 control__null;
} ScsiRead10Cdb;
      
typedef struct scsicdb10variant2 {
  u_int16 length__opcode;
  u_int16 flags__lbab3;
  u_int16 lbab2__lbab1;
  u_int16 lbab0__res;
  u_int16 wLength; //OK BIG-ENDIAN :)
  u_int16 control__null;
} ScsiWrite10Cdb;
      

/// Handle any pending SCSI operation
extern void ScsiTaskHandler();

/// Process a SCSI command block
void DiskProtocolCommand(u_int16 *cmd);

/// Reset SCSI state
void ScsiReset();

/// Process data from host
void DiskDataReceived(int length, u_int16 *datablock);

#define OPERATION_CODE 0


//SCSI COMMANDS
#define SCSI_INQUIRY 0x12
#define SCSI_FORMAT_UNIT 0x04
#define SCSI_READ_6 0x08
#define SCSI_READ_10 0x28
#define SCSI_READ_12 0xa8
#define ATAPI_READ_FORMAT_CAPACITIES 0x23
#define SCSI_READ_CAPACITY_10 0x25
#define SCSI_READ_CAPACITY_16 0x9e
#define SCSI_READ_CAPACITY_16_2 0x10
#define SCSI_RECEIVE_DIAGNOSTIC_RESULTS 0x1c
#define SCSI_REPORT_LUNS 0xA0
#define SCSI_REQUEST_SENSE 0x03
#define SCSI_SEND_DIAGNOSTIC 0x1d
#define SCSI_TEST_UNIT_READY 0x00
#define SCSI_WRITE_6 0x0a
#define SCSI_WRITE_10 0x2a
#define SCSI_WRITE_12 0xaa
#define SCSI_MODE_SENSE_6 0x1a
#define SCSI_MODE_SENSE_10 0x5a
#define SCSI_SYNCHRONIZE_CACHE 0x35
#define SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL 0x1e
#define SCSI_VERIFY 0x2f
#define SCSI_START_STOP_UNIT 0x1b
#define SCSI_MODE_SELECT 0x15

//SCSI SENSE KEYS
#define SK_NO_SENSE 0
#define	SK_RECOVERED_ERROR 1
#define SK_NOT_READY 2
#define SK_MEDIUM_ERROR 3
#define	SK_HARDWARE_ERROR 4
#define	SK_ILLEGAL_REQUEST 5
#define	SK_UNIT_ATTENTION 6
#define	SK_DATA_PROTECT 7
#define	SK_BLANK_CHECK 8
#define	SK_VENDOR_SPECIFIC 9
#define	SK_COPY_ABORTED 10
#define SK_ABORTED_COMMAND 11
#define SK_EQUAL 12
#define SK_VOLUME_OVERFLOW 13
#define SK_MISCOMPARE 14

enum SCSIStageEnum ScsiState(void);

/* support */
u_int16 ScsiOrBlock(register __i0 u_int16 *buffer, register __a0 s_int16 size);


#endif


