# def vals():

# TLV API
DWM1001_TLV_MAX_SIZE = 255
DWM1001_TLV_RET_VAL_MIN_SIZE = 3

# Definitions for TLV Type byte
DWM1001_TLV_TYPE_UNKNOWN = 0	        # unknown TLV type
DWM1001_TLV_TYPE_CMD_POS_SET = 1        # request set position coordinates XYZ
DWM1001_TLV_TYPE_CMD_POS_GET = 2	    # request get position coordinates XYZ
DWM1001_TLV_TYPE_CMD_UR_SET = 3	        # request set position update rate
DWM1001_TLV_TYPE_CMD_UR_GET = 4	        # request get position update rate
DWM1001_TLV_TYPE_CMD_CFG_TN_SET = 5	    # request set configuration for the tag
DWM1001_TLV_TYPE_CMD_CFG_AN_SET = 7	    # request set configuration for the anchor
DWM1001_TLV_TYPE_CMD_CFG_GET = 8	    # request get configuration data
DWM1001_TLV_TYPE_CMD_CFG_SAVE = 9	    # request save configuration data
DWM1001_TLV_TYPE_CMD_SLEEP = 10	        # request sleep
DWM1001_TLV_TYPE_CMD_LOC_GET = 12	    # request location get
DWM1001_TLV_TYPE_CMD_BLE_ADDR_SET = 15	# request BLE address set
DWM1001_TLV_TYPE_CMD_BLE_ADDR_GET = 16	# request BLE address get
DWM1001_TLV_TYPE_CMD_RESET = 20	        # request reset
DWM1001_TLV_TYPE_CMD_VER_GET = 21	    # request FW version get
DWM1001_TLV_TYPE_CMD_UWB_CFG_ANT_SET = 30	 # request set uwb antenna tx, rx delay
DWM1001_TLV_TYPE_CMD_UWB_CFG_ANT_GET = 31	 # request get uwb antenna tx, rx delay
DWM1001_TLV_TYPE_CMD_GPIO_CFG_OUTPUT = 40	 # request configure output pin and set
DWM1001_TLV_TYPE_CMD_GPIO_CFG_INPUT	= 41	 # request configure input pin
DWM1001_TLV_TYPE_CMD_GPIO_VAL_SET = 42	     # request set pin value
DWM1001_TLV_TYPE_CMD_GPIO_VAL_GET = 43	     # request get pin value
DWM1001_TLV_TYPE_CMD_GPIO_VAL_TOGGLE = 44	 # request toggle pin value
DWM1001_TLV_TYPE_CMD_STATUS_GET	= 50	     # request status get
DWM1001_TLV_TYPE_CMD_INT_CFG = 52	         # request configure interrupts
DWM1001_TLV_TYPE_CMD_N_POS_SET = 128	     # nested request set position
# DWM1001_TLV_TYPE_CMD_N_LOC_GET = 130	     # nested request location get
DWM1001_TLV_TYPE_RET_VAL = 64	             # request return value (as the response)
DWM1001_TLV_TYPE_POS_XYZ = 65	        # position coordinates x,y,z
DWM1001_TLV_TYPE_POS_X	= 66	        # position coordinate x
DWM1001_TLV_TYPE_POS_Y = 67	            # position coordinate y
DWM1001_TLV_TYPE_POS_Z = 68	            # position coordinate z
DWM1001_TLV_TYPE_UR	= 69	            # update rate
DWM1001_TLV_TYPE_CFG = 70	            # configuration data
DWM1001_TLV_TYPE_DIST = 72	            # distances
DWM1001_TLV_TYPE_RNG_AN_POS_DIST = 73	# ranging anchor distances and positions
DWM1001_TLV_TYPE_FW_VER	= 80	        # fw_version
DWM1001_TLV_TYPE_CFG_VER = 81	 # cfg_version
DWM1001_TLV_TYPE_HW_VER	= 82	 # hw_version
DWM1001_TLV_TYPE_PIN_VAL = 85	 # pin value
DWM1001_TLV_TYPE_STATUS	= 90	 # status
DWM1001_TLV_TYPE_IDLE = 255      # Dummy byte, or type: idle


# DWM Error codes, returned from DWM1001 module
DWM_OK = 0
DWM_ERR_INTERNAL = -1
DWM_ERR_BUSY = -2
DWM_ERR_INVAL_ADDR = -3
DWM_ERR_INVAL_PARAM = -4
DWM_ERR_OVERRUN = -5
DWM_ERR_I2C_ANACK = -10
DWM_ERR_I2C_DNACK = -11


#  Return value Error codes, returned from DWM API functions
RV_OK = 0   # ret value OK
RV_ERR = 1  # ret value ERROR


# Maximum count of nodes in location data
DWM_LOC_CNT_MAX = 15


# BLE address length
DWM_BLE_ADDR_LEN = 6


# DWM status bits mask
DWM_STATUS_LOC_DATA_READY_MASK = 0x01
DWM_STATUS_UWBMAC_JOINED_MASK = 0x02


# Bit definitions for TLV Type request: DWM1001_TLV_TYPE_CMD_INT_CFG
DWM1001_INTR_NONE = 0
DWM1001_INTR_LOC_READY = (1 << 0)	    # location data ready
DWM1001_INTR_SPI_DATA_READY = (1 << 1)  # SPI data ready

