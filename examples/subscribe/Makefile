PROJECT_ROOT = ../..

WEBSOCKETPP_ROOT = ../../deps/websocketpp
JSON_SPIRIT_ROOT = ../../deps/json_spirit

CXX_FLAGS = -std=c++0x -Wall DWEBSOCKETPP_NOEXCEPT_TOKEN 

ifdef DEBUG
    CXX_FLAGS += -g
else
    CXX_FLAGS += -O3
endif

LIBS = \
    -l pthread \
    -l boost_random \
    -l boost_system \
    -l boost_thread

$(PROJECT_ROOT)/src/obj/%.o: $(PROJECT_ROOT)/src/%.cpp
	$(CXX) $(CXX_FLAGS) -c -o $@ $< \
	-I$(PROJECT_ROOT)/src -I$(WEBSOCKETPP_ROOT) -I$(JSON_SPIRIT_ROOT)

subscribe: src/subscribe.cpp $(PROJECT_ROOT)/src/obj/ripplesocket.o
	$(CXX) $(CXX_FLAGS) -o build/$@ $^ \
	-I$(PROJECT_ROOT)/src -I$(WEBSOCKETPP_ROOT) -I$(JSON_SPIRIT_ROOT) \
	$(WEBSOCKETPP_ROOT)/libwebsocketpp.a \
	$(LIBS)

#	g++ -std=c++0x -Wall -o build/test_ripplesocket test_ripplesocket.cpp ../ripplesocket.cpp -I../../deps/websocketpp ../../deps/websocketpp/libwebsocketpp.a -I../../deps/json_spirit -l pthread -l boost_random -l boost_system -l boost_thread
