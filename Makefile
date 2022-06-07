#use make servidor para compilar e executar o codigo do servidor
servidor:
	g++ ./src/servidor.cpp -o ./exe/server.exe -pthread
	./exe/server.exe

#use make cliente para compilar e executar o codigo do cliente
cliente:
	g++ ./src/cliente.cpp -o ./exe/client.exe -pthread
	./exe/client.exe