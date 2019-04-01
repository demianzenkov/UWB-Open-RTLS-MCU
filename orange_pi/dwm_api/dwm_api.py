from enum import Enum

class DWM1001:
    # Initializes the required components for DWM1001 module,
    def dwm_init(self):
        pass

    # Position coordinates in millimeters + quality factor
    dwm_pos_t = {'x': 0,
                 'y': 0,
                 'z': 0,
                 'qf': 0}

    # Sets position of anchor node
    def dwm_pos_set(self, pos):
        pass

    # Gets position of the node
    def dwm_pos_get(self):
        pass

    # maximum and minimum update rate in multiple of 100 ms
    class dwm_upd_rate(Enum):
        DWM_UPD_RATE_MAX = 600  # 1 minute
        DWM_UPD_RATE_MIN = 1    # 100 ms


    # Sets update rate
    def dwm_upd_rate_set(self, ur):
        pass

    # Gets update rate
    def dwm_upd_rate_get(self, ur):
        pass

    #  Position measurement modes
    class dwm_meas_mode(Enum):
        DWM_MEAS_MODE_TWR = 0
        DWM_MEAS_MODE_TDOA = 1

    # Device modes
    class dwm_mode(Enum):
        DWM_MODE_TAG = 0,
        DWM_MODE_ANCHOR = 1

    class dwm_uwb_mode(Enum):
        DWM_UWB_MODE_OFF = 0,
        DWM_UWB_MODE_PASSIVE = 1,
        DWM_UWB_MODE_ACTIVE = 2

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

    # Gets location data
    def dwm_loc_get(self, loc):
        pass

    # Sets the public BLE address used by device. New address takes effect after reset.
    def dwm_baddr_set(self, baddr):
        pass

    # Gets the current BLE address used by device.
    def dwm_baddr_get(self):
        pass

    # Resets DWM module
    def dwm_reset(self):
        pass

    # Gets versions
    def dwm_ver_get(self):
        pass

    # Get system status: Location Data ready
    def dwm_status_get(self):
        pass

    def dwm_int_cfg(self, value):
        pass



