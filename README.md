UWB RTLS

- Generate .c and .h proto files from .proto file using Nanopb:

  ```bash
pip install protobuf grpcio-tools
  python proto/nanopb_generator/nanopb_generator.py proto/proto_files/Settings.proto
  ```
  
- Generate .py proto files from proto file using Protoc

  - Mac OS

    ```bash
    brew install protobuf
    cd proto/proto_files
    protoc --proto_path=. --python_out=. ./Settings.proto
    protoc --proto_path=. --python_out=. ./Monitoring.proto
    ```
