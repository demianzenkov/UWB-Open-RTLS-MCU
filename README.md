## UWB RTLS

- Generate .c and .h proto files from .proto file using Nanopb:

  `pip install protobuf grpcio-tools`

  `python proto/nanopb_generator/nanopb_generator.py proto/proto_files/Settings.proto`
  
  `python nanopb_generator/nanopb_generator.py ./proto_files/Monitoring.proto`

