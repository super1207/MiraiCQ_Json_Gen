g++ dllfunview.cpp -lws2_32 -DNDEBUG -O2 -m32 -static -shared -o dllfunview.dll -Wl,--out-implib,dllfunview.lib -s
