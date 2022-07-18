# Internet Relay Chat
   
| Name | #USP |
|--|--|
| Ciro Grossi Falsarella | 11795593 |
| Guilherme Jun Grilo | 11208350 |
| Victor Paulo Cruz Lutes | 11795512 |

---  

# PT-BR

## Inspiração  
Essa é uma atividade para se colocar em prática a disciplina "Redes de Computadores".

## Como funciona (PT-BR)

Para se comunicar um dos clientes terá que agir como Servidor. Logo, use o comando `make servidor -B` para iniciar a execução de um dos clientes que servirá como Servidor. Depois execute o comando `make cliente -B`, para se comunicar com o servidor. Com isso os dois programas podem enviar mensagens um para o outro.

O copy paste para mensagens maiores do que 4096 bytes não funciona, mas mensagens que são maiores do que esse tamanho sao divididas, enviadas e remontadas.

Envie a mensagem `/quit` para parar a execucao.

|Comandos| |
|--|--|
| /connect | Cliente estabelece uma conexão com o servidor
| /quit | Cliente fecha a aplicação
| /ping | Cliente envia um ping para testar a conexão com o servidor (espera-se um 'pong')
| /join [NOME_CANAL] | Entra ou cria um canal
| /nickname [NICKNAME] | Cliente muda seu nickname

|Comandos (admins)| |
|--|--|
| /kick [NICKNAME] | Fecha a conexão de um cliente
| /mute [NICKNAME] | Bloqueia um usuário de mandar mensagens em um canal
| /unmute [NICKNAME] | Desbloqueia um usuário de mandar mensagens em um canal
| /whois [NICKNAME] | Exibe o endereço IP de um usuário

---

## EN

## Inspiration  
This is an activity to put in practice the subject Computer Networks.

## How it works (EN)

For a client communicate to a server, you have to use the terminal command `make servidor -B` to start an execution of a client as a server. Then, run `make cliente -B` to create a client that will communicate to the running server. After that, the both programs can send messages to each other.

If you paste a message that have 4096 bytes longer, it will not work. But if you write them, they will be divided and sent correctly.

Send a `/quit` message to stop the client execution.

|Commands| |
|--|--|
| /connect | Stablish connection with server
| /quit | Client closes the application
| /ping | Server send a ping to test connection (expect 'pong')
| /join [CHANNEL_NAME] | Join or create a channel  
| /nickname [NICKNAME] | Client will be recognized by the nickname

|Commands (admins)| |
|--|--|
| /kick [NICKNAME] | Close connection of an user
| /mute [NICKNAME] | Forbids an user of sending messages
| /unmute [NICKNAME] | Removes mute of an user  
| /whois [NICKNAME] | Returns IP address of an user

