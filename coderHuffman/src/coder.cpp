#include <fstream>
#include <iostream>
#include <string>
#include "console.h"
#include "strlib.h"
#include "vector.h"
#include "pqueue.h"

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
unsigned int countingWeightOfCharacters(ifstream & readFile, Vector <int> &);


/*
 * Builds a coding tree for characters of the text according to the Huffman's algoritm
 * @param Vector<int> takes a reference to the vector of number of occurences for all characters of ASCII
 * @return Node* pointer to the root of the coding tree
 */
Node* buildTree(Vector <int> & weightOfChars);


/*
 * Return true if the current node is leaf, that means it has an actual value of character
 */
bool isLeaf(Node *);


/*
 * Generates a table of charactars and their codes
 * @param Node* takes a pointer to the current node
 * @param Vector<bool> takes a reference to the vector for saving code of current character
 * @param Vector<Vector<bool> > takes a reference to the vector for saving coding table
 * @param int takes a reference for counting the number of codes (rows of table)
 */
void codingTable(Node* n, Vector<bool> &code, Vector<Vector<bool> > & codeTable, int &);


/*
 * Encodes the text and writes it to the text file
 * @param string takes a name of the file that will be archived
 * @param Vector<Vector<bool> > takes a reference to the vector with code table
 * @param ofstream takes a reference to the stream of open file to record as binary
 */
void codingText(string filename, Vector <Vector<bool> > & codeTable, ofstream & writefile);


/*
 * Records the code table to the beginning of the file
 * @param ofstream takes a reference to the stream of open file to record as binary
 * @param Vector<Vector<bool> > takes a reference to the vector with code table
 * @param unsigned int takes a number of all characters in the text
 * @param int takes a number of encoded characters
 */
void writeTable(ofstream & writeFile, Vector< Vector<bool> > & codeTable, unsigned int numOfChars, int numOfCodes);


/*
 * Writes code of character as set of bytes where each bit represens a bool value
 * @param ofstream takes a reference to the stream of open file record as binary
 * @param Vector<bool> takes a code of the character
 */
void writeCode(ofstream & writefile, Vector<bool> code);


/*
 * Writes code of character to the output file by bytes where a sequence of bits is equal to the sequence of bool values of code.
 * @param ofstream takes a reference to the stream of open file record as binary
 * @param Vector<bool> takes a code of the character
 * @param char & takes a reference to the character which represents encoded byte.
 * @param int & takes a reference to the possition of a current bit of encoded byte.
 */
void codingChar(ofstream & writefile, Vector<bool> code, char & byte, int & possition);



/*
 * Appends the required number of bits for a byte to write.
 * @param ofstream takes a reference to the stream of open file record as binary
 * @param char takes a character which represents encoded byte.
 * @param int takes a possition of a current bit of encoded byte.
 */
void extendedToByte(ofstream & writefile, char byte, int possition);



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

    Vector<int>weightOfChars(256, 0); // create a vector with size 256 initialized with zeros for saving number of occurences for each character
    unsigned int numOfChars = countingWeightOfCharacters(readFile, weightOfChars);

    Node* rootPtr = buildTree(weightOfChars);

    Vector<bool> code; // declare a Vector for saving pass to a particular character, which corresponds to the code
    Vector< Vector<bool> > codeTable(256); // declare a Vector for saving codes for each character, where possition of code is equal to ASCII code of character
    int numOfCodes = 0;
    if(rootPtr){ // if file is not empty
        if (isLeaf(rootPtr)){ // if file consists only one character assign it a code
            code.add(0);
        }
        codingTable(rootPtr, code, codeTable, numOfCodes);
    }
    // composes a name for archive file by adding "Arch" between name of the source file and its extencion
    string archFile = filename.substr(0, filename.length() - 4) + "Arch.txt";
    ofstream writeFile(archFile.c_str(), ios::binary); // create an object of stream class to archive a file


    writeTable(writeFile, codeTable, numOfChars, numOfCodes);

    codingText(filename, codeTable, writeFile);

    return 0;
}


string requestForFileName(ifstream & readFile){
    while (true){
        string fileName = "";
        cout << "Please, enter file name: " << endl;
        cin >> fileName;
        readFile.open(fileName.c_str(), ios::binary); //leads a string to a c-style to open a file for reading the file as binary
        if(readFile){
            return fileName;
        }
        readFile.clear(); // resets the status indecator in the stream for reuse it.
        cout << "The file can not be found. Check the correctness of the file name." << endl;
    }
}


unsigned int countingWeightOfCharacters(ifstream & readFile, Vector <int> & weightOfChars){
    char ch; // declare a variable for reading a file by single character
    unsigned int numOfChars = 0; // number of characters in the source file
    // reads the file while it has next character
    while (readFile.get(ch)){
        weightOfChars[(ch + 128)]++; // possition in the vector corresponds to ASCII code of character +128 (adding 128 to avoid negative values)
        ++numOfChars;
    }
    readFile.close();

    return numOfChars;
}




Node* buildTree(Vector <int> & weightOfChars){
    PriorityQueue<Node*> weightsOfNodes; // declare a PriorityQueue for simple sorting weights of nodes
    for(int i = 0; i < weightOfChars.size(); ++i){
        if(weightOfChars[i] > 0){
            Node* node = new Node((i - 128), weightOfChars[i]);
            weightsOfNodes.enqueue(node, weightOfChars[i]);
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


void codingTable(Node* n, Vector<bool> & code, Vector< Vector<bool> > & codeTable, int & numOfCodes){

    if(n->left != NULL){
        code.add(0);
        codingTable(n->left, code, codeTable, numOfCodes);
    }
    if(n->right != NULL){
        code.add(1);

        codingTable(n->right, code, codeTable, numOfCodes);
    }

    if(isLeaf(n)){

        codeTable.set((n->value + 128), code);
        ++numOfCodes; // counter of coded symbols

    }
    if(code.size() > 0){
        code.remove(code.size()-1); // remove the last symbol in the code when returning one level above
    }
}


bool isLeaf(Node * n){
    return ((n->left == NULL) && (n->right == NULL));
}



void writeTable(ofstream & writeFile, Vector< Vector<bool> > & codeTable, unsigned int numOfChars, int numOfCodes){
    writeFile.write((char *)&numOfChars, sizeof(numOfChars));
    writeFile.put(numOfCodes);

    for(int i = 0; i < codeTable.size(); ++i){
        if (codeTable[i].size() > 0){
            writeFile.put((i - 128));
            writeFile.put(codeTable[i].size());
            writeCode(writeFile, codeTable[i]);
        }
    }
}


void writeCode(ofstream & writefile, Vector<bool> code){
      while(code.size() % 8){ // if the code is not a multiple of 8
          code.add(0);          // append it with 0
      }
      int size = (code.size() / 8 );
      char buff [size];
      char byte = 0;

      for (int i = 0; i < code.size(); ++i){
          byte = (byte << 1) | code[i]; // shift by one and perform bitwise addition
          if ((i + 1) % 8 == 0){ // after 7 shift obtain a byte
              buff[i/8] = byte;
              byte = 0;
          }
      }
      writefile.write(buff, size);
}


void extendedToByte(ofstream & writefile, char byte, int possition){
    while (possition < 8){
        byte = byte | 0 << (7 - possition); // append zeros to the last byte from getting position
        ++possition;
    }
    writefile << byte;

}


void codingChar(ofstream & writefile, Vector<bool> code, char & byte, int & possition){
        for(int i = 0; i < code.size(); ++i){
            byte = byte | (code[i] << (7 - possition)); // shift to the right possition to put the correct bit
            ++possition;
            if(possition == 8){ // when the byte is complete
                writefile << byte;
                byte = 0;
                possition = 0;
            }
        }
}

void codingText(string filename, Vector< Vector<bool> > & codeTable, ofstream & writeFile){

    ifstream readFile;
    readFile.open(filename.c_str(), ios::binary); // open sourse file as binary

    char ch; // declare a variable for reading a file by single character
    char byte = 0; // to form a byte to write
    int possition = 0; // possition of the current bit in the byte
    // reads the file while it has next character
    while (readFile.get(ch)){
        codingChar(writeFile, codeTable[ch + 128], byte, possition);
    }
    if (possition > 0){
        extendedToByte(writeFile, byte, possition);
    }
    readFile.close();
    writeFile.close();
}


