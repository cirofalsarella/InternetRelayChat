#use make servidor para compilar e executar o codigo do servidor
servidor:
	g++ -o servidor servidor.cpp
	./servidor

#use make cliente para compilar e executar o codigo do cliente
cliente:
	g++ -o cliente cliente.cpp
	./cliente