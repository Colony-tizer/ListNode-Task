#include <string>

class ListNode
{
    public:
        ListNode() : data(""), prev(nullptr), next(nullptr), rand(nullptr) {}
        ListNode(const std::string data, ListNode* prev = nullptr, ListNode* next = nullptr, ListNode* rand = nullptr) : data(data), prev(prev), next(next), rand(rand) {}
        ListNode(ListNode& copy) : data(copy.data), prev(copy.prev), next(copy.next), rand(copy.rand) {}
        ListNode& operator=(ListNode const& node) {
            prev = node.prev; 
            next = node.next;
            rand = node.rand;
            data = node.data;
            return *this;
        }
        ~ListNode() {}
    public:
        ListNode* prev; // previous list element
        ListNode* next; // next list element
        ListNode* rand; // random element in the list
        std::string data; // data of the element
};