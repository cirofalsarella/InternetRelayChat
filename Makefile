#use make servidor -B para compilar e executar o codigo do servidor
servidor:
	g++ src/servidor.cpp -o servidor -pthread
	./servidor

#use make cliente -B para compilar e executar o codigo do cliente
cliente:
	g++ src/cliente.cpp -o cliente -pthread
	./cliente
