INCL = -I ./3rdpart/st-1.9/LINUX_5.15.0-84-generic_DBG/
LIBRARY = -L ./3rdpart/st-1.9/LINUX_5.15.0-84-generic_DBG/
SRC_FILES = main_server.cpp server.cpp client.cpp io_socket.cpp rtmp_protocol.cpp io_message.cpp io_buffer.cpp value_object.cpp util.cpp all_kinds_of_messages.cpp socket_demo_define.h

DIRECTORY = binary_o

BINARY_FILE = ${DIRECTORY}/main_server.o ${DIRECTORY}/server.o ${DIRECTORY}/client.o ${DIRECTORY}/io_socket.o ${DIRECTORY}/rtmp_protocol.o ${DIRECTORY}/io_message.o ${DIRECTORY}/io_buffer.o ${DIRECTORY}/value_object.o ${DIRECTORY}/util.o ${DIRECTORY}/all_kinds_of_messages.o

default: PATH ${BINARY_FILE}
	g++ ${BINARY_FILE} ${INCL} ${LIBRARY} -lst -o server -g

PATH:
	if [ ! -d "${DIRECTORY}" ]; then \
		mkdir "${DIRECTORY}"; \
	fi

${DIRECTORY}/main_server.o: main_server.cpp
	g++ -c main_server.cpp ${INCL} -o ${DIRECTORY}/main_server.o -g

${DIRECTORY}/server.o: server.cpp
	g++ -c server.cpp ${INCL} -o ${DIRECTORY}/server.o -g

${DIRECTORY}/client.o: client.cpp
	g++ -c client.cpp ${INCL} -o ${DIRECTORY}/client.o -g

${DIRECTORY}/io_socket.o: io_socket.cpp
	g++ -c io_socket.cpp ${INCL} -o ${DIRECTORY}/io_socket.o -g

${DIRECTORY}/rtmp_protocol.o: rtmp_protocol.cpp
	g++ -c rtmp_protocol.cpp ${INCL} -o ${DIRECTORY}/rtmp_protocol.o -g

${DIRECTORY}/io_message.o: io_message.cpp
	g++ -c io_message.cpp ${INCL} -o ${DIRECTORY}/io_message.o -g

${DIRECTORY}/io_buffer.o: io_buffer.cpp
	g++ -c io_buffer.cpp ${INCL} -o ${DIRECTORY}/io_buffer.o -g

${DIRECTORY}/value_object.o: value_object.cpp
	g++ -c value_object.cpp ${INCL} -o ${DIRECTORY}/value_object.o -g

${DIRECTORY}/util.o: util.cpp
	g++ -c util.cpp ${INCL} -o ${DIRECTORY}/util.o -g

${DIRECTORY}/all_kinds_of_messages.o: all_kinds_of_messages.cpp
	g++ -c all_kinds_of_messages.cpp ${INCL} -o ${DIRECTORY}/all_kinds_of_messages.o -g

clean:
	rm -f ${DIRECTORY}/*.o
