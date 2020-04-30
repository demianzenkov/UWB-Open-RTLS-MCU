#ifndef __MX25
#define __MX25

#include "cmsis_os.h"
#include "prj_defs.h"
#include "main.h"
#include "bsp_spi.h"

#define MX25_CMD_WR_SREG                0x01
#define MX25_CMD_PAGE_PROG              0x02
#define MX25_CMD_RD_SREG                0x05
#define MX25Q_CMD_WR_ENABLE             0x06
#define MX25Q_CMD_PAGE_FAST_READ        0x0B
#define MX25Q_CMD_SUBSECTOR_ERASE       0x20
#define MX25_CMD_RDID                  	0x9F
#define MX25Q_CMD_SECTOR_ERASE          0xD8

#define MX25_SREG_BUSY                  (1 << 0)
#define MX25_SREG_LATCH_EN              (1 << 1)

#define JEDEC_MACRONIX                  (0xC2)
#define MX25_MEMORY_TYPE                (0x20)
#define MX25_16MB_DENSITY             	(0x15)

#define MX25_PAGE_SIZE                  (0x100UL)
#define MX25_SUBSECTOR_SIZE             (0x1000UL)
#define MX25_SUBSECTOR_MASK             (0xFFFUL)
#define MX25_SECTOR_SIZE                (0x10000UL)

#define MX25_16MB_SECTOR_START          (0)
#define MX25_16MB_SECTOR_END            (255)

#define MX25_UNIT_SIZE              	(MX25_SECTOR_SIZE)
#define MX25_16MB_CONFIG_SECTOR_FIRST 	(MX25_16MB_SECTOR_END-2)
#define MX25_16MB_CONFIG_SECTOR_LAST	(MX25_16MB_SECTOR_END)


typedef enum
{
  FL_TYPE_UNDEF,
  FL_TYPE_MX25_16MB
} flash_hw_te;	// flash memory type

typedef enum
{
  PARTNUM_EMPTY = -1,
  PARTNUM_CONFIG = 0,
} mem_part_num_te;

typedef struct
{
  U08 man_id;
  U08 type;
  U08 density;
} flash_id_ts;

typedef struct
{
  U16 unit_first;	// first page/sector address
  U16 unit_last;	// last page/sector address
  U32 mem_size;		// size of memory for the partition
  flash_hw_te hw_type;	// type of memory
  mem_part_num_te pn;	// section number
  U32 unit_size;	// sector size
  U32 page_size;	// page size
  U32 subsector_size;	// subsector size
} mem_part;


class MX25 {
public:
  MX25();
  
  S08 init (BSP_SPI * spi);
  
  S08 deinit (void);
  S08 get_dev_inf (flash_id_ts* p_inf);
  S08 read (U32 addr, U08* p_buf, U16 len);
  S08 wr_page (U32 addr, U08* p_buf, U16 len);
  S08 subsect_erase (U32 addr);
  S08 sector_erase (U32 addr);
  S08 detect (void);
  const mem_part* get_partition(mem_part_num_te part);
  
  inline S08 lock (void);
  inline S08 unlock (void);
  inline S08 wr_enable (void);
  inline S08 wait_prog (void);
  inline S08 wait_erase (void);
  inline U08 rd_sreg (void);
  
private:
  U08 temp_buf[MX25_PAGE_SIZE];
  OS_SEM sem;
  BSP_SPI * bsp_spi;
  
};

#endif