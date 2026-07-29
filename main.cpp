#include <bits/stdc++.h>
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

int max_size;
unordered_map<string, Node*> mpp;
Node* head;     // dummy head and tail for easy accessing
Node* tail;

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

// get prints out value
void get(string key) {
    if(mpp.find(key)==mpp.end()) {
        cout<<"not found"<<endl;
        return ;
    }
    removeNode(mpp[key]);
    placeFront(mpp[key]);
    cout<<mpp[key]->value<<endl;
    return;
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
    cout<<"OK"<<endl;
}



void DELETE(string key) {
    if (mpp.find(key) != mpp.end()) {
        deleteNode(mpp[key]);
        cout << "OK" << endl;
    } 
    else {
        cout << "not found" << endl;
    }
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

int main() {

    int capacity;
    cout << "enter cache size: ";
    cin >> capacity;
    cin.ignore(); // clears the leftover newline so getline() works correctly right after
    LRUCache(capacity);

    // ifstream is used to read from file
    ifstream inFile("data.txt");
    loadFromFile(inFile);
    inFile.close();

    while (true) {
        string command;
        cout << "> ";
        // cin but reads line and stops at endl (cin stops at " ")
        getline(cin, command);

        stringstream ss(command);
        string function, key, val;
        ss >> function >> key;

        if (function == "SET") {
            if(ss >> val){
                put(key, val);
            }
        } else if (function == "GET") {
            get(key);
        } else if (function == "DELETE") {
            DELETE(key);
        } else if (function == "EXIT") {
            break;
        } else {
            cout << "Unknown command" << endl;
        }
    }
// ofstream is used to write into the file
    //the map contains all the values from previous times the code is executed
    // create file if doesnt exist, truncate file if exists
    ofstream outFile("data.txt"); 
    saveToFile(outFile);
    outFile.close();
    return 0;
}