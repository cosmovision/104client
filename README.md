# 104client

###Installing

You will need the libiec60870 to use this tool:  
http://libiec61850.com/libiec61850/downloads/

You can build 104client with the following command
```bash
g++ -Wall -I/usr/local/include -pthread 104client.c -o104client -l60870
```

###Usage

Options:
```
104client -i (+IP-Address)

Optional parameters:

    -c   -   CASDU adress           (+integer value)
    -a   -   IOA                    (+integer value)
    -T   -   Connection duration    (+integer value)
    -t   -   Command  with type     (+integer value)
             Supported types: 45 46
    -d   -   Command data           (+integer value)
    -g   -   General Interrogation
    -v   -   Verbose
    -h   -   Show all Information Object types
```
