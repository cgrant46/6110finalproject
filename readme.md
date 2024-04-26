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

Edits to the QUIC source code

I added a private field to ```applications/model/quic-echo-server.h``` AND ```applications/model/quic-server.h``` that makes it easier to track goodput
Under the private fields, add:
```c++
uint64_t m_totalRx;
```
Under the public methods, add:
```c++
  /**
   * \return the total bytes received in this app
   */
  uint64_t GetTotalRx() const;
```
In ```applications/model/quic-echo-server.cpp``` AND ```applications/model/quic-server.cpp```, edit the constructor and add the following method (BE SURE TO CHANGE CLASS MEMBERSHIP TO QuicServer):
```c++
QuicEchoServer::QuicEchoServer ()
{
  NS_LOG_FUNCTION (this);
  m_totalRx = 0; //set total received to 0
}

uint64_t
QuicEchoServer::GetTotalRx() const 
{
  NS_LOG_FUNCTION(this);
  return m_totalRx;
}
```

Finally, in the ```QuicEchoServer::HandleRead(Ptr<Socket> socket)``` function, add the following line of code INSIDE THE WHILE LOOP:
```c++
m_totalRx += packet->GetSize();
```
Line 458 of ```applications/model/quic-echo-client.cc```, along with line 223 of ```applications/model//quic-client.cc``` was changed to:
```c++
if (m_sent < m_count || m_count == 0)
```

