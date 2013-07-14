////////////////////////////////////////////////////////////////////////////////
//
// ripplesocket.cpp
//
// Copyright (c) 2013 Eric Lombrozo, all rights reserved
//

#include "ripplesocket.h"

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using namespace std;
using namespace json_spirit;

/// Public Methods
RippleSocket::RippleSocket()
{
    bConnected = false;
    sequence = 0;

    client.set_access_channels(websocketpp::log::alevel::all);
    client.set_error_channels(websocketpp::log::elevel::all);

    client.init_asio();

    client.set_open_handler(bind(&RippleSocket::onOpen, this, ::_1));
    client.set_close_handler(bind(&RippleSocket::onClose, this, ::_1));
    client.set_fail_handler(bind(&RippleSocket::onFail, this, ::_1));
    client.set_message_handler(bind(&RippleSocket::onMessage, this, ::_1, ::_2));
}

RippleSocket::~RippleSocket()
{
    if (bConnected) {
        pConnection->close(websocketpp::close::status::going_away, "");
    }
}

void RippleSocket::start(const string& serverUrl, socket_handler_t on_open, log_handler_t on_log)
{
    if (bConnected) {
        throw runtime_error("Already connected.");
    }

    error_code_t error_code;
    pConnection = client.get_connection(serverUrl, error_code);
    if (error_code) {
        client.get_alog().write(websocketpp::log::alevel::app, error_code.message());
        throw runtime_error(error_code.message());
    }
    
    this->on_open = on_open;
    this->on_log = on_log;
    client.connect(pConnection);
    client.run();

    client.reset();
    bConnected = false;
}

void RippleSocket::stop()
{
    if (bConnected) {
        pConnection->close(websocketpp::close::status::going_away, "");
    }
}

void RippleSocket::sendCommand(Object& cmd, callback_t callback)
{
    cmd.push_back(Pair("id", sequence));
    if (callback) {
        callback_map[sequence] = callback;
    }
    sequence++;
    string cmdStr = write_string<Value>(cmd, false);
    if (on_log) on_log(string("Sending command: ") + cmdStr);
    pConnection->send(cmdStr);
}

RippleSocket& RippleSocket::on(const string& messageType, callback_t handler)
{
    msg_handler_map[messageType] = handler;
    return *this;
}

/// Protected Methods
void RippleSocket::onOpen(connection_hdl_t hdl)
{
    bConnected = true;
    if (on_log) on_log("Connection opened.");
    if (on_open) on_open();
}

void RippleSocket::onClose(connection_hdl_t hdl)
{
    bConnected = false;
    if (on_log) on_log("Connection closed.");
}

void RippleSocket::onFail(connection_hdl_t hdl)
{
    bConnected = false;
    if (on_log) on_log("Connection failed.");
}

void RippleSocket::onMessage(connection_hdl_t hdl, message_ptr_t msg)
{
    try {
        string json = msg->get_payload();
        Value value;
        read_string(json, value);
        const Object& obj = value.get_obj();
        const Value& typeVal = find_value(obj, "type");
        if (typeVal.type() == null_type) {
            throw runtime_error("Message type not found.");
        }
        string messageType = typeVal.get_str();
        if (messageType == "response") {
            onResponse(obj);
        }
        else {
            auto it = msg_handler_map.find(messageType);
            if (it != msg_handler_map.end()) {
                it->second(obj);
            }
            else {
                if (on_log) on_log(string("Unhandled message type: ") + messageType);
            }
        }
    }
    catch (const exception& e) {
        if (on_log) on_log(string("Error parsing server payload: ") + msg->get_payload() + " - " + e.what());
    }
}

void RippleSocket::onResponse(const Object& obj)
{
    if (on_log) {
        on_log("Received a server response.");
        on_log(write_string<Value>(obj, true));
    }
    try {
        const Value& idVal = find_value(obj, "id");
        if (idVal.type() == null_type) {
            throw runtime_error("Response has no id.");
        }
        int64_t id = idVal.get_int64();
        auto it = callback_map.find(id);
        if (it != callback_map.end()) {
            it->second(obj);
            callback_map.erase(id);
        }
    }
    catch (const exception& e) {
        if (on_log) on_log(string("Error parsing response: ") + e.what());
    }
}
