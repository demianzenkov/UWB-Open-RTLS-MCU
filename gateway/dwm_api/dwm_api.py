from enum import Enum
import numpy as np
import dwm_constants as dwm

class DWM1001:
    def __init__(self):
        pass

    # Initializes the required components for DWM1001 module,
    def dwm_init(self):
        pass

    # Position coordinates in millimeters + quality factor
    dwm_pos = {'x': np.int32,
               'y': np.int32,
               'z': np.int32,
               'qf': np.uint8}

    # Sets position of anchor node
    def dwm_pos_set(self, pos):
        pass

    # Gets position of the node
    def dwm_pos_get(self):
        pass

    # maximum and minimum update rate in multiple of 100 ms
    dwm_upd_rate = {
        'DWM_UPD_RATE_MAX': 600,     # 1 minute
        'DWM_UPD_RATE_MIN': 1        # 100 ms
    }

    # Sets update rate
    def dwm_upd_rate_set(self, ur):
        pass

    # Gets update rate
    def dwm_upd_rate_get(self, ur):
        pass

    #  Position measurement modes
    dwm_meas_mode = {
        'DWM_MEAS_MODE_TWR': 0,
        'DWM_MEAS_MODE_TDOA': 1
    }

    # Device modes
    dwm_mode = {
        'DWM_MODE_TAG': 0,
        'DWM_MODE_ANCHOR': 1
    }

    dwm_uwb_mode = {
        'DWM_UWB_MODE_OFF': 0,
        'DWM_UWB_MODE_PASSIVE': 1,
        'DWM_UWB_MODE_ACTIVE': 2
    }

    dwm_cfg_common = {
        'fw_update_en': bool,
        'dwm_uwb_mode': dwm_uwb_mode.get('DWM_UWB_MODE_ACTIVE'),
        'security_en': bool,
        'ble_en': bool,
        'led_en': bool,
    }

    dwm_cfg_anchor = {
        'common': dwm_cfg_common,
        'bridge': bool,
        'initiator': bool
    }

    dwm_cfg_tag = {
        'common': dwm_cfg_common,
        'loc_engine': bool,
        'low_power_en': bool,
        'accel_en': bool,
        'meas_mode': dwm_meas_mode.get('DWM_MEAS_MODE_TWR')
    }

    dwm_cfg = {
        'common': dwm_cfg_common,
        'loc_engine': bool,
        'low_power_en': bool,
        'accel_en': bool,
        'meas_mode': dwm_meas_mode.get('DWM_MEAS_MODE_TWR'),
        'bridge': bool,
        'initiator': bool,
        'mode': dwm_mode.get('DWM_MODE_TAG')
    }

    # Configures node to tag mode with given options
    def dwm_cfg_tag_set(self, cfg):
        pass

    # Configures node to anchor mode with given options
    def dwm_cfg_anchor_set(self, cfg):
        pass

    # Reads configuration of the node
    def dwm_cfg_get(self):
        pass

    #  Puts device to sleep mode
    def dwm_sleep(self):
        pass

    # Distances of ranging anchors
    dwm_distance = {
        'cnt': int,
        'addr': np.zeros(dwm.DWM_LOC_CNT_MAX, dtype=np.uint64),
        'dist': np.zeros(dwm.DWM_LOC_CNT_MAX, dtype=np.uint32),
        'qf': np.zeros(dwm.DWM_LOC_CNT_MAX, dtype=np.uint8),
    }

    # Position of ranging anchors
    dwm_anchor_pos = {
        'cnt': np.uint8,
        'pos': [dict() for i in range(dwm.DWM_LOC_CNT_MAX)]     # dwm_pos
    }

    # Distances and position of ranging anchors
    dwm_ranging_anchors = {
        'dist': dwm_distance,
        'an_pos': dwm_anchor_pos
    }

    # Location data (position of current node and list of positions and distances of ranging anchors)
    dwm_loc_data = {
        'pos': dwm_pos,
        'anchors': dwm_ranging_anchors
    }

    # Gets location data
    def dwm_loc_get(self, loc):
        pass

    # BLE address
    dwm_baddr = {
        'byte':  np.zeros(dwm.DWM_BLE_ADDR_LEN, dtype=np.uint8)
    }

    # Sets the public BLE address used by device. New address takes effect after reset.
    def dwm_baddr_set(self, baddr):
        pass

    # Gets the current BLE address used by device.
    def dwm_baddr_get(self):
        pass

    # Resets DWM module
    def dwm_reset(self):
        pass

    # Firmware version data
    dwm_fw_ver = {
        'maj': np.uint8,
        'min': np.uint8,
        'patch': np.uint8,
        'res': np.uint8,
        'var': np.uint8
    }

    # Version data
    dwm_ver_t = {
        'fw': dwm_fw_ver,
        'cfg': np.uint32,
        'hw': np.uint32
    }

    # Gets versions
    def dwm_ver_get(self):
        pass

    # DWM status
    dwm_status = {
        'loc_data': bool,
        'uwbmac_joined': bool
    }

    # Get system status: Location Data ready
    def dwm_status_get(self):
        pass

    def dwm_int_cfg(self, value):
        pass



