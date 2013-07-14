////////////////////////////////////////////////////////////////////////////////
//
// subscribe.cpp
//
// Copyright (c) 2013 Eric Lombrozo, all rights reserved
//

#include <ripplesocket.h>

#include <iostream>

using namespace std;
using namespace json_spirit;

int main(int argc, char** argv)
{
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <server url>" << endl;
        return 1;
    }

    RippleSocket socket;
    socket.on("transaction", [] (const Object& obj) {
        cout << "transaction: " << write_string<Value>(obj, false) << endl << endl;
    }).on("ledgerClosed", [] (const Object& obj) {
        cout << "ledgerClosed: " << write_string<Value>(obj, false) << endl << endl;
    }).on("serverStatus", [] (const Object& obj) {
        cout << "serverStatus: " << write_string<Value>(obj, false) << endl << endl;
    });
    socket.start(argv[1], [&] () {
        vector<string> streams;
        streams.push_back("transactions");
        streams.push_back("ledger");
        streams.push_back("server");
        const Array streamsVal(streams.begin(), streams.end());
        Object cmd;
        cmd.push_back(Pair("command", "subscribe"));
        cmd.push_back(Pair("streams", streamsVal));
        socket.sendCommand(cmd, [] (const Object& obj) {
            cout << "Response: " << write_string<Value>(obj, true) << endl;
        });
    }, [] (const string& msg) { cout << "Log: " << msg << endl << endl; });
    return 0;
}
