#include "dll.h"

#ifndef DOUBLY_LINKED_LIST_FUNCS_DEFINED
#define DOUBLY_LINKED_LIST_FUNCS_DEFINED

extern int linkedListCount;
struct Node* head[12]; // global variable - pointer to head node.

//Creates a new Node and returns pointer to it. 
struct Node* GetNewNode(float x) {
	struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
	newNode->data = x;
	newNode->prev = NULL;
	newNode->next = NULL;
	newNode->last = NULL;
	return newNode;
}

//Inserts a Node at head of doubly linked list
void InsertAtHead(float x, int node_index) {
	struct Node* newNode = GetNewNode(x);

	// empty
	if(head[node_index] == NULL) {
		head[node_index] = newNode;
		head[node_index]->last = newNode;
		return;
	}

	// not empty
	head[node_index]->prev = newNode;
	newNode->next = head[node_index]; 
	head[node_index] = newNode;
}

//Inserts a Node at tail of Doubly linked list
void InsertAtTail(float x, int node_index) {
	struct Node* temp = head[node_index];
	struct Node* newNode = GetNewNode(x);
	if(head[node_index] == NULL) {
		head[node_index] = newNode;
		head[node_index]->last = newNode;
		return;
	}
	
	// slow O(n) traversal
	//while(temp->next != NULL) temp = temp->next; 
	
	// fast O(1) traversal
	// Go To last Node
	temp=temp->last;
	temp->next = newNode;
	newNode->prev = temp;
	head[node_index]->last = newNode;
}

// deletes one node at the head of the doubly linked list
void RemoveAtHead(int node_index){
	struct Node* temp = head[node_index];
	struct Node* temp_last;

	if(temp == NULL) return; // empty list, exit

	// check if there is exactly one item in the linked list
	if((temp->next == NULL)){
		// delete that one item. The first one.
		free(temp);

		// bring the head to an empty state
		head[node_index] = NULL;
		return;
	} else {
		// save the last node's address which is only available in the head node.
		temp_last = head[node_index]->last;

		// reassign head to the second node
		head[node_index]=head[node_index]->next;

		head[node_index]->last = temp_last;

		// delete the first node
		free(temp);
		return;
	}
}

// removes a linked list from head with index "node_index"
void RemoveLinkedList(int node_index){
	struct Node* temp;
	
	if(head[node_index] == NULL) return; // empty list, exit

	// deleting all items with O(n) speed
	do {
		temp = head[node_index];					// save head
		head[node_index]=head[node_index]->next;	// go to the next item
		free(temp);									// delete first item
	} while(head[node_index] != NULL);
}

void RemoveEntireLinkedList(){
	int i;
	for(i = 0; i < 12; i++){
		RemoveLinkedList(i);
	}
}

//Prints all the elements in linked list in forward traversal order
void Print(int node_index) {
	struct Node* temp = head[node_index];
	
	if(temp == NULL) return; // empty list, exit

	printf("Forward: ");
	while(temp != NULL) {
		printf("%f ",temp->data);
		temp = temp->next;
	}
	printf("\n");
}

//Prints all elements in linked list in reverse traversal order. 
void ReversePrint(int node_index) {
	struct Node* temp = head[node_index];
	if(temp == NULL) return; // empty list, exit
	
	// // slow O(n) traversal 
	/*
	// Going to last Node
	while(temp->next != NULL) {
		temp = temp->next;
	}
	*/

	// fast O(1) traversal
	temp = temp->last;

	// Traversing backward using prev pointer
	printf("Reverse: ");
	while(temp != NULL) {
		printf("%f ",temp->data);
		temp = temp->prev;
	}
	printf("\n");
}

int saveCSVfile(){
	float values[12];
	struct Node* temp[12];

	FILE *fp = NULL;

	fp=fopen("ECG.csv","a");

	if(fp==NULL){
		return -1;
		printf("Failed to open file for writing a CSV file...\n");
	}

	int i, j;

	for(j = 0; j < 12; j++){
		temp[j] = head[j];
	}

	
	for(i = 0; i <= 5000; i++) {
		for(j = 0; j < 12; j++){
			if(temp[j] != NULL){
				values[j] = temp[j]->data;
				temp[j]=temp[j]->next;
			} else
			{
				fclose(fp);
				return 1;
			}
			
		}

  		fprintf(fp, "%1.4f,"
					"%1.4f,"
					"%1.4f,"
					"%1.4f,"
					"%1.4f,"
					"%1.4f,"
					"%1.4f,"
					"%1.4f,"
					"%1.4f,"
					"%1.4f,"
					"%1.4f,"
					"%1.4f\n",
					values[0],
					values[1],
					values[2],
					values[3],
					values[4],
					values[5],
					values[6],
					values[7],
					values[8],
					values[9],
					values[10],
					values[12]);
	}

	return 1;
}

#endif