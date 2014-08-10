#ifndef LIST_H
#define LIST_H

#include <stddef.h>

template<typename T> 
class list_node
{
	public:
		T data; //Freely accessible by list
		list_node *next;
		list_node *prev;
		
		list_node() : prev(NULL), next(NULL) {}
		~list_node() {} 
		
};

template<typename T> 
class list
{
protected:
	list_node<T> *head;
	list_node<T> *current;
	size_t len;
	
public:
	list() : len(0) 
	{
		head = new list_node<T>();
		current = head;
	}
	~list() 
	{
		for(size_t i = 0; i < len-1; i++)
		{
			head = head->next;
			delete[] head->prev;
		}
		delete[] head;
	}

	void push_back(const T& value) 
	{
		current->data = value;
		list_node<T> *t;
		t = new list_node<T>();		
		current->next = t;
		t->prev = current;
		current = t;
		len++;
	}
	
	void del(list_node<T> *node) 
	{
		if(node == NULL || !len)
			return;
		if(node == head)
			head = node->next;
		if(node->next != NULL)
			node->next->prev = node->prev;
		if(node->prev != NULL)
			node->prev->next = node->next;			
		delete[] node;	
		len--;
		return;
	}
	
	size_t size() { return len; }

	list_node<T>* end() const { return (current==head)?current:current->prev; }
	list_node<T>* begin() const { return head; }

	typedef const list_node<T>* const_iterator;
	typedef list_node<T>* iterator;
};



#endif
