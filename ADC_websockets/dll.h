#include <stdio.h>
#include <stdlib.h>

struct Node  {
	float data;
	struct Node* next;
	struct Node* prev;
	struct Node* last;
	unsigned int count;
};

// all 12 of them
int linkedListCount; 

struct Node* GetNewNode(float x);
void InsertAtHead(float x, int node_index);
void InsertAtTail(float x, int node_index);
void RemoveAtHead(int node_index);
void RemoveLinkedList(int node_index);
void RemoveEntireLinkedList();
void Print();
void ReversePrint();
int saveCSVfile();