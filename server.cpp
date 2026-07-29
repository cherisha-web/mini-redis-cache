#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>
using namespace std;

// doubly linked list
class Node{
    public:
        string key;
        string value;
        Node* frontptr;
        Node* backptr;
    public:
        Node(string data,string val){
            key=data;
            value=val;
            frontptr=NULL;
            backptr=NULL;
        }
};
//so if i use any variable inside the function i defenitely have to declare them before the function

mutex mtx;
//mutex is just a lock when i call lock_guard mutex myLock(mtx);
//it gets locked and gets unlocked when it comes out of the bracket, thats why use bracket

int max_size;
unordered_map<string, Node*> mpp;
Node* head;     // dummy head and tail for easy accessing
Node* tail;



void saveToFile(ofstream &outFile){
    if (!outFile.is_open()){
        cout<< "error opening file"<< endl;
    }
    else{
        Node* node=tail->backptr;
        while(node!=head){
            outFile << node->key<<" "<<node->value<<endl;
            node=node->backptr;
        }
    }
}

void saveState(){ // to save data incase of power cuts
    ofstream outFile("data.txt"); 
    saveToFile(outFile);
    outFile.close();
}

//declare removeNode before deleteNode
// remove node from its place
void removeNode(Node* node){
    node->frontptr->backptr=node->backptr;
    node->backptr->frontptr=node->frontptr;
}

// delete least used node
void deleteNode(Node* node){
    removeNode(node);
    mpp.erase(node->key);
    delete(node);
} 

//put the recently use node in the front
void placeFront(Node* node){
    head->frontptr->backptr=node;
    node->frontptr=head->frontptr;
    head->frontptr=node;
    node->backptr=head;
}

//initialising required for LRU
void LRUCache(int capacity) {
    head=new Node("","");
    tail=new Node("","");
    head->frontptr=tail;
    tail->backptr=head;
    max_size=capacity;
}

string get(string key) {
    if(mpp.find(key)==mpp.end()){
        return "not found";
    }
    removeNode(mpp[key]);
    placeFront(mpp[key]);
    saveState();
    return mpp[key]->value;
}
    
void put(string key, string value) {
    if(mpp.find(key)==mpp.end()){
        if(mpp.size()==max_size){
            deleteNode(tail->backptr);
        }
        Node* temp=new Node(key,value);
        mpp.insert({key,temp});
            placeFront(temp);
    }
    else{
        mpp[key]->value=value;
        removeNode(mpp[key]);
        placeFront(mpp[key]);
    }
    saveState();
}

string DELETE(string key) {
    if (mpp.find(key) != mpp.end()) {
        deleteNode(mpp[key]);
        saveState();
        return "OK";
    }
    saveState();
    return "not found";
}


void handleClient(int clientSocket){
    // race condition, two threads when accessing a function simultaneously causing weird behavior
    // just lock shared functions if everything is locked, i.e recv() and send() also, its basically processing one client after another
    while(true){
        char buffer[1024]; // raw space to receive bytes into
        int bytesRead = recv(clientSocket, buffer, 1024, 0);  // fill it, get back how many bytes arrived

        if(bytesRead <= 0){
                // 0 means client disconnected cleanly, negative means an error
                close(clientSocket);
                return;
        }
        string command(buffer,bytesRead); // only read whats sent, conver char array into string 
        stringstream ss(command);
        string function, key, val,response;
        ss >> function >> key;

        if (function == "SET") {            
            if(ss >> val){
                {
                    lock_guard <mutex> myLock(mtx);
                    put(key, val);
                }
                response="OK";
            }
            else {
                response = "Error: SET requires a value";
            }
            send(clientSocket,response.c_str(),response.length(),0); // c_str returns pointer to the string
        } 
        else if (function == "GET") {
            {
                lock_guard <mutex> myLock(mtx);
                response=get(key);
            }
            send(clientSocket,response.c_str(),response.length(),0);
        } 
        else if (function == "DELETE") {
            {
                lock_guard <mutex> myLock(mtx);
                response=DELETE(key);
            }
            send(clientSocket,response.c_str(),response.length(),0);
        } 
        else if (function == "EXIT") {
            response = "Goodbye";
            send(clientSocket, response.c_str(), response.length(), 0);
            break;
        }
        else {
            response="Unknown command";
            send(clientSocket,response.c_str(),response.length(),0);
        }
    }
    close(clientSocket);
}

void loadFromFile(ifstream &inFile){
    //checks if file exists and opened successfully
    if(inFile.is_open()){
        string line;
        //reads line by lline until EOF
        while(getline(inFile,line)){
            stringstream sss(line);
            string name,data;
            if(sss >>name >> data){
                put(name,data);
            }
        }
    }
}

int main(){
    LRUCache(100);

    // ifstream is used to read from file
    ifstream inFile("data.txt");
    loadFromFile(inFile);
    inFile.close();

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        cout << "Socket creation failed\n";
        return 1;
    }
    // AF_INET -> use IPv4 (32 bytes,four decimal numbers rom 0-255 seperated by .)
    // SOCK_STREAM -> guarenteed order of information and lets us know if it failed

    sockaddr_in serverAddress;
    serverAddress.sin_family=AF_INET; // server uses IP4
    serverAddress.sin_port=htons(5000); //Host TO Network Short, It converts your port number into the format expected on the network.
    serverAddress.sin_addr.s_addr = INADDR_ANY; //means accept connections on any network interface. wifi, ethernet. like aims only works on iith vpn 

    if(bind(server_fd,(sockaddr*)&serverAddress,sizeof(serverAddress))<0){
        cout<<"error failed"<<endl;
        return 1;
    };

    if (listen(server_fd, 5) < 0) {
        cout << "Listen failed\n";
        return 1;
    }
    //a client can start connecting (the TCP handshake) before your program calls accept(). 
    //The OS holds those half-finished, not-yet-accepted connections in a queue that's what the 5 controls, how many can be waiting at once before the OS starts rejecting new ones. 
    //accept() is you, at your desk, calling the next person in line.

    sockaddr_in clientAddress; 
    socklen_t clientaddlen=sizeof(clientAddress);

    while(true){
        // When a client connects, the OS writes into that memory itself, filling in the connecting client's actual IP address and port. 
        // That's why you pass a pointer instead of the struct directly accept() needs to modify your variable in place.
        // so you one by one assign sockets to clients and then through threading process them all at a time
        // Assigning = one door, one person let in at a time. Serving = many separate rooms, all occupied and active simultaneously.
        int clientSocket=accept(server_fd, (sockaddr*)&clientAddress,&clientaddlen);
        if (clientSocket < 0) {
            cout << "Accept failed\n";
            break ;
        }
        thread t(handleClient,clientSocket); // clientSocket value copied by thread and passed to handleClient
        t.detach(); // handleClient now runs independantly
    }
    
    // ofstream is used to write into the file
    //the map contains all the values from previous times the code is executed
    // create file if doesnt exist, truncate file if exists

    close(server_fd);
    return 0;
}

