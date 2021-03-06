#include "bpt.h"

/*
 * GLOBAL.
 */

FILE* fp = NULL;
page_t* HD;
int num_of_opened_tablepage = 0;

bool verbose = 0;

void usage(int flag) {
	printf("Enter any of the following commands after the prompt > :\n"
		//"\to <k>     -- Open existing data file using pathname <k> (an string) or create one.\n"
		"\ti <k> <v> -- Insert <k>(integer) as key and <v>(string) as value).\n"
		"\tf <k>     -- Find and prtnt the value under key <k>.\n"
		"\td <k>     -- Delete key <k> and its associated value.\n"
		"\tq         -- Quit. (Or use Ctrl+d)\n");
	if (!flag) printf("> ");
	return;
}

pagenum_t pagenum_to_offset(pagenum_t pagenum) {
	return pagenum * PAGE_SIZE;
}
pagenum_t offset_to_pagenum(pagenum_t offset) {
	return offset / PAGE_SIZE;
}

/*
 * FILE MANAGER API.
 */

pagenum_t file_alloc_page() {
	// Allocate header page and get metadata.
	/*
	page_t* HD;
	HD = (page_t*)malloc(sizeof(page_t));
	if (HD == NULL) {
		perror("Header creation @file_alloc_page.");
	}
	*/
	file_read_page(PAGENUM_OF_HEADER, HD);

	if (verbose) {
		printf("<file_alloc_page>\n");
		printf("free_page_offset: %lu\n", HD->free_page_offset);
		printf("num_of_pages: %lu\n", HD->number_of_pages);
		printf("root_page_offset: %lu\n", HD->root_page_offset);
	}

	// Case: There is free page for allocation.
	if (HD->free_page_offset != 0) {
		pagenum_t ret_pagenum = offset_to_pagenum(HD->free_page_offset);

		free_page_t temp_free_page;
		fseek(fp, HD->free_page_offset, SEEK_SET);
		fread(&temp_free_page, PAGE_SIZE, 1, fp);
		HD->free_page_offset = temp_free_page.next_free_page_number;
		file_write_page(PAGENUM_OF_HEADER, HD);

		//free(HD);
		return ret_pagenum;
	}

	// Case: There is NO free page for allocation.
	else {
		HD->free_page_offset = HD->number_of_pages * PAGE_SIZE;
		pagenum_t ret_pagenum = offset_to_pagenum(HD->free_page_offset);

		fseek(fp, HD->free_page_offset, SEEK_SET);
		//printf(" [%d] ", HD->free_page_offset);

		free_page_t temp_free_page;
		for (int i = 1; i < 100; ++i) {
			temp_free_page.next_free_page_number = HD->free_page_offset + i * PAGE_SIZE;
			fwrite(&temp_free_page, PAGE_SIZE, 1, fp);
			fflush(fp);
		}

		// Creating 100th free page.
		temp_free_page.next_free_page_number = 0;
		fwrite(&temp_free_page, PAGE_SIZE, 1, fp);
		fflush(fp);

		HD->number_of_pages += 100;
		HD->free_page_offset += PAGE_SIZE;

		file_write_page(PAGENUM_OF_HEADER, HD);

		//free(HD);
		return ret_pagenum;
	}
}

void file_free_page(pagenum_t pagenum) {
	// Allocate header page and get metadata.
	/*
	page_t* HD;
	HD = (page_t*)malloc(sizeof(page_t));
	if (HD == NULL) {
		perror("Header creation @file_alloc_page.");
	}
	*/
	file_read_page(PAGENUM_OF_HEADER, HD);

	free_page_t temp_free_page;
	temp_free_page.next_free_page_number = offset_to_pagenum(HD->free_page_offset);
	HD->free_page_offset = pagenum_to_offset(pagenum);

	fseek(fp, pagenum_to_offset(pagenum), SEEK_SET);
	fwrite(&temp_free_page, PAGE_SIZE, 1, fp);

	fflush(fp);
	file_write_page(PAGENUM_OF_HEADER, HD);

	//free(HD);
	return;
}

void file_read_page(pagenum_t pagenum, page_t * dest) {

	fseek(fp, pagenum_to_offset(pagenum), SEEK_SET);

	// Case: Pagenum indicates header.
	if (pagenum == PAGENUM_OF_HEADER) {
		header_page_t temp_header;
		fread(&temp_header, PAGE_SIZE, 1, fp);
		dest->free_page_offset = temp_header.free_page_offset;
		dest->root_page_offset = temp_header.root_page_offset;
		dest->number_of_pages = temp_header.num_of_pages;
		return;
	}

	temp_read_page_t temp_page;
	fread(&temp_page, PAGE_SIZE, 1, fp);

	dest->pagenum = pagenum;
	dest->parent_page_number = temp_page.parent_page_number;
	dest->is_leaf = temp_page.is_leaf;
	dest->number_of_keys = temp_page.number_of_keys;

	// Case: Pagenum indicates Internal.
	if (temp_page.is_leaf == 0) {
		dest->left_page_number = temp_page.left_page_number;
		for (int i = 0; i < temp_page.number_of_keys; ++i)
			memcpy(&dest->internal_record[i], &temp_page.internal_record[i], sizeof(internal_record_t));
		return;
	}

	// Case: Pagenum indicates Leaf.
	if (temp_page.is_leaf == 1) {
		dest->right_sibling_page_number = temp_page.right_sibling_page_number;
		for (int i = 0; i < temp_page.number_of_keys; ++i)
			memcpy(&dest->record[i], &temp_page.record[i], sizeof(record_t));
		return;
	}
}

void file_write_page(pagenum_t pagenum, const page_t * src) {

	fseek(fp, pagenum_to_offset(pagenum), SEEK_SET);

	// Case: Pagenum indicates header.
	if (pagenum == PAGENUM_OF_HEADER) {
		header_page_t temp_header;
		temp_header.free_page_offset = src->free_page_offset;
		temp_header.root_page_offset = src->root_page_offset;
		temp_header.num_of_pages = src->number_of_pages;

		fwrite(&temp_header, PAGE_SIZE, 1, fp);
		fflush(fp); // O_sync @Linux
		return;
	}

	// Case: Pagenum indicates Internal.
	if (src->is_leaf == 0) {
		internal_page_t temp_internal;
		temp_internal.parent_page_number = src->parent_page_number;
		temp_internal.is_leaf = src->is_leaf;
		temp_internal.number_of_keys = src->number_of_keys;
		temp_internal.left_page_number = src->left_page_number;
		for (int i = 0; i < temp_internal.number_of_keys; ++i)
			memcpy(&temp_internal.internal_record[i], &src->internal_record[i], sizeof(internal_record_t));

		fwrite(&temp_internal, PAGE_SIZE, 1, fp);
		fflush(fp); // O_sync @Linux
		return;
	}

	// Case: Pagenum indicates Leaf.
	else {
		leaf_page_t temp_leaf;
		temp_leaf.parent_page_number = src->parent_page_number;
		temp_leaf.is_leaf = src->is_leaf;
		temp_leaf.number_of_keys = src->number_of_keys;
		temp_leaf.right_sibling_page_number = src->right_sibling_page_number;
		for (int i = 0; i < temp_leaf.number_of_keys; ++i)
			memcpy(&temp_leaf.record[i], &src->record[i], sizeof(record_t));

		fwrite(&temp_leaf, PAGE_SIZE, 1, fp);
		fflush(fp);

		return;
	}
}

int open_table(char* pathname) {
	// Assign unique table ID for each table.
	int unique_table_id = ++num_of_opened_tablepage;

	// Case: There is a file in the path.
	// Open file in read & write mode with binary format.
	if ((fp = fopen(pathname, "r+b")) != NULL) {
		//page_t* HD;
		HD = (page_t*)malloc(sizeof(page_t));
		if (HD == NULL) {
			perror("Header creation @file_alloc_page.");
		}
		file_read_page(PAGENUM_OF_HEADER, HD);

		//free(HD);
		return unique_table_id;
	}

	// Case: There is a NO file in the path.
	// Open file in read & write mode with binary format.
	else {
		fp = fopen(pathname, "w+b");
		if (fp == NULL) {
			perror("File open @open_table.");
			exit(EXIT_FAILURE);
		}

		//page_t* HD;
		HD = (page_t*)malloc(sizeof(page_t));
		if (HD == NULL) {
			perror("Header creation @file_alloc_page.");
		}

		// Create new header page.
		HD->pagenum = 0;
		HD->free_page_offset = 0;
		HD->root_page_offset = 0;
		HD->number_of_pages = 1;

		file_write_page(PAGENUM_OF_HEADER, HD);
		file_alloc_page();

		//free(HD);
		return unique_table_id;
	}
}

int db_find(int64_t key, char* ret_val) {
	pagenum_t temp_pagenum = find_leaf(key);

	//("[%ld: %s] in db_find\n", key, ret_val);

	// Case: The tree is not created yet.
	if (temp_pagenum == PAGENUM_OF_HEADER) {
		return 2;
	}

	page_t temp_page;
	file_read_page(temp_pagenum, &temp_page);

	int idx;
	for (idx = 0; idx < temp_page.number_of_keys; ++idx)
		if (temp_page.record[idx].key == key)
			break;

	// Case: There is no such key in the tree.
	if (idx == temp_page.number_of_keys) {
		return 1;
	}

	// Case: Founded matching key. Return matched value.
	else {
		memcpy(ret_val, &temp_page.record[idx].value, sizeof(VALUE_SIZE));
		return 0;
	}
}

/*
 * INSERT.
 */

int cut(int length) {
	if (length % 2 == 0) return length / 2;
	else return length / 2 + 1;
}

void start_new_tree(record_t* new_record) {
	// Allocate header page and get metadata.
	/*
	page_t* HD;
	HD = (page_t*)malloc(sizeof(page_t));
	if (HD == NULL) {
		perror("Header creation @file_alloc_page.");
	}
	*/
	file_read_page(PAGENUM_OF_HEADER, HD);

	HD->root_page_offset = pagenum_to_offset(file_alloc_page());

	// printf(" [%d] ", HD->root_page_offset);

	page_t new_leaf;
	new_leaf.pagenum = offset_to_pagenum(HD->root_page_offset);
	new_leaf.parent_page_number = 0;
	new_leaf.is_leaf = 1;
	new_leaf.number_of_keys = 1; // Key is inserted in the next next line.
	new_leaf.right_sibling_page_number = 0;
	memcpy(&new_leaf.record[0], new_record, sizeof(record_t));

	if (verbose) {
		printf("<start_new_tree>\n");
		printf("pagenum: %lu\n", new_leaf.pagenum);
	}

	file_write_page(PAGENUM_OF_HEADER, HD);
	file_write_page(new_leaf.pagenum, &new_leaf);

	//free(HD);
	return;
}

pagenum_t find_leaf(int64_t key) {
	// Allocate header page and get metadata.
	/*
	page_t* HD;
	HD = (page_t*)malloc(sizeof(page_t));
	if (HD == NULL) {
		perror("Header creation @file_alloc_page.");
	}
	*/
	file_read_page(PAGENUM_OF_HEADER, HD);

	if (verbose) {
		printf("<find_leaf>\n");
		printf("free_page_offset: %lu\n", HD->free_page_offset);
		printf("num_of_pages: %lu\n", HD->number_of_pages);
		printf("root_page_offset: %lu\n", HD->root_page_offset);
	}

	pagenum_t temp_pagenum = offset_to_pagenum(HD->root_page_offset);
	//free(HD);

	// Case: Pagenum indicates header.
	if (temp_pagenum == 0) {
		return temp_pagenum;
	}

	// Case: Pagenum indicates non-header.
	page_t temp_page;
	file_read_page(temp_pagenum, &temp_page);

	while (!temp_page.is_leaf) {
		int idx = -1;
		while (temp_page.internal_record[idx + 1].key <= key
			&& idx + 1 < temp_page.number_of_keys) {
			idx++;
		}
		if (idx == -1) temp_pagenum = temp_page.left_page_number;
		else temp_pagenum = temp_page.internal_record[idx].page_number;

		file_read_page(temp_pagenum, &temp_page);
	}

	return temp_pagenum;
}

void insert_into_leaf(page_t* leaf_page, record_t* new_record) {
	// Find insertion point.
	int insertion_point = 0;
	while (insertion_point < leaf_page->number_of_keys
		&& leaf_page->record[insertion_point].key < new_record->key)
		insertion_point++;

	// Make empty space for insert given record.
	for (int i = leaf_page->number_of_keys; i > insertion_point; --i) {
		memcpy(&leaf_page->record[i], &leaf_page->record[i - 1], sizeof(record_t));
	}
	memcpy(&leaf_page->record[insertion_point], new_record, sizeof(record_t));
	leaf_page->number_of_keys++;

	// Insert the new page which includes given record.
	file_write_page(leaf_page->pagenum, leaf_page);
}

void insert_into_leaf_after_splitting(page_t* leaf_page, record_t* new_record) {
	// Create new leaf for insertion.
	page_t new_leaf;
	new_leaf.pagenum = file_alloc_page();
	new_leaf.parent_page_number = leaf_page->parent_page_number;
	new_leaf.is_leaf = 1;
	new_leaf.number_of_keys = 0;
	new_leaf.right_sibling_page_number = leaf_page->right_sibling_page_number;
	leaf_page->right_sibling_page_number = new_leaf.pagenum;

	// Find insertion point.
	int insertion_point = 0;
	while (insertion_point < leaf_page->number_of_keys
		&& leaf_page->record[insertion_point].key < new_record->key)
		insertion_point++;

	// Create temp record for saving all exisiting data.
	record_t* temp_record;
	temp_record = (record_t*)malloc(sizeof(record_t) * LEAF_ORDER);
	if (temp_record == NULL) {
		perror("Record creation @insert_into_leaf_after_splitting.");
		exit(EXIT_FAILURE);
	}

	// Back up exisiting data before insertion.
	for (int i = 0, j = 0; i < leaf_page->number_of_keys; ++i, ++j) {
		if (j == insertion_point) j++; // Avoid insertion point.
		memcpy(&temp_record[j], &leaf_page->record[i], sizeof(record_t));
	}
	// Inset new record in the insertion point.
	memcpy(&temp_record[insertion_point], new_record, sizeof(record_t));

	// Initialize the key counter.
	leaf_page->number_of_keys = 0;

	// Make spilt point.
	int split_point = cut(LEAF_ORDER - 1);

	// Copy the data divided by split point.
	for (int i = 0; i < split_point; ++i) {
		memcpy(&leaf_page->record[i], &temp_record[i], sizeof(record_t));
		leaf_page->number_of_keys++;
	}
	for (int i = split_point, j = 0; i < LEAF_ORDER; ++i, ++j) {
		memcpy(&new_leaf.record[j], &temp_record[i], sizeof(record_t));
		new_leaf.number_of_keys++;
	}

	file_write_page(leaf_page->pagenum, leaf_page);
	file_write_page(new_leaf.pagenum, &new_leaf);

	insert_into_parent(leaf_page, new_leaf.record[0].key, &new_leaf);
	free(temp_record);
	return;
}

void insert_into_parent(page_t* left, int64_t key, page_t* right) {
	// Case: new root.
	if (left->parent_page_number == 0) {
		return insert_into_new_root(left, key, right);
	}

	// Case: leaf or node. (Remainder of function body.)
	page_t temp_page;
	file_read_page(left->parent_page_number, &temp_page);

	// Simple case: the new key fits into the node.
	if (temp_page.number_of_keys < INTERNAL_ORDER - 1)
		insert_into_node(&temp_page, key, right);

	// Harder case:  split a node in order to preserve the B+ tree properties.
	else insert_into_node_after_splitting(&temp_page, key, right);

	return;
}

void insert_into_new_root(page_t* left, int64_t key, page_t* right) {
	// Allocate header page and get metadata.
	/*
	page_t* HD;
	HD = (page_t*)malloc(sizeof(page_t));
	if (HD == NULL) {
		perror("Header creation @file_alloc_page.");
	}
	*/
	file_read_page(PAGENUM_OF_HEADER, HD);

	if (verbose) {
		printf("<insert_into_new_root>\n");
		printf("free_page_offset: %lu\n", HD->free_page_offset);
		printf("num_of_pages: %lu\n", HD->number_of_pages);
		printf("root_page_offset: %lu\n", HD->root_page_offset);
	}

	// Allocate temporary in-memory page
	page_t* temp_page;
	temp_page = (page_t*)malloc(sizeof(page_t)); // Internal page
	if (temp_page == NULL) {
		perror("Page creation @insert_into_new_root.");
		exit(EXIT_FAILURE);
	}

	// Initial settings.
	temp_page->pagenum = file_alloc_page();
	temp_page->parent_page_number = 0;
	temp_page->is_leaf = 0;
	temp_page->number_of_keys = 1; // Key is inserted in the next next line.
	temp_page->left_page_number = left->pagenum;
	temp_page->internal_record[0].key = key;
	temp_page->internal_record[0].page_number = right->pagenum;

	// Update parent number of left and right.
	left->parent_page_number = temp_page->pagenum;
	right->parent_page_number = temp_page->pagenum;

	// Update the metadata about root page in header page.
	HD->root_page_offset = pagenum_to_offset(temp_page->pagenum);

	file_write_page(left->pagenum, left);
	file_write_page(right->pagenum, right);
	file_write_page(temp_page->pagenum, temp_page);
	file_write_page(PAGENUM_OF_HEADER, HD);

	return;
}

void insert_into_node(page_t* node_page, int64_t key, page_t* right) {
	// Find point where node is inserted in.
	int insertion_point = 0;
	while (insertion_point < node_page->number_of_keys
		&& node_page->internal_record[insertion_point].key < key)
		insertion_point++;

	// Copy records after the insertion point in the exisiting tree.
	for (int i = node_page->number_of_keys; i > insertion_point; --i)
		memcpy(&node_page->internal_record[i], &node_page->internal_record[i - 1], sizeof(internal_record_t));

	// Put new internal record in the insertion point.
	node_page->internal_record[insertion_point].key = key;
	node_page->internal_record[insertion_point].page_number = right->pagenum;
	node_page->number_of_keys++;

	// Update the node to the file.
	file_write_page(node_page->pagenum, node_page);

	return;
}

void insert_into_node_after_splitting(page_t* node_page, int64_t key, page_t* right) {
	// Create temporary internal record containing given key and page number.
	internal_record_t temp_internal_record;
	temp_internal_record.key = key;
	temp_internal_record.page_number = node_page->pagenum;

	// Allocate back-up internal record to keep every data in.
	internal_record_t* backup_internal_record = (internal_record_t*)malloc(sizeof(internal_record_t) * INTERNAL_ORDER);
	if (backup_internal_record == NULL) {
		perror("Record creation @insert_into_node_after_splitting.");
		exit(EXIT_FAILURE);
	}

	// Find insertion point.
	int insertion_point = 0;
	while (insertion_point < INTERNAL_ORDER - 1
		&& node_page->internal_record[insertion_point].key < key)
		insertion_point++;

	// Push-back what's remaining and put the temporary internal record in the insertion point.
	for (int i = 0, j = 0; i < INTERNAL_ORDER - 1; ++i, ++j) {
		if (j == insertion_point) j++;
		memcpy(&backup_internal_record[j], &node_page->internal_record[i], sizeof(internal_record_t));
	}
	memcpy(&backup_internal_record[insertion_point], &temp_internal_record, sizeof(internal_record_t));

	// Create the new page and copy half records to the old and half to the new.
	// Make split point
	int split_point = cut(INTERNAL_ORDER);

	// Create new temporary page to insert in.
	page_t temp_page;
	temp_page.pagenum = file_alloc_page();
	temp_page.parent_page_number = node_page->parent_page_number;
	temp_page.is_leaf = 0;
	temp_page.number_of_keys = 0;
	temp_page.left_page_number = backup_internal_record[split_point - 1].page_number;

	// Clear node's key counter for re-filling node's key.
	node_page->number_of_keys = 0;

	// Copy records to node from back-up record BEFORE the split point.
	for (int i = 0; i < split_point - 1; ++i) {
		memcpy(&node_page->internal_record[i], &backup_internal_record[i], sizeof(internal_record_t));
		node_page->number_of_keys++;
	}
	// Set the k_prime number which is going to upper level by splitting.
	int64_t k_prime = backup_internal_record[split_point - 1].key;
	// Copy records to temporary page freom back-up just AFTER the split point.
	for (int i = split_point, j = 0; i < INTERNAL_ORDER; ++i, ++j) {
		memcpy(&temp_page.internal_record[j], &backup_internal_record[i], sizeof(internal_record_t));
		temp_page.number_of_keys++;
	}
	file_write_page(node_page->pagenum, node_page);
	file_write_page(temp_page.pagenum, &temp_page);

	// Change the children's parent page number into parent page number of temporary page.
	page_t child;
	file_read_page(temp_page.left_page_number, &child);
	child.parent_page_number = temp_page.pagenum;
	file_write_page(temp_page.left_page_number, &child);
	for (int i = 0; i < temp_page.number_of_keys; ++i) {
		file_read_page(temp_page.internal_record[i].page_number, &child);
		child.parent_page_number = temp_page.parent_page_number;
		file_write_page(temp_page.internal_record[i].page_number, &child);
	}

	free(backup_internal_record);

	// Insert a new key into the parent of the two pages resulting from the split,
	// with the old page to the left and the new to the right.
	return insert_into_parent(node_page, k_prime, &temp_page);
}

int db_insert(int64_t key, char* value) {
	// The current implementation ignores duplicates.
	int findng_result = db_find(key, value);
	if (findng_result == 0) {
		return -1;
	}

	//printf("[%ld: %s] in db_insert\n", key, value);

	// Allocate header page and get metadata.
	/*
	page_t* HD;
	HD = (page_t*)malloc(sizeof(page_t));
	if (HD == NULL) {
		perror("Header creation @file_alloc_page.");
	}
	*/
	file_read_page(PAGENUM_OF_HEADER, HD);

	// Create a new record for the value.
	record_t* new_record = (record_t*)malloc(sizeof(record_t));
	if (new_record == NULL) {
		perror("Record creation @db_insert.");
		exit(EXIT_FAILURE);
	}
	else {
		new_record->key = key;
		memcpy(new_record->value, value, sizeof(char) * VALUE_SIZE);
	}

	//printf("[%ld: %s] record\n", key, new_record->value);

	// Case: the tree does not exist yet. 
	// Start a new tree.
	if (HD->root_page_offset == 0) {
		start_new_tree(new_record);
		free(new_record);
		//free(HD);
		return 0;
	}

	// Case: the tree already exists. (Rest of function body.)
	pagenum_t temp_pagenum = find_leaf(key);

	page_t temp_page;
	file_read_page(temp_pagenum, &temp_page);

	// Case: leaf has room for key and value.
	if (temp_page.number_of_keys < LEAF_ORDER - 1) {
		insert_into_leaf(&temp_page, new_record);
	}

	// Case: leaf must be split.
	else {
		insert_into_leaf_after_splitting(&temp_page, new_record);
	}

	free(new_record);
	//free(HD);
	return 0;
}

/*
 * Delete
 */

void adjust_root(page_t* node) {
	// Case: Root is not empty.
	if (node->number_of_keys > 0) {
		file_write_page(node->pagenum, node);
		return;
	}

	// Case: Root is empty with no sibling.
	else if (node->is_leaf) {
		file_free_page(node->pagenum);
		HD->root_page_offset = 0;
		file_write_page(PAGENUM_OF_HEADER, HD);
		return;
	}

	// Case: Root is empty with siblings.
	else {
		file_free_page(node->pagenum);
		HD->root_page_offset = node->left_page_number;

		page_t temp_page;
		file_read_page(offset_to_pagenum(HD->root_page_offset), &temp_page);

		// Adjust parent page number to header page number
		temp_page.parent_page_number = PAGENUM_OF_HEADER;
		file_write_page(offset_to_pagenum(HD->root_page_offset), &temp_page);

		file_write_page(PAGENUM_OF_HEADER, HD);
		return;
	}
}

void redistribute_nodes(page_t* node, page_t* parent, page_t* neighbor, int neighbor_index, int k_prime_index) {
	internal_record_t* temp_k_prime = (internal_record_t*)malloc(sizeof(internal_record_t));
	temp_k_prime->key = parent->internal_record[k_prime_index].key;
	temp_k_prime->page_number = neighbor->left_page_number;
	
	// Case: node has neighbor to the left.
	if (neighbor_index == -2) {
		memcpy(&parent->internal_record[k_prime_index], &neighbor->internal_record[0], sizeof(internal_record_t));
		parent->internal_record[k_prime_index].page_number = neighbor->pagenum;

		memcpy(&node->internal_record[0], temp_k_prime, sizeof(internal_record_t));
		node->number_of_keys++;

		neighbor->left_page_number = neighbor->internal_record[0].page_number;
		for (int i = 0; i < neighbor->number_of_keys - 1; ++i)
			memcpy(&neighbor->internal_record[i], &neighbor->internal_record[i + 1], sizeof(internal_record_t));
		neighbor->number_of_keys--;

		free(temp_k_prime);
	}

	// Case: node is the leftmost child.
	else {
		memcpy(&parent->internal_record[k_prime_index], &neighbor->internal_record[neighbor->number_of_keys-1], sizeof(internal_record_t));
		parent->internal_record[k_prime_index].page_number = node->pagenum;

		pagenum_t right_page_number = neighbor->internal_record[neighbor->number_of_keys - 1].page_number;
		neighbor->number_of_keys--;

		memcpy(&node->internal_record[0], temp_k_prime, sizeof(internal_record_t));
		node->number_of_keys++;

		node->internal_record[0].page_number = node->left_page_number;
		node->left_page_number = right_page_number;

		free(temp_k_prime);
	}

	file_write_page(node->pagenum, node);
	file_write_page(parent->pagenum, parent);
	file_write_page(neighbor->pagenum, neighbor);

	return;
}

void coalesce_nodes(page_t* node, page_t* parent, page_t* neighbor, int neighbor_index, int k_prime_key) {
	
	// Swap neighbor with node.
	if (neighbor_index == -2) {
		page_t* temp_page;
		temp_page = node;
		node = neighbor;
		neighbor = temp_page;
	}

	// Case: nonleaf node.
	if (!node->is_leaf) {
		internal_record_t* k_prime_index = (internal_record_t*)malloc(sizeof(internal_record_t));
		k_prime_index->key = k_prime_key;
		k_prime_index->page_number = node->left_page_number;
		neighbor->number_of_keys++;

		for (int i = 0, j = neighbor->number_of_keys; i < node->number_of_keys; ++i, ++j) {
			memcpy(&neighbor->internal_record[j], &node->internal_record[i], sizeof(internal_record_t));
			neighbor->number_of_keys++;
		}

		page_t temp_page;
		for (int i = 0; i < neighbor->number_of_keys; ++i) {
			file_read_page(neighbor->internal_record[i].page_number, &temp_page);
			temp_page.parent_page_number = neighbor->pagenum;
			file_write_page(neighbor->internal_record[i].page_number, &temp_page);
		}

		free(k_prime_index);
	}

	// Case: leaf node
	else {
		for (int i = 0, j = neighbor->number_of_keys; i < node->number_of_keys; ++i, ++j) {
			memcpy(&neighbor->record[j], &node->record[i], sizeof(record_t));
			neighbor->number_of_keys++;
		}
		neighbor->right_sibling_page_number = node->right_sibling_page_number;
	}

	file_write_page(neighbor->pagenum, neighbor);
	file_free_page(node->pagenum);

	return delete_entry(parent, k_prime_key);
}

int get_neighbor_index(page_t* node, pagenum_t child_page_number) {
	if (node->left_page_number = child_page_number) return -2;
	for (int i = 0; i < node->number_of_keys; ++i)
		if (node->internal_record[i].page_number) return i - 1;

	perror("Pointing nonexisiting pointer @get_neighbor_index ");
	exit(EXIT_FAILURE);
}

void delete_entry(page_t* node, int64_t key) {
	int min_keys = 1;

	// Remove key and pointer from node.
	// Case : node for deletion is leaf.
	if (node->is_leaf) {
		int i = 0;
		while (node->record[i].key != key) i++;
		for (++i; i < node->number_of_keys; ++i)
			memcpy(&node->record[i - 1], &node->record[i], sizeof(record_t));
		node->number_of_keys--;
	}
	
	// Case : node for deletion is internal.
	else {
		int i = 0;
		while (node->internal_record->key != key) i++;
		for (++i; i < node->number_of_keys; ++i)
			memcpy(&node->internal_record[i - 1], &node->internal_record[i], sizeof(internal_record_t));
		node->number_of_keys--;
	}

	// Case: node for deletion is root.
	if (HD->root_page_offset == pagenum_to_offset(node->pagenum)) {
		return adjust_root(node);
	}

	// Case: Deletion from a node below the root.
	// (Rest of function body.)

	// Case:  node stays at or above minimum.
	if (node->number_of_keys >= min_keys) {
		file_write_page(node->pagenum, node);
		return;
	}

	// Case: node falls below minimum
	page_t temp_parent_page;
	file_read_page(node->parent_page_number, &temp_parent_page);

	int neighbor_index = get_neighbor_index(&temp_parent_page, node->pagenum);
	int k_prime_index;
	if (neighbor_index == -2 || neighbor_index == -1) {
		k_prime_index = 0;
	}
	else {
		k_prime_index = neighbor_index + 1;
	}

	int64_t k_prime_key = temp_parent_page.internal_record[k_prime_index].key;
	pagenum_t neighbor_page_number;

	switch (neighbor_index) {
	case -2:
		neighbor_page_number = temp_parent_page.internal_record[0].page_number;
		break;
	case -1:
		neighbor_page_number = temp_parent_page.left_page_number;
		break;
	default:
		neighbor_page_number = temp_parent_page.internal_record[neighbor_index].page_number;
		break;
	}

	page_t temp_neighbor_page;
	file_read_page(neighbor_page_number, &temp_neighbor_page);

	int capacity = node->is_leaf ? LEAF_ORDER : INTERNAL_ORDER - 1;

	// Case: Coalescence.
	if (temp_neighbor_page.number_of_keys + node->number_of_keys < capacity)
		return coalesce_nodes(node, &temp_parent_page, &temp_neighbor_page, neighbor_index, k_prime_key);

	// Case: Redistribution
	return redistribute_nodes(node, &temp_parent_page, &temp_neighbor_page, neighbor_index, k_prime_index);
}

int db_delete(int64_t key) {
	page_t * temp_page;
	temp_page->pagenum = find_leaf(key);

	char * temp_value;
	db_find(key, temp_value);

	if (temp_value != NULL && temp_page != NULL) {
		delete_entry(temp_page, key);
		return 0;
	}

	return -1;
}

// ********** ????????? **********
void enqueue(pagenum_t offset, queue* q) { q->arr[++q->r] = offset; }
pagenum_t dequeue(queue* q) { return q->arr[q->f++]; }
int get_rank(uint64_t offset) {
	// Allocate header page and get metadata.
	/*
	page_t* HD;
	HD = (page_t*)malloc(sizeof(page_t));
	if (HD == NULL) {
		perror("Header creation @file_alloc_page.");
	}
	*/
	file_read_page(PAGENUM_OF_HEADER, HD);

	page_t* pg = (page_t*)malloc(sizeof(page_t));
	uint64_t next_offset = offset;
	int rank = 0;
	while (next_offset != HD->root_page_offset) {
		file_read_page(offset_to_pagenum(next_offset), pg);
		next_offset = pg->parent_page_number;
		rank++;
	}

	free(pg);
	//free(HD);
	return rank;
}
void print_tree() {
	// Allocate header page and get metadata.
	/*
	page_t* HD;
	HD = (page_t*)malloc(sizeof(page_t));
	if (HD == NULL) {
		perror("Header creation @file_alloc_page.");
	}
	*/
	file_read_page(PAGENUM_OF_HEADER, HD);

	if (HD->root_page_offset == 0) {
		printf("Empty tree!\n");
		//free(HD);
		return;
	}

	queue* q = (queue*)malloc(sizeof(queue));
	q->arr = (uint64_t*)malloc(sizeof(uint64_t) * 100);
	q->f = 0;
	q->r = -1;

	enqueue(HD->root_page_offset, q);

	page_t page, parent_node;
	page_t* node = &page;
	page_t* parent = &parent_node;
	int new_rank = 0;
	while (q->f <= q->r) {
		uint64_t offset = dequeue(q);
		file_read_page(offset_to_pagenum(offset), node);

		if (node->parent_page_number != 0) {
			file_read_page(node->parent_page_number, parent);
			if (get_rank(offset) != new_rank) {
				new_rank = get_rank(offset);
				printf("\n");
			}
		}

		if (!node->is_leaf) {
			for (int i = 0; i < node->number_of_keys; ++i)
				printf("%ld  ", node->internal_record[i].key);
			enqueue(node->left_page_number, q);

			for (int i = 0; i < node->number_of_keys; ++i)
				enqueue(node->internal_record[i].page_number, q);
		}
		else {
			for (int i = 0; i < node->number_of_keys; ++i)
				printf("%ld : %s  ", node->record[i].key, node->record[i].value);
		}
		printf("| ");
	}
	printf("\n");

	free(q->arr);
	free(q);
	//free(HD);

	return;
}
