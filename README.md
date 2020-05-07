UWB RTLS

- Generate .c and .h proto files from .proto file using Nanopb:

  ```bash
pip install protobuf grpcio-tools
  python proto/nanopb_generator.py -v proto/Settings.proto
  ```
  
- Generate .py proto files from proto file using Protoc

  - Mac OS

    ```bash
    brew install protobuf
    protoc -I=proto --python_out=proto Settings.proto
    ```

