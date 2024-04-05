ifeq ($(shell uname),Linux)
	BIN = /var/www/authoring/converter/bin/linux64/convertScShuttered
else
	BIN = /Users/toshi/SDK/Communicator/HOOPS_Communicator_2024/authoring/converter/bin/macos/convertScShuttered
endif

OBJS = ImportExport.cpp utilities.cpp Matrix.cpp
CC = g++ -std=c++11

ifeq ($(shell uname),Linux)
	PATH_TO_INCLUDES = -I /usr/local/include \
					-I /var/hoops/HOOPS_Exchange_Publish_2024.1.0/include \
					-I /var/www/authoring/libconverter/include

	PATH_TO_LIBCONVERTER = /var/www/authoring/converter/bin/linux64
else
	PATH_TO_INCLUDES = -I /usr/local/include \
			-I /Users/toshi/SDK/Exchange/HOOPS_Exchange_2024.1.0/include \
			-I /Users/toshi/SDK/Communicator/HOOPS_Communicator_2024.1.0/authoring/libconverter/include

	PATH_TO_LIBCONVERTER = /Users/toshi/SDK/Communicator/HOOPS_Communicator_2024.1.0/authoring/converter/bin/macos
endif

LDLIBS = -ldl -lstdc++ \
	 -L$(PATH_TO_LIBCONVERTER) \
	 -lconverter \
	 -lstdc++fs

$(BIN): $(OBJS)
	$(CC) $(PATH_TO_INCLUDES) $(OBJS) -o $(BIN) $(LDLIBS)

.PHONY: clean
clean:
	rm -f $(BIN)
