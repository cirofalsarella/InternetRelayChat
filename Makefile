#use make servidor para compilar e executar o codigo do servidor
servidor:
	g++ servidor.cpp -o servidor -pthread
	./servidor

#use make cliente para compilar e executar o codigo do cliente
cliente:
	g++ cliente.cpp -o cliente -pthread
	./cliente
