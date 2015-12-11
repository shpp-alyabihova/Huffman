#include <fstream>
#include <iostream>
#include <string>
#include "console.h"
#include "strlib.h"
#include "vector.h"

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
 * @param ifstream takes a reference to the stream of open file for reading as binary
 * @param Vector <char> takes a reference to vector for saving encoded symbols
 * @param Vector<Vector<bool> > takes a reference to vector for saving codes of encoded symbols
 * @return int a number of symbols in sourse file
 */
unsigned int readTable(ifstream & readArch, Vector <char> & symbols, Vector<Vector<bool> > &codes);


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
 * @param ifstream takes a reference to the stream of open file
 * @param Node* takes a pointer to the root of the coding tree
 */
void decodingText(unsigned int numOfChars, ofstream & writeFile, ifstream & readArch, Node* );


/*
 * Return true if the current node is leaf, that means it has an actual value of character
 */
bool isLeaf(Node *);

/*
 * Reads the calculated number of bytes that corresponds to the number of characters in the code divided by 8
 * @param ifstream takes a reference to the stream of open file
 * @param int takes the number of code symbols
 * @return Vector <bool> the code of current charecter
 */
Vector <bool> readCode(ifstream & readArch, int codeSize);


/*
 * Prompts the user for a filename and tries to open file. If an unsuccessful attempt
 * to open a file again prompts the user for a filename. Otherwise returns a filename.
 */
string requestForFileName(ifstream &);


/*
 *  Prompts the user for a filename that he wants to dearchive and creates a new text file where saves encoded text.
 *  Text decoded using a table of characters with corresponding codes, which is at the beggining of the archive file.
 */
int main() {
    ifstream readArch; // create an object of stream class to open a file
    requestForFileName(readArch);

    // requests the file name to save the encoded file content
    string filename;
    cout << "Please, input a filename to to unzip: ";
    cin >> filename;
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


string requestForFileName(ifstream & readArch){
    while (true){
        string fileName = "";
        cout << "Please, enter file name: " << endl;
        cin >> fileName;
        readArch.open(fileName, ios::binary);
        if(!readArch.fail()){
            return fileName;
        }
        readArch.clear(); // resets the status indecator in the stream for reuse it.
        cout << "The file can not be found. Check the correctness of the file name." << endl;
    }
}


unsigned int readTable(ifstream & readArch, Vector <char> & symbols, Vector<Vector<bool> > &codes){
    unsigned int numChars;
    readArch.read((char *)&numChars, sizeof(numChars));
    if (numChars == 0){
        return numChars;
    }
    char ch = 0;
    readArch.get(ch); // read a number of coded symbols of the sourse file
    int numCodes = ch;

    if (numCodes == 0){
        numCodes = 256;
    }

    for(int i = 0; i < numCodes; ++i){ // read a code table
        readArch.get(ch);
        symbols.add(ch);
        readArch.get(ch);
        int codeSize = ch;
        codes.add(readCode(readArch, codeSize));
    }
    return numChars;
}

Vector <bool> readCode(ifstream & readArch, int codeSize){
    Vector<bool> code;
    int size = (codeSize / 8 ) + ((codeSize % 8) ? 1 : 0); // size of buffer for reading bytes
    char * buff = new char [size];
    readArch.read(buff, size);
    for (int i = 0; i < codeSize; ++i){
        code.add(buff[i/8] < 0); // check most significant bit that is responsible for the sign of char
        buff[i/8] = buff[i/8] << 1;
    }
    delete [] buff;
    return code;
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



void decodingText(unsigned int numOfChars, ofstream & writeFile, ifstream & readArch, Node* root){

    Node * current = root;
    char ch;

    while (readArch.get(ch)){
        //readArch.get(ch);
        for (int i = 7; i >= 0; --i){
            if(ch & (1 << i))
                current = current->right;
            else
                current = current->left;
            if(isLeaf(current)){
                writeFile.put(current->value);
                current = root;
                --numOfChars;
            }
            if (numOfChars == 0){
                break;
            }
        }
    }
    readArch.close();
    writeFile.close();
}


bool isLeaf(Node * n){
    return ((n->left == NULL) && (n->right == NULL));
}
