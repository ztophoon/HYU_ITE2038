#ifndef __BPT_H__
#define __BPT_H__

// Uncomment the line below if you are compiling on Windows.
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define LEAF_ORDER 4 //32
#define INTERNAL_ORDER 4 //249

#define PAGE_SIZE 4096
#define VALUE_SIZE 120
#define PAGENUM_OF_HEADER 0

/*
 * PAGE AND RECORD LAYOUTS.
 */

typedef struct header_page {
	uint64_t free_page_offset;
	uint64_t root_page_offset;
	uint64_t num_of_pages;
	char reserved[4072]; // For standardization
}header_page;

typedef struct free_page {
	uint64_t next_free_page_number;
	char reserved[4088]; // For standardization
}free_page;

typedef struct internal_record {
	uint64_t key;
	uint64_t page_number;
}internal_record;

typedef struct internal_page {
	uint64_t parent_page_number;
	uint32_t is_leaf;
	uint32_t number_of_keys;

	char reserved[104]; // For standardization

	uint64_t left_page_number;
	internal_record internal_record[248];
}internal_page;

typedef struct record {
	uint64_t key;
	char value[VALUE_SIZE];
}record;

typedef struct leaf_page {
	uint64_t parent_page_number;
	uint32_t is_leaf;
	uint32_t number_of_keys;

	char reserved[104]; // For standardization

	uint64_t right_sibling_page_number;
	record record[31];
}leaf_page;

/*
 * FILE MANAGER API.
 */

typedef uint64_t pagenum_t;
pagenum_t pagenum_to_offset(pagenum_t pagenum);
pagenum_t offset_to_pagenum(pagenum_t offset);

// In - memory page structure.
typedef struct page {
	pagenum_t pagenum;

	// Header page or Free page
	pagenum_t free_page_offset;
	pagenum_t root_page_offset;
	uint64_t number_of_pages;

	// Internal page or Leaf page
	uint64_t parent_page_number;
	int32_t is_leaf;

	uint32_t number_of_keys;
	union {
		uint64_t left_page_number; // Case: Internal page
		uint64_t right_sibling_page_number; // Case: Leaf page
	};
	union {
		internal_record internal_record[248]; // Case: Internal page
		record record[31]; // Case: Leaf page
	};
}page_t;

typedef struct temp_read_page {
	uint64_t parent_page_number;
	int32_t is_leaf;
	uint32_t number_of_keys;
	char reserved[104]; // For standardization
	union {
		uint64_t left_page_number; // Case: Internal page
		uint64_t right_sibling_page_number; // Case: Leaf page
	};
	union {
		internal_record internal_record[248]; // Case: Internal page
		record record[31]; // Case: Leaf page
	};
}temp_read_page_t;

pagenum_t file_alloc_page();
void file_free_page(pagenum_t pagenum);
void file_read_page(pagenum_t pagenum, page_t * dest);
void file_write_page(pagenum_t pagenum, const page_t * src);

/*
 * FUNCTION PROTOTYPES.
 */

 // Output and utility.
void usage(int flag);
pagenum_t find_leaf(int64_t key);
int db_find(int64_t key, char* ret_val);
int cut(int length);

/*
void enqueue(node * new_node);
node * dequeue(void);
int height(node * root);
int path_to_root(node * root, node * child);
void print_leaves(node * root);
void print_tree(node * root);
void find_and_print(node * root, int key, bool verbose);
void find_and_print_range(node * root, int range1, int range2, bool verbose);
int find_range(node * root, int key_start, int key_end, bool verbose, int returned_keys[], void * returned_pointers[]);
*/

// Insertion.
int db_insert(int64_t key, char* value);
void start_new_tree(record* new_record);
void insert_into_leaf(page_t* temp_page, record* new_record);
void insert_into_leaf_after_splitting(page_t* temp_page, record* new_record);
void insert_into_parent(page_t* left, int64_t key, page_t* right);
void insert_into_new_root(page_t* left, int64_t key, page_t* right);
void insert_into_node(page_t* node, int64_t key, page_t* right);
void insert_into_node_after_splitting(page_t* node, int64_t key, page_t* right);

// Deletion.
int db_delete(int64_t key);

/*
int get_neighbor_index(node * n);
node * adjust_root(node * root);
node * coalesce_nodes(node * root, node * n, node * neighbor, int neighbor_index, int k_prime);
node * redistribute_nodes(node * root, node * n, node * neighbor, int neighbor_index, int k_prime_index, int k_prime);
node * delete_entry(node * root, node * n, int key, void * pointer);
void destroy_tree_nodes(node * root);
node * destroy_tree(node * root);
*/

// 디버깅
typedef struct queue {
	uint64_t* arr;
	int f;
	int r;
}queue;
void enqueue(uint64_t offset, queue* q);
uint64_t dequeue(queue* q);
void print_tree();

#endif /* __BPT_H__*/
