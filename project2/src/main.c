#include "bpt.h"

int main(int argc, char ** argv) {
	int key;
	char* inp_val;
	char* ret_val;
	char instruction[10];
	
	/*
	char* temp_path_name;
	temp_path_name = (char*)malloc(sizeof(char) * 100);
	//temp
	memcpy(&temp_path_name, "temp", sizeof(temp_path_name));
	*/

	// Open DB file priot to other fuctions.
	if (open_table("temp_DB.txt") != 0) printf("File is successfully opened!\n");

	usage(0);

	while (scanf("%s", instruction) != EOF) {
		switch (instruction[0]) {
		/*
		case 'o': // OPEN
			inp_val = (char*)malloc(sizeof(char) * VALUE_SIZE);
			if (inp_val == NULL) {
				perror("String creation @main");
				exit(EXIT_FAILURE);
			}
			scanf("%s", inp_val);
			int table_id = open_table(inp_val);
			if (table_id != 0) printf("File is successfully opened!\n");
			break;
		*/
		case 'i': // INSERT
			inp_val = (char*)malloc(sizeof(char) * VALUE_SIZE);
			if (inp_val == NULL) {
				perror("String creation @main");
				exit(EXIT_FAILURE);
			}
			scanf("%d %s", &key, inp_val);
			//printf("[%ld: %s] in main\n", key, inp_val);
			db_insert(key, inp_val);
			//print_tree();
			free(inp_val);
			break;
		case 'f': // FIND
			// Memory allocation in caller function.
			ret_val = (char*)malloc(sizeof(char) * VALUE_SIZE);
			if (ret_val == NULL) {
				perror("String creation @main");
				exit(EXIT_FAILURE);
			}
			scanf("%d", &key);
			int findng_result = db_find(key, ret_val);
			if (findng_result == 0) printf("There is matching key matched with the value.\n%d : %s\n", key, ret_val);
			else if (findng_result == 1) printf("Finding failed. (There is no such key in the tree).\n");
			else if (findng_result == 2) printf("Finding failed. (Tree does not exisit).\n");
			free(ret_val);
			break;
		case 'd': // DELETE
			scanf("%d", &key);
			int deleting_result = db_delete(key);
			if (deleting_result == 0) printf("Deleting key %d complete.\n", key);
			else if (deleting_result == -1) printf("Deletion failed.\n");
			break;
		case 'q': // QUIT
			while (getchar() != (int)'\n');
			return EXIT_SUCCESS;
		default:
			usage(1);
			break;
		}

		while (getchar() != (int)'\n');
		printf("> ");
	}
	printf("\n");
	return EXIT_SUCCESS;
}
