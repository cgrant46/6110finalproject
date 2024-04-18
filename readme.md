cd to the ns-3.40/src folder

Run the following command: 
```bash
git clone https://github.com/signetlabdei/quic quic
```bash

Download the entire repository from github (https://github.com/signetlabdei/quic-ns-3/tree/master) and extract all files beginning with 'quic' from the src/applications/helper into your ns3 project (same path, there should be four of them)

Do the same thing in the src/applications/model directory (same path, 8 files this time)


Go into the src/applications/CMakeLists.txt file and paste the following under SOURCE_FILES:

```python
    helper/quic-client-server-helper.cc
    helper/quic-echo-helper.cc
    model/quic-client.cc
    model/quic-echo-client.cc
    model/quic-echo-server.cc
    model/quic-server.cc
```python

Paste the following under HEADER_FILES

```python
    helper/quic-client-server-helper.h
    helper/quic-echo-helper.h
    model/quic-client.h
    model/quic-echo-client.h
    model/quic-echo-server.h
    model/quic-server.h
```python