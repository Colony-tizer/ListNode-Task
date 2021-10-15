#include <string>
#include <iostream>
#include <fstream>
#include <time.h>
#include <chrono>
#include "ListRand.cpp"

using namespace std;

const string PATH = "./";
const string SERIALISATION_FILENAME = PATH + "test.json";
const int VERY_BIG_LIST_LENGTH = 1000000;

// utility function to print the content of list
void printList(ListRand &list) {
    int temp_cnt = 0;
    ListNode *ptr = list.head;
    while (ptr != nullptr)
    {
        cout<<temp_cnt<<". "<<ptr->data;
        if (ptr->rand)
            cout<<" . rand "<<ptr->rand->data<<endl;
        else cout<<endl;
        temp_cnt++; ptr = ptr->next;
    }
    cout<<endl;
}
// Test (de)serialization of empty list
bool runEmptyTest(ListRand &testList, ListRand& verificationList, ofstream &outFileStream, ifstream &inFileStream) {
    bool result = true;

    auto startTime = chrono::high_resolution_clock::now();
    
    outFileStream.open(SERIALISATION_FILENAME, ios::out);
    testList.Serialize(outFileStream);
    
    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
    cout << "Serialization duration: " << duration.count() << endl;

    startTime = chrono::high_resolution_clock::now();

    inFileStream.open(SERIALISATION_FILENAME, ios::in);
    verificationList.Deserialize(inFileStream);

    endTime = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
    cout << "Deserialization duration: " << duration.count() << endl;

    cout << "Empty List Test: " << endl;
    string caseRes = "1. [SUCCESS] Correct nodes count";
    if (testList.count != verificationList.count) {
        caseRes = "1. [ERROR] Wrong nodes count after deserialization!";
        result = false;
    }
    cout << caseRes << endl;
    caseRes = "2. [SUCCESS] List is empty";
    if (verificationList.head || verificationList.tail) {
        caseRes = "2. [ERROR] Deserialized list is not empty!";
        result = false;
    }
    cout << caseRes << endl;
    return result;
}
// Test (de)serialization of list without rand references
bool runNoRandReferencesTest(ListRand& testList, ListRand& verificationList, ofstream& outFileStream, ifstream& inFileStream) {
    bool result = true;
    
    auto startTime = chrono::high_resolution_clock::now();

    outFileStream.open(SERIALISATION_FILENAME, ios::out);
    testList.Serialize(outFileStream);

    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
    cout << "Serialization duration: " << duration.count() << endl;

    startTime = chrono::high_resolution_clock::now();

    inFileStream.open(SERIALISATION_FILENAME, ios::in);
    verificationList.Deserialize(inFileStream);

    endTime = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
    cout << "Deserialization duration: " << duration.count() << endl;

    string caseRes = "1. [SUCCESS] Correct nodes count";
    if (testList.count != verificationList.count) {
        caseRes = "1. [ERROR] Wrong nodes count after deserialization!";
        result = false;
    }
    cout << caseRes << endl;
    caseRes = "2. [SUCCESS] List is not empty";
    if (!testList.head && !verificationList.head && !testList.tail && !verificationList.tail) {
        caseRes = "2. [ERROR] Deserialized list is empty!";
        result = false;
    }
    cout << caseRes << endl;
    
    caseRes = "3. [SUCCESS] List nodes data are identical";
    auto testListPtr = testList.head;
    auto verificationListPtr = verificationList.head;
    int count = 0;
    while (testListPtr && verificationListPtr && testListPtr->data == verificationListPtr->data) {
        testListPtr = testListPtr->next; 
        verificationListPtr = verificationListPtr->next;
        ++count;
    }
    if ((!testListPtr && verificationListPtr) || (testListPtr && !verificationListPtr))
        caseRes = "3. [ERROR] Lists length are different despite the meta info about count!";
    else if (testListPtr && verificationListPtr && testListPtr->data != verificationListPtr->data)
        caseRes = "3. [ERROR] List nodes data are not identical!";
    cout << caseRes << endl;

    return result;
}
// Test (de)serialization of list with rand references
bool runRandReferencesTest(ListRand& testList, ListRand& verificationList, ofstream& outFileStream, ifstream& inFileStream) {
    bool result = true;

    result &= runNoRandReferencesTest(testList, verificationList, outFileStream, inFileStream);

    string caseRes = "4. [SUCCESS] Random relations are identical";
    auto testListPtr = testList.head;
    auto verificationListPtr = verificationList.head;

    while (testListPtr && verificationListPtr && testListPtr->rand->data == verificationListPtr->rand->data) {
        testListPtr = testListPtr->next;
        verificationListPtr = verificationListPtr->next;
    }
    if (testListPtr && verificationListPtr && testListPtr->data != verificationListPtr->data)
        caseRes = "4. [ERROR] List nodes random references data are not identical!";
    cout << caseRes << endl;

    return result;
}
// Method to fill list with random values
// args: 
// list -- list to fill
// count -- number of generating nodes
// if count is not specfied then function randomly generates count
void fillListWithRandomValues(ListRand& list, const int count = -1) {
    const int MAX_NUM = 100;
    const int NODES_COUNT = (count < 0) ? rand() % MAX_NUM + MAX_NUM/10 : count;
    
    list.clear();
    auto hash_int = hash<int>();
    for (int i = 0; i < NODES_COUNT; ++i)
        list.addTail(new ListNode(to_string(hash_int(i))));
}
// Method to make random connections within list
// args: 
// list -- list to make connections
void makeRandomRelations(ListRand& list) {
    // array with pointers to ListNodes
    // it is needed to simplify accessing list nodes by index
    auto arr = new ListNode*[list.count];
    
    list.fillArrayOfPointers(arr);

    int randIndex = 0;
    for (int i = 0; i < list.count; ++i) {
        randIndex = rand() % list.count;
        arr[i]->rand = arr[randIndex];
    }
    delete[] arr;
}
// run all tests
bool runTests() {
    // set seed for randomizer
    srand(time(0));

    bool testsResult = true;
    ListRand testList; 
    ListRand verificationList;
    // test empty
    ofstream outFileStream;
    ifstream inFileStream;

    auto startTime = chrono::high_resolution_clock::now();
    
    testsResult &= runEmptyTest(testList, verificationList, outFileStream, inFileStream);
    
    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
    cout << "Test duration, ms: " << duration.count() << endl;
    verificationList.clear();
    // test no relations

    cout << "List with No Random References Test: " << endl;
    fillListWithRandomValues(testList);

    startTime = chrono::high_resolution_clock::now();

    testsResult &= runNoRandReferencesTest(testList, verificationList, outFileStream, inFileStream);

    endTime = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
    cout << "Test duration, ms: " << duration.count() << endl;

    verificationList.clear();
    
    // test complex relations
    cout << "List with Random References Test: " << endl;
    fillListWithRandomValues(testList);
    makeRandomRelations(testList);

    startTime = chrono::high_resolution_clock::now();

    testsResult &= runRandReferencesTest(testList, verificationList, outFileStream, inFileStream);

    endTime = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
    cout << "Test duration, ms: " << duration.count() << endl;

    verificationList.clear();
    // test very big list 
    cout << "Running tests with big list length = "<< VERY_BIG_LIST_LENGTH<<endl;
    
    fillListWithRandomValues(testList, VERY_BIG_LIST_LENGTH);
    makeRandomRelations(testList);

    startTime = chrono::high_resolution_clock::now();

    testsResult &= runRandReferencesTest(testList, verificationList, outFileStream, inFileStream);
    
    endTime = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
    cout << "Test duration, ms: " << duration.count() << endl;

    return testsResult;
}

int main(int argc, char** argv) {
    bool result = runTests();

    if (result) cout << "TESTS HAVE BEEN PASSED SUCCESSFULLY"<<endl;
    else cout << "SOME OR ALL TESTS HAVE BEEN FAILED" << endl;
    return 0;
}