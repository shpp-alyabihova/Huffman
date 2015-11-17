#include <fstream>
#include <iostream>
#include <string>
#include "console.h"
#include "strlib.h"
#include "vector.h"
#include "pqueue.h"
#include "bitstream.h"

using namespace std;


//Defines the class for saving nodes of coding tree
class Node{
public:
    //default constructor 
    Node(){}
    
    //constructor for leafs of coding tree
    Node(char ch, int weight)
        : value(ch), weight(weight){
        left = NULL;
        right = NULL;
    }

    //constuctor for nodes, that composed from other nodes
    Node(Node *l, Node *r)
        : left(l), right (r) {
        weight = l->weight + r->weight;
    }
    
    ~Node(){
        delete left;
        delete right;
    }


    char value; // for saving character that is found in the text
    int weight; // number of character's occurences in the text
    Node *left; // a pointer to the left child
    Node *right; // a pointer to the right child
};


/*
 * Counts the number character's occurences in the text
 * @param ifstream takes reference to the stream of open file for reading
 * @param Vector<int> takes a reference to the vector for number of occurences for all characters of ASCII
 * @return int number of all characters in the text
 */
int countingWeightOfCharacters(ifstream & readFile, Vector <int> &);


/*
 * Builds a coding tree for characters of the text according to the Huffman's algoritm
 * @param Vector<int> takes a reference to the vector of number of occurences for all characters of ASCII
 * @param char takes a reference to the character with number of occurences which is equal zero
 * @return Node* pointer to the root of the coding tree
 */
Node* buildTree(Vector <int> & weightOfChars, char & delimiter);


/*
 * Return true if the current node is leaf, that means it has an actual value of character
 */
bool isLeaf(Node *);


/*
 * Generates a table of charactars and their codes
 * @param Node* takes a pointer to the current node
 * @param Vector<int> takes a reference to the vector for saving code of current character
 * @param Vector<Vector<int> > takes a reference to the vector for saving coding table
 * @param int takes a reference for counting the number of codes (rows of table)
 */
void codingTable(Node* n, Vector<int> &code, Vector<Vector<int> > & codeTable, int &);


/*
 * Encodes the text and writes it to the text file
 * @param string takes a name of the file that will be archived
 * @param Vector<Vector<int> > takes a reference to the vector with code table
 * @param ofbitstream takes a reference to the stream of open file to a bit-recording
 */
void codingText(string filename, Vector <Vector<int> > & codeTable, ofbitstream & writefile);


/*
 * Records the code table to the beginning of the file
 * @param ofbitstream takes a reference to the stream of open file to a bit-recording
 * @param Vector<Vector<int> > takes a reference to the vector with code table
 * @param char takes a character that hasn't been founded in the source text
 * @param int takes a number of all characters in the text
 * @param int takes a number of all codes (rows of the table)
 */
void writeTable(ofbitstream & writeFile, Vector< Vector<int> > & codeTable, char delimiter, int numOfChars, int numOfCodes);


/*
 * Bitwise record one byte of information
 * @param ofbitstream takes a reference to the stream of open file to a bit-recording
 * @param char takes a character for recording to the file
 */
void writeByte(ofbitstream & writefile, char ch);


/*
 * Bitwise record integer to the open file
 * @param ofbitstream takes a reference to the stream of open file to a bit-recording
 * @param int takes a integer for recording to the file
 */
void writeInteger(ofbitstream & writefile, int number);


/*
 * Prompts the user for a filename and tries to open file. If an unsuccessful attempt
 * to open a file again prompts the user for a filename. Otherwise returns a filename.
 */
string requestForFileName(ifstream &);


/*
 *  Prompts the user for a filename that he wants to archive and creates a new text file that has a smaller size
 *  which is achieved by applying Huffman's algoritm. A table with codes saves at the beggining of the archive file.
 */
int main() {
    ifstream readFile; // create an object of stream class to open a file
    string filename = requestForFileName(readFile);

    Vector<int>weightOfChars(255, 0); // create a vector with size 255 initialized with zeros for saving number of occurences for each character
    int numOfChars = countingWeightOfCharacters(readFile, weightOfChars);


    //declare a char with a space as default value, that will be used as a delimiter in archive file.
    char delimiter = ' ';

    Node* rootPtr = buildTree(weightOfChars, delimiter);

    Vector<int> code; // declare a Vector for saving pass to a particular character, which corresponds to the code
    Vector< Vector<int> > codeTable(255); // declare a Vector for saving codes for each character, where possition of code is equal to ASCII code of character
    int numOfCodes = 0;
    if(rootPtr){ // if file is not empty
        if (isLeaf(rootPtr)){ // if file consists only one character assign it a code
            code.add(0);
        }
        codingTable(rootPtr, code, codeTable, numOfCodes);
    }
    // composes a name for archive file by adding "Arch" between name of the source file and its extencion
    string archFile = filename.substr(0, filename.length() - 4) + "Arch.txt";
    ofbitstream writeFile(archFile); // create an object of bit stream class to archive a file


    writeTable(writeFile, codeTable, delimiter, numOfChars, numOfCodes);

    codingText(filename, codeTable, writeFile);

    return 0;
}


string requestForFileName(ifstream & readFile){
    while (true){
        string fileName = "";
        cout << "Please, enter file name: " << endl;
        cin >> fileName;
        readFile.open(fileName.c_str()); //leads a string to a c-style to open a file
        if(readFile){
            return fileName;
        }
        readFile.clear(); // resets the status indecator in the stream for reuse it.
        cout << "The file can not be found. Check the correctness of the file name." << endl;
    }
}


int countingWeightOfCharacters(ifstream & readFile, Vector <int> & weightOfChars){
    char ch; // declare a variable for reading a file by single character
    int numOfChars = 0; // number of characters in the source file
    // reads the file while it has next character
    while (readFile.get(ch)){
        weightOfChars[ch]++;
        ++numOfChars;
    }
    readFile.close();

    return numOfChars;
}




Node* buildTree(Vector <int> & weightOfChars, char & delimiter){
    PriorityQueue<Node*> weightsOfNodes; // declare a PriorityQueue for simple sorting weights of nodes
    bool flag = 1; // set flag to prevent overwriting a delimiter
    for(int i = 0; i < weightOfChars.size(); ++i){
        if(weightOfChars[i] > 0){
            Node* node = new Node(i, weightOfChars[i]);
            weightsOfNodes.enqueue(node, weightOfChars[i]);
        }
        else if(flag && (i > 31)){ // looking for a character that is not a control code and has a number of occurences which is equal zero
            delimiter = i;
            flag = 0;
        }
    }

    // if file is empty the pointer of root of the tree is a NULL
    if(weightsOfNodes.isEmpty()){
        return NULL;
    }


    while (weightsOfNodes.size() > 1) {
        Node* lNode = weightsOfNodes.dequeue();
        Node* rNode = weightsOfNodes.dequeue();
        Node* newNode = new Node(lNode, rNode);
        weightsOfNodes.enqueue(newNode, newNode->weight);
    }


    Node* root = weightsOfNodes.dequeue();


    return root;
}


void codingTable(Node* n, Vector<int> & code, Vector< Vector<int> > & codeTable, int & numOfCodes){

    if(n->left != NULL){
        code.add(0);
        codingTable(n->left, code, codeTable, numOfCodes);
    }
    if(n->right != NULL){
        code.add(1);

        codingTable(n->right, code, codeTable, numOfCodes);
    }

    if(isLeaf(n)){
        codeTable.set(n->value, code);
        ++numOfCodes; // counter of coded symbols

    }
    if(code.size() > 0){
        code.remove(code.size()-1); // remove the last symbol in the code when returning one level above
    }
}


bool isLeaf(Node * n){
    return ((n->left == NULL) && (n->right == NULL));
}



void writeTable(ofbitstream & writeFile, Vector< Vector<int> > & codeTable, char delimiter, int numOfChars, int numOfCodes){

    writeByte(writeFile, delimiter); // write a delimiter to delimit information in the archive file
    writeInteger(writeFile, numOfChars); // write a number of all symbols of the sourse file
    writeByte(writeFile, delimiter); // write a delimiter to delimit information in the archive file
    writeInteger(writeFile, numOfCodes); // write a number of coded symbols of the sourse file
    writeByte(writeFile, delimiter); // write a delimiter to delimit information in the archive file
    for(int i = 0; i < codeTable.size(); ++i){
        if (codeTable[i].size() > 0){
            writeByte(writeFile, i); // write a character - one of coded symbols of the sourse file
            for(int j = 0; j < codeTable[i].size(); ++j){
                writeByte(writeFile, codeTable[i][j]); // write his code
            }
            writeByte(writeFile, delimiter);// write a delimiter to delimit information in the archive file
        }
    }
}


void writeByte(ofbitstream & writefile, char ch){
    int bit;
    // use bitwise multiplication and bit shift unit for determining current bit of given character
    for(int i = 7; i >= 0; --i){
        if(ch & (1 << i)){
            bit = 1;
        }
        else bit = 0;
        writefile.writeBit(bit);
    }
}


void writeInteger(ofbitstream & writefile, int number){
    string num = integerToString(number);
    for(int i = 0; i < num.length(); ++i){
        writeByte(writefile, num[i]);
    }
}

void codingText(string filename, Vector< Vector<int> > & codeTable, ofbitstream & writeFile){

    ifstream readFile;
    readFile.open(filename.c_str()); // open sourse file

    char j; // declare a variable for reading a file by single character
    // reads the file while it has next character
    while (readFile.get(j)){
        for(int i = 0; i < codeTable[j].size(); ++i){ // necessary character is located at possition (index) that corresponds to of its ASCII code
            writeFile.writeBit(codeTable[j][i]);
        }
    }
    readFile.close();
    writeFile.close();
}


