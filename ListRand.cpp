#include "ListNode.cpp"
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <stack>
using namespace std;

static const string JSON_COUNT_PROPERTY_NAME = "\"count\"";
static const string JSON_LIST_PROPERTY_NAME = "\"list\"";
static const string JSON_RAND_LIST_PROPERTY_NAME = "\"randomAssociate\"";
static const string JSON_RAND_PARENT_ID_NAME = "\"nodeInd\"";
static const string JSON_RAND_REFERENCE_ID_NAME = "\"randomInd\"";

class ListRand
{
    private:
        // Hash function for list
        // maybe can be simpler
        // i didn't test performance with different hash methods
        struct ListNodeHash {
            size_t operator()(const ListNode *node) const {
                size_t hash1 = hash<string>{}(node->data);
                size_t hash2(0), hash3(0), hash4(0);
                if (node->prev) hash2 =  hash<string>{}(node->prev->data);
                if (node->next) hash3 =  hash<string>{}(node->next->data);
                if (node->rand) hash4 = hash<string>{}(node->rand->data);
                return hash1 ^ (hash2 << 3) ^ (hash3 << 2) ^ (hash4 << 1);
            }
        };
        struct ListNodeEqual {
            bool operator()(const ListNode* node1, const ListNode* node2) const {
                return node1 == node2;
            }
        };
        typedef unordered_map<ListNode*, int, ListNodeHash, ListNodeEqual> dataToIndexMap;

    public:
        ListNode *head;
        ListNode *tail;
        int count;

    ListRand() : head(nullptr), tail(nullptr), count(0) {}
    ~ListRand() {
        while (count > 0) removeHead();
        
    }
    void removeHead() {
        if (count == 0) return;

        auto deleteNode = head;
        head = head->next;

        // this part of code deletes random reference to deleting node
        // may be source of poor performance
        auto ptr = head;
        while (ptr) {
            if (ptr->rand == deleteNode) ptr->rand = nullptr;
            ptr = ptr->next;
        }

        if (head) head->prev = nullptr;
        deleteNode->next = nullptr;
        deleteNode->rand = nullptr;
        
        if (count == 1) tail = nullptr;

        delete deleteNode;
        --count;
    }
    void removeTail() {
        if (count == 0) return;
        
        auto deleteNode = tail;
        tail = tail->prev;
        
        // see comment in removeHead() function
        auto ptr = head;
        while (ptr && ptr->rand != deleteNode) {
            if (ptr->rand = deleteNode) ptr->rand = nullptr;
            ptr = ptr->next;
        }

        if (tail) tail->next = nullptr;
        deleteNode->prev = nullptr;
        deleteNode->rand = nullptr;
        
        if (count == 1) head = nullptr;

        delete deleteNode;
        --count;
    }
    void addHead(ListNode *node) {
        if (!tail) {
            head = node;
            tail = head;
        } else {
            head->prev = node;
            node->next = head;
            head = node;
        }
        ++count;
    }
    void addTail(ListNode *node) {
        if (!head) {
            tail = node;
            head = tail;
        }
        else {
            tail->next = node;
            node->prev = tail;
            tail = node;
        }
        ++count;
    }
    void clear() {
        while (count > 0) removeTail();
    }
    // fill array of pointers to ListNode
    // note that input array is must be initialized and properly deallocated!!!
    void fillArrayOfPointers(ListNode** arr) {
        auto listPtr = head;
        int index = 0;
        while (listPtr) {
            arr[index] = listPtr;
            listPtr = listPtr->next;
            ++index;
        }
    }
    // custom serialize method
    void Serialize(ofstream &s) const
    {
        // array of pointers to ListNode
        // this is aray representation of list
        // to simplify accessing list nodes by index
        ListNode** arr = new ListNode*[count];
        // unordered_map with following KEY:VALUE pairs
        // KEY -- Pointer to ListNode, VALUE -- index of ListNode in array arr
        dataToIndexMap map = dataToIndexMap();

        // filling array and map with list nodes
        int cnt_tmp = 0;
        auto listPtr = head;
        while (listPtr) {
            arr[cnt_tmp] = listPtr;
            map[listPtr] = cnt_tmp;
            listPtr = listPtr->next;
            ++cnt_tmp;
        }

        // stack to keep track of brackets
        stack<char> bracketsStack;
        
        s.flush();
        
        // opening json object
        s<<"{"; bracketsStack.push('}');
        // adding count property
        s<<JSON_COUNT_PROPERTY_NAME<<": "<< count<<", ";
        // adding list property array of strings
        s<<JSON_LIST_PROPERTY_NAME<<": ["; bracketsStack.push(']');
        for (int i = 0; i < count; ++i) {
            if (i != 0) s<<", ";
            s<<"\""<<arr[i]->data<<"\"";
        }
        s << bracketsStack.top(); bracketsStack.pop();
        // adding random relations as array of objects with following structure
        // {nodeInd: $id, randomInd: $randId}
        s<<", "<<JSON_RAND_LIST_PROPERTY_NAME<<" :["; bracketsStack.push(']');
        for (int i = 0; i < count; ++i) {
            if (!arr[i]->rand) continue;
            if (i != 0) s<<", ";

            s<<"{"<<JSON_RAND_PARENT_ID_NAME<<":"<<i<<", "<<JSON_RAND_REFERENCE_ID_NAME<<":"<<map[arr[i]->rand]<<"}";
        }
        while (!bracketsStack.empty()) {
            s<<bracketsStack.top(); bracketsStack.pop();
        }
        s.close();
        delete[] arr;
    }

    // custom deserialize method
    void Deserialize(ifstream &s)
    {   
        // the number of characters in file
        int length = 0;
        string json = string();
        if (s.is_open()) {
            char* buffer = nullptr;
            s.seekg (0, s.end);
            length = s.tellg();
            s.seekg (0, s.beg);
            
            // there can be error during opening file
            if (length < 0) return;

            buffer = new char[length];
            s.read(buffer, length);
            
            json = string(buffer);
            
            delete[] buffer;
        } else {
            cout << "Error occured while opening the specified file" << endl;
            return;
        }

        // array of pointers to ListNode
        ListNode** arr = nullptr;
        
        // stack to keep track of brackets
        // INFO: we can enhance usage of this stack to verify json structure integrity
        // for now, program can accept invalid jsons
        stack<char> bracketsStack = stack<char>();

        int nodesCount = 0;
        int ind = 0;
        // flag to detect parsed properties
        bool isCountParsed(0), isListParsed(0), isRelationsParsed(0);
        while (ind < length) {
            string bufferStr = "";

            if (ind == 0) {
                while (json[ind] != '{' && ind < length) ++ind;

                if (json[ind] == '{') {
                    bracketsStack.push('}');
                    ++ind;
                    continue;
                }
            }
            
            if (!isCountParsed) {
                // find count property keyword
                ind = json.find(JSON_COUNT_PROPERTY_NAME, ind);

                if ((size_t)ind == json.npos) break;
                
                ind += JSON_COUNT_PROPERTY_NAME.length();
                
                if (ind >= length) break;
                // skil colon
                const int COUNT_NUMBER_START_INDEX = ++ind;
                
                int countEndInd = COUNT_NUMBER_START_INDEX;
                while (json[countEndInd] != ',') {
                    ++countEndInd;
                };
                bufferStr = json.substr(COUNT_NUMBER_START_INDEX, countEndInd - COUNT_NUMBER_START_INDEX);
                try {
                    nodesCount = stoi(bufferStr, nullptr);
                } catch (const invalid_argument &ex) {
                    cout << ex.what() << endl;
                    return;
                } catch (const out_of_range& ex) {
                    cout << ex.what() << endl;
                    return;
                }
                isCountParsed = true;
            }

            if (!isListParsed) {
                ind = json.find(JSON_LIST_PROPERTY_NAME, ind);
                
                if ((size_t)ind == json.npos) break;
                
                ind += JSON_LIST_PROPERTY_NAME.length();

                while (json[ind] != '[' && ind < length) ++ind; 

                if (json[ind] == '[') bracketsStack.push(']');
                else break;
                
                // array of pointers to pointers to ListNode
                arr = new ListNode*[nodesCount];

                while (json[ind] != bracketsStack.top() && ind != length) {
                    // find qoute if there are spaces or other unecessary characters
                    // can be case when property is empty array without the qoutes
                    // can be case when file ends unexpectedly
                    while (json[ind] != '\"' && json[ind] != bracketsStack.top() && ind < length) ++ind;

                    if (json[ind] == bracketsStack.top()) break;
                    
                    // set char pointer to the next position of the opening qoute
                    ++ind;

                    const int DATA_START_INDEX = ind;
                    while (json[ind] != '\"' && ind < length) ++ind;
                    
                    if (ind >= length) break;
                    
                    int dataEndInd = ind;
                    // creates new node at the end of current list
                    this->addTail(new ListNode(json.substr(DATA_START_INDEX, dataEndInd - DATA_START_INDEX)));
                    // set array element at specified index pointer to the last element
                    arr[count-1] = this->tail;

                    while (json[ind] != ',' && json[ind] != bracketsStack.top() && ind < length) ++ind; // find comma
                }
                if (json[ind] == bracketsStack.top()) bracketsStack.pop();
                else
                    // corrupted json file detected
                    break;
                
                isListParsed = true;
                
                if (nodesCount != count)
                    // if there is difference between meta data about count and actual count value
                    break;
            }

            if (!isRelationsParsed) {
                ind = json.find(JSON_RAND_LIST_PROPERTY_NAME, ind);

                if ((size_t)ind == json.npos) break;

                ind +=  JSON_RAND_LIST_PROPERTY_NAME.length();

                while (json[ind] != '[' && ind < length) ++ind; 

                if (json[ind] == '[') bracketsStack.push(']');
                else break;

                while (json[ind] != ']' && ind < length) {
                    while (json[ind] != '{' && json[ind] != ']' && ind < length) ++ind;
                    
                    // empty array detection
                    if (json[ind] == ']') break;
                    
                    if (json[ind] == '{') bracketsStack.push('}');
                    else break;

                    // pointer to parent node which rand property should be changed
                    ListNode* parentNode = nullptr;
                    // flag to detec if relation object is parsed
                    bool isRelationParsed = false;

                    while (json[ind] != bracketsStack.top() && ind < length && !isRelationParsed) {
                        const string SEARCH_STRING = (!parentNode) ? JSON_RAND_PARENT_ID_NAME : JSON_RAND_REFERENCE_ID_NAME;
                        
                        ind = json.find(SEARCH_STRING, ind);

                        if ((size_t)ind == json.npos) break;

                        while (json[ind] != ':' && ind < length) ++ind;
                        
                        if (ind >= length) break;

                        // set char pointer after found colon
                        const int START_INDEX = ++ind;

                        while (json[ind] != ',' && json[ind] != '}' && ind < length) ++ind;   

                        if (ind >= length) break;
                        
                        bufferStr = json.substr(START_INDEX, ind - START_INDEX);

                        if (!parentNode) {
                            try {
                                const int parentNode_INDEX = stoi(bufferStr);
                                parentNode = arr[parentNode_INDEX];
                            } catch (const invalid_argument& ex) {
                                cout << "Error parsing relations property: " << ex.what() << endl;
                                break;
                            } catch (const out_of_range& ex) {
                                cout << "Error parsing relations property: " << ex.what() << endl;
                                break;
                            }
                        } else {
                            try {
                                const int REFERENCE_NODE_INDEX = stoi(bufferStr);
                                parentNode->rand = arr[REFERENCE_NODE_INDEX];
                            }
                            catch (const invalid_argument& ex) {
                                cout << "Error parsing relations property: " << ex.what() << endl;
                                break;
                            }
                            catch (const out_of_range& ex) {
                                cout << "Error parsing relations property: " << ex.what() << endl;
                                break;
                            }
                            isRelationParsed = true;
                        }
                    }
                    
                    while (json[ind] != '}' && ind < length) ++ind;

                    if (ind >= length) break;

                    if ((json[ind] == '}' && bracketsStack.top() == '}') || ind < length) { ++ind; bracketsStack.pop(); }
                    else break;
                }
                
                if (json[ind] == ']') { bracketsStack.pop(); isRelationsParsed = true; }
                
                if (isCountParsed && isListParsed && isRelationsParsed) {
                    while (json[ind] != '}' && ind < length) ++ind;
                    
                    if (ind >= length) break;
                    
                    if (json[ind] == '}' && bracketsStack.top() == '}') { ++ind; bracketsStack.pop(); }
                }
            }
        }
        if (!bracketsStack.empty()) cout << "Json file is corrupted: there are brackets incompetance!" << endl;
        if (count != nodesCount) cout<<"Warning while parsing: there is difference between meta cound and actual count of nodes!"<<endl;
        
        s.close();
        delete[] arr;
    }
};
