#include "monitoring_pb.h"

MonitoringPB pb_monitoring;

MonitoringPB::MonitoringPB()
{
}

S08 MonitoringPB::init()
{
  return RC_ERR_NONE;
}

void MonitoringPB::clearMessage()
{
  message = Monitoring_init_zero;
}
