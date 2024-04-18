cd to the ns-3.40/src folder

Run the following command: 
```bash
git clone https://github.com/signetlabdei/quic quic
```

Download the entire repository from github (https://github.com/signetlabdei/quic-ns-3/tree/master) and extract all files beginning with 'quic' from the src/applications/helper into your ns3 project (ns-3.40/src/applications/helper) - there should be four files - path below

```bash
ns-3.40/src/applications/helper/quic-client-server-helper.cc
ns-3.40/src/applications/helper/quic-client-server-helper.h
ns-3.40/src/applications/helper/quic-echo-helper.cc
ns-3.40/src/applications/helper/quic-echo-helper.h
```

Do the same thing in the src/applications/model directory (same idea as above, 8 files this time)
```bash
ns-3.40/src/applications/model/quic-client.cc
ns-3.40/src/applications/model/quic-client.h
ns-3.40/src/applications/model/quic-echo-client.cc
ns-3.40/src/applications/model/quic-echo-client.h
ns-3.40/src/applications/model/quic-echo-server.cc
ns-3.40/src/applications/model/quic-echo-server.h
ns-3.40/src/applications/model/quic-server.cc
ns-3.40/src/applications/model/quic-server.h
```

Go into the src/applications/CMakeLists.txt file and paste the following under SOURCE_FILES:

```python
    helper/quic-client-server-helper.cc
    helper/quic-echo-helper.cc
    model/quic-client.cc
    model/quic-echo-client.cc
    model/quic-echo-server.cc
    model/quic-server.cc
```

Paste the following under HEADER_FILES

```python
    helper/quic-client-server-helper.h
    helper/quic-echo-helper.h
    model/quic-client.h
    model/quic-echo-client.h
    model/quic-echo-server.h
    model/quic-server.h
```
