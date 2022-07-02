#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <string>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <map>
#include <vector>

#define MAX 80
#define PORT 3002
#define SA struct sockaddr
using namespace std;
/*
• /connect - Estabelece a conexão com o servidor;
• /quit - O cliente fecha a conexão e fecha a aplicação;
• /ping - O servidor retorna "pong"assim que receber a mensagem.

• /join nomeCanal - Entra no canal;
• /nickname apelidoDesejado - O cliente passa a ser reconhecido pelo apelido especificado;
• /ping - O servidor retorna "pong"assim que receber a mensagem.
Comandos apenas para administradores de canais:
• /kick nomeUsuario - Fecha a conexão de um usuário especificado
• /mute nomeUsuario - Faz com que um usuário não possa enviar mensagens neste canal
• /unmute nomeUsuario - Retira o mute de um usuário.
• /whois nomeUsuario - Retorna o endereço IP do usuário apenas para o administrador
*/


vector<pthread_t> recieveAndForwardThreads;
int connfd;
vector<int> connections; //lista dos connfd conectados VAI PRECISAR USAR LOCK
map<int, string> connToIp;
map<int, pthread_t> connToThread;
map<int, vector<string>> connToChannels;//mapeia os connfd nos caneis que ele está conectado VAI PRECISAR USAR LOCK
map<string, int> nickToConnfd;//mapeia os strings nick para os respectivos enderecos connfd VAI PRECISAR USAR LOCK
map<string, vector<int>> channelsToConns;//mapeia os channel names para os connfds conectados a ele VAI PRECISAR USAR LOCK
map<string, int> channelsToAdmins;//mapeia os channel names para os connfd dos admins
map<string, vector<int>> muted;//mapeia os canais na lista de mutados
bool hasQuit=false;

void removeAndDisconnect(int con){
    for(int i=0; i<(int) connections.size(); ++i){
        if(connections[i]==con) connections.erase(connections.begin()+i);
    }
    connToIp.erase(con);
    connToChannels.erase(con);
}

bool sendAndAck(int con, char *buffer){
    char rec[4096] = { 0 };
    memset(rec, 0, sizeof(buffer));
    for(int i=0; i<5; ++i){
        send(con, buffer, 4096, MSG_NOSIGNAL);
        read(con, rec, 4096);
        if(rec[0]=='a' && rec[1]=='c' && rec[2]=='k') return true;
    }
    //nao recebeu ack depois de 5 tentativas, fechar conexao
    return false;
}

void sendMessageToChannel(int  sock, string nick, string channel, char *buffer){
    vector<int> connectedClients=channelsToConns[channel];
    vector<int> connectionsToDisconnect;
    bool recAck;
    string firstMessage="Message from client \""+nick+"\" to channel \"" + channel +"\": ";
    cout<<firstMessage;
    //check if there is a problem with sending an intro
    char bufferForIntro[4096];
    memset(bufferForIntro, 0, sizeof(bufferForIntro));
    int pos=0;
    for(; pos<firstMessage.length(); ++pos){
        bufferForIntro[pos]=firstMessage[pos];
    }
    bufferForIntro[pos]='\0';
    bufferForIntro[4095]=' ';
    for(int i=0; i<(int) connectedClients.size(); ++i){
        recAck=sendAndAck(connectedClients[i], bufferForIntro);
        if(!recAck) connectionsToDisconnect.push_back(connectedClients[i]);
    }
    printf("%s", buffer);
    for(int i=0; i<(int) connectedClients.size(); ++i){
        sendAndAck(connectedClients[i], buffer);
    }
    if(buffer[4095]!='\0'){
        char buffer2[4096] = { 0 };
        memset(buffer2, 0, sizeof(buffer2));
        read(sock, buffer2, 4096);
        while (buffer2[4095]!='\0'){
            memset(buffer2, 0, sizeof(buffer2));
            read(sock, buffer2, 4096);
            printf("%s", buffer2);
            for(int i=0; i<(int) connectedClients.size(); ++i){
                sendAndAck(connectedClients[i], buffer);
            }
        }
    }
    printf("\n");
    for(int i=0; i<(int) connectionsToDisconnect.size(); ++i){
        removeAndDisconnect(connectionsToDisconnect[i]);
        pthread_cancel(connToThread[connectionsToDisconnect[i]]);
        cout << "connection" << connectionsToDisconnect[i] <<" has been closed for not sending ack" << endl;
    }
}

void sendPongToAll(){
    vector<int> connectionsToDisconnect;
    bool recAck;
    string pong="pong";
    cout<<pong << endl;
    //check if there is a problem with sending an intro
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    int pos=0;
    for(; pos<pong.length(); ++pos){
        buffer[pos]=pong[pos];
    }
    buffer[pos]='\0';
    for(int i=0; i<(int) connections.size(); ++i){
        recAck=sendAndAck(connections[i], buffer);
        if(!recAck) connectionsToDisconnect.push_back(connections[i]);
    }    
    for(int i=0; i<(int) connectionsToDisconnect.size(); ++i){
        removeAndDisconnect(connectionsToDisconnect[i]);
        cout << "connection" << connectionsToDisconnect[i] <<" has been closed for not sending ack" << endl;
    }
}

bool isChannelName(string s){
    if(s[0]!='&' && s[0]!='#') return false;
    if(s.length()>200) return false;
    if(s.find(' ') != string::npos || s.find(',') != string::npos || s.find(7) != string::npos) return false;
    return true;
}

void *recieveAndForward(void *sock){
    int network_socket = *(int *)sock;
    string nick="nick"+to_string(network_socket);
    nickToConnfd[nick]=network_socket;
    char buffer[4096] = { 0 };
    while(!hasQuit){
        memset(buffer, 0, sizeof(buffer));
        read(network_socket, buffer, 4096);
        if(buffer[0]!=0 && (buffer[0]!='a'  || buffer[1]!='c' || buffer[2]!='k')){
            if(buffer[0]=='/'){
                if ((strstr(buffer, "/quit")) != NULL) {
                    cout<<nick<<"has quit"<<endl;
                    removeAndDisconnect(network_socket);
                    pthread_exit(NULL);
                    return NULL;
                }
                else if((strstr(buffer, "/ping")) != NULL){
                    sendPongToAll();
                }
                else if((strstr(buffer, "/join")) != NULL){
                    string s=string(&buffer[6], strlen(buffer)-6);

                    if(channelsToConns.find(s)==channelsToConns.end()){
                        bool validName=isChannelName(s);
                        if(validName){
                            vector<int> v;
                            v.push_back(network_socket);
                            channelsToConns[s]=v;
                            channelsToAdmins[s]=network_socket;
                            connToChannels[network_socket].push_back(s);
                            cout<<nick<<" has created channel "<<s<<endl;
                        }
                        else{
                            cout<<nick<<" didnt create channel because name was invalid"<<endl;
                        }
                    }
                    else{
                        bool hasntJoined=true;
                        vector<int> connsInChannel=channelsToConns[s];
                        for(int i=0; i<(int) connsInChannel.size(); ++i){
                            if(connsInChannel[i]==network_socket) hasntJoined=false;
                        }
                        if(hasntJoined){
                            connToChannels[network_socket].push_back(s);
                            channelsToConns[s].push_back(network_socket);
                            cout<<nick<<" has joined channel "<<s<<endl;
                        }
                        else{
                            cout<<nick<<" has already joined channel "<<s<<endl;
                        }
                    }
                    
                }
                else if((strstr(buffer, "/nickname")) != NULL){
                    string s=string(&buffer[10], strlen(buffer)-10);
                    nickToConnfd.erase(nick);
                    nickToConnfd[s]=network_socket;
                    cout<<nick<<" has changed nick to "<<s<<endl;
                    nick=s;

                }
                else if((strstr(buffer, "/kick")) != NULL){
                    string name=string(&buffer[6], strlen(buffer)-6);
                    cout << name << endl;
                    int removing=nickToConnfd[name];
                    vector<string> hisChannels=connToChannels[removing];
                    vector<string> channels=connToChannels[network_socket];
                    for(int i=0; i<(int) channels.size(); ++i){
                        if(channelsToAdmins[channels[i]]==network_socket){
                            for(int j=0; j<(int) connToChannels[removing].size();++j){
                                if(channels[i].compare(connToChannels[removing][j])==0){
                                    connToChannels[removing].erase(connToChannels[removing].begin()+j);
                                    cout<<nick<<" has kicked "<<name<<" from channel "<<channels[i]<<endl;
                                }
                            }
                            vector<int> channelConnections=channelsToConns[channels[i]];
                            for(int j=0; j<(int) channelConnections.size();++j){
                                if(channelsToConns[channels[i]][j]==removing){
                                    channelsToConns[channels[i]].erase(channelsToConns[channels[i]].begin()+j);
                                }
                            }
                        }
                    }
                }
                else if((strstr(buffer, "/mute")) != NULL){
                    //olha se esse usuario é admin de um canal, se for, remove o usuario selecionado do canal
                    string name=string(&buffer[6], strlen(buffer)-6);
                    int muting=nickToConnfd[name];
                    vector<string> hisChannels=connToChannels[muting];
                    vector<string> channels=connToChannels[network_socket];
                    for(int i=0; i<(int) channels.size(); ++i){//olho nos canais do usuario para checar se ele é admin
                        if(channelsToAdmins[channels[i]]==network_socket){//nos canais que ele é admin
                            for(int j=0; j<(int) hisChannels.size();++j){//olho se quem ele quer mutar está nesse canal
                                if(channels[i].compare(hisChannels[j])==0){//se sim, adiciono na lista de mutados
                                    muted[channels[i]].push_back(muting);
                                    cout<<nick<<" has muted "<<name<<" in channel "<<channels[i]<<endl;
                                }
                            }
                        }
                    }
                }
                else if((strstr(buffer, "/unmute")) != NULL){
                    string name=string(&buffer[8], strlen(buffer)-8);
                    int unmuting=nickToConnfd[name];
                    vector<string> channels=connToChannels[network_socket];
                    for(int i=0; i<(int) channels.size(); ++i){
                        if(channelsToAdmins[channels[i]]==network_socket){
                            vector<int> mutedInChannel=muted[channels[i]];
                            for(int j=0; j<(int) mutedInChannel.size();++j){//olho se quem ele quer desmutar está nesse canal
                                if(mutedInChannel[j]==unmuting){//se sim, removo da lista de mutados
                                    muted[channels[i]].erase(muted[channels[i]].begin()+j);
                                    cout<<nick<<" has unmuted "<<name<<" in channel "<<channels[i]<<endl;
                                }
                            }
                        }
                    }
                }
                else if((strstr(buffer, "/whois")) != NULL){
                    string name=string(&buffer[7], strlen(buffer)-7);
                    int gettingIp=nickToConnfd[name];
                    vector<string> hisChannels=connToChannels[gettingIp];
                    vector<string> channels=connToChannels[network_socket];
                    bool canSeeHisIp=false;
                    for(int i=0; i<(int) channels.size(); ++i){
                        if(channelsToAdmins[channels[i]]==network_socket){
                            for(int j=0; j<(int) hisChannels.size();++j){
                                if(channels[i].compare(hisChannels[j])==0){
                                    canSeeHisIp=true;
                                }
                            }
                        }
                    }
                    if(canSeeHisIp){
                        char buffer2[4096] = {0};
                        memset(buffer2, 0, sizeof(buffer));
                        //write his ip to buffer
                        string hisIp="The IP address of client \"" + name + "\" is: "+connToIp[gettingIp];
                        strcpy(buffer2, hisIp.c_str());
                        sendAndAck(network_socket, buffer2);
                        cout<<nick<<" has seen ip of client "<<name<< endl;
                    }
                }
            }
            else{
                vector<string> connectedChannels=connToChannels[network_socket];
                for(int i=0; i<(int) connectedChannels.size(); ++i){
                    string s=connectedChannels[i];
                    bool isMuted=false;
                    vector<int> mutedInChannel=muted[s];
                    for(int j=0; j<(int) mutedInChannel.size(); ++j){
                        if(network_socket==mutedInChannel[j]) isMuted=true;
                    }
                    if(!isMuted) sendMessageToChannel(network_socket, nick, s, buffer);
                }

            }
        }
        
    }
    
    pthread_exit(NULL);
    return NULL;
}

int main(){
    int sockfd, len;
    struct sockaddr_in servaddr, cli;
    //criar um vetor de conexoes e um map para os nicks
   
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));
   
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
   
    bind(sockfd, (SA*)&servaddr, sizeof(servaddr));
   
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening...\n");
    len = sizeof(cli);
    //MAKE WHILE TO CONNECT MULTIPLE CLIENTS
    while(true){
        connfd = accept(sockfd, (SA*)&cli, (socklen_t*)&len);
        if (connfd < 0) {
            printf("server accept failed...\n");
            exit(0);
        }
        else
            printf("server accept the client...\n");
        connections.push_back(connfd);
        pthread_t newThread;
        recieveAndForwardThreads.push_back(newThread);
        connToThread[connfd]=newThread;
        string clientip(inet_ntoa(cli.sin_addr));
        connToIp[connfd]=clientip;
        pthread_create(&newThread, NULL,recieveAndForward, &connfd);
    }
    for(int i=0; i<(int) recieveAndForwardThreads.size(); ++i)
        pthread_join(recieveAndForwardThreads[i], NULL);

    close(sockfd);
    return 0;
}
