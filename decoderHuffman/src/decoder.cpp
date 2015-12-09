#include <fstream>
#include <iostream>
#include <string>
#include "console.h"
#include "strlib.h"
#include "vector.h"
#include "bitstream.h"

using namespace std;


//Defines the class for saving nodes of coding tree
class Node{
public:
    Node(){
        left = NULL;
        right = NULL;
    }
    ~Node(){
        delete left;
        delete right;
    }

    char value;
    Node *left;
    Node *right;
};

/*
 * Read from the file information about encoded text
 * @param ifbitstream takes a reference to the stream of open file for bit by bit reading
 * @param Vector <char> takes a reference to vector for saving encoded symbols
 * @param Vector<Vector<bool> > takes a reference to vector for saving codes of encoded symbols
 * @return int a number of symbols in sourse file
 */
unsigned int readTable(ifbitstream & readArch, Vector <char> & symbols, Vector<Vector<bool> > &codes);


/*
 * Reconstructs a coding tree with help of which has been done encoding of sourse text
 * @param Vector <char> takes a reference to vector of encoded symbols
 * @param Vector<Vector<bool> > takes a reference to vector of codes that are correspond to the encoded symbols
 * @return Node* a pointer to the root of the tree
 */
Node* buildTree(Vector <char> & symbols, Vector<Vector<bool> > &codes);


/*
 * Decodes a sourse text and writes encoded text into a text file
 * @param int takes a number of all characters in the sourse text
 * @param ofstream takes a reference to the stream for writing file
 * @param ifbitstream takes a reference to the stream of open file for bit by bit reading
 * @param Node* takes a pointer to the root of the coding tree
 */
void decodingText(unsigned int numOfChars, ofstream & writeFile, ifbitstream & readArch, Node* );


/*
 * Return true if the current node is leaf, that means it has an actual value of character
 */
bool isLeaf(Node *);


/*
 * Bitwise reading of one byte information
 * @param ifbitstream takes a reference to the stream of open file for bit by bit reading
 * @return char a character read bitwise
 */
char readByte(ifbitstream & readArch);


/*
 * Bitwise reading of unsigned int
 * @param ifbitstream takes a reference to the stream of open file for bit by bit reading
 * @return unsigned int a number read bitwise
 */
unsigned int readInteger(ifbitstream & readArch);


/*
 * Prompts the user for a filename and tries to open file. If an unsuccessful attempt
 * to open a file again prompts the user for a filename. Otherwise returns a filename.
 */
string requestForFileName(ifbitstream &);


/*
 *  Prompts the user for a filename that he wants to dearchive and creates a new text file where saves encoded text.
 *  Text decoded using a table of characters with corresponding codes, which is at the beggining of the archive file.
 */
int main() {
    ifbitstream readArch; // create an object of stream class to open a file
    requestForFileName(readArch);

    // composes a name for text file by deleting "Arch" from the end of the filename
    string filename;
    cout << "Please, input a filename to to unzip: ";
    cin >> filename;
    cout << endl;
    ofstream writeFile(filename, ios::binary);

    Vector <char> symbols; // declare a Vector for saving encoded characters
    Vector<Vector<bool> >codes; // declare a Vector for saving codes that are correspond to the encoded symbols

    unsigned int numOfChars = readTable(readArch, symbols, codes);

    //close a file and complete the program if source file was empty
    if(numOfChars == 0){
        writeFile.close();
        return 0;
    }

    Node* rootPtr = buildTree(symbols, codes);
    decodingText(numOfChars, writeFile, readArch, rootPtr);

    return 0;
}


string requestForFileName(ifbitstream & readArch){
    while (true){
        string fileName = "";
        cout << "Please, enter file name: " << endl;
        cin >> fileName;
        readArch.open(fileName);
        if(!readArch.fail()){
            return fileName;
        }
        readArch.clear(); // resets the status indecator in the stream for reuse it.
        cout << "The file can not be found. Check the correctness of the file name." << endl;
    }
}


unsigned int readTable(ifbitstream & readArch, Vector <char> & symbols, Vector<Vector<bool> > &codes){
    unsigned int numChars = readInteger(readArch); // read a number of all symbols of the sourse file

    if (numChars == 0){
        return numChars;
    }

    int numCodes = readByte(readArch); // read a number of coded symbols of the sourse file

    if (numCodes == 0){
        numCodes = 256;
    }


    for(int i = 0; i < numCodes; ++i){ // read a code table
        char ch = readByte(readArch);
        symbols.add(ch);
        Vector<bool>code;
        int codeSize = readByte(readArch);
        for(int i = 0; i < codeSize; ++i){
            int bit = readArch.readBit();
            code.add(bit);
        }
        codes.add(code);
    }
    return numChars;
}


char readByte(ifbitstream & readArch){
    char ch = 0;
    //use bitwice addition and bit shift current bit to obtain the correct value of character
    for(int i = 7; i >= 0; --i){
        int bit = readArch.readBit();
        ch = ch | bit << i;
    }
    return ch;
}

unsigned int readInteger(ifbitstream & readArch){
    unsigned int num = 0;
    //use bitwice addition and bit shift current bit to obtain the correct value of integer
    for(int i = (sizeof (int) * 8 - 1); i >= 0; --i){
        int bit = readArch.readBit();
        num = num | bit << i;
    }
    return num;
}


Node* buildTree(Vector <char> & symbols, Vector<Vector<bool> > & codes){
    Node * root = new Node();
    Node * current = root;
    for(int i = 0; i < codes.size(); ++i){ // for all codes
        for(int j = 0; j < codes[i].size(); ++j){ // for all characters in code
            if(codes[i][j]){                      // if character is equal 1
                if(current->right == NULL){       // if node has not a right child
                    Node * n = new Node();        // create a new node
                    current->right = n;
                }
                current = current->right;

            }
            else{
                if(current->left == NULL){        // if node has not a left child
                    Node * n = new Node();        // create a new node
                    current->left = n;
                }
                current = current->left;
            }
        }
        current->value = symbols[i];              // for the last node writes the value of a character
        current = root;
    }
    return root;
}



void decodingText(unsigned int numOfChars, ofstream & writeFile, ifbitstream & readArch, Node* root){

    Node * current = root;

    while (numOfChars > 0){
        int bit = readArch.readBit();

        if(bit == 1){
            current = current->right;
        }
        else {
            current = current->left;
        }
        if(isLeaf(current)){
            writeFile.put(current->value);
            current = root;
            --numOfChars;
        }

    }
    readArch.close();
    writeFile.close();
}


bool isLeaf(Node * n){
    return ((n->left == NULL) && (n->right == NULL));
}
