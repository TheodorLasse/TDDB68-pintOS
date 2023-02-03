#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct student {
	char *name;
	struct list_elem elem;
};


void insert (struct list *student_list) {
	char *newName = malloc(sizeof(char) * 128);
	printf("Which student will you insert? \n");
	scanf("%s", newName);
	printf("%s inserted \n", newName);

	struct student* newStudent = (struct student*) malloc(sizeof(struct student));
	newStudent->name = newName;
	list_push_back(student_list, &newStudent->elem);
}

void delete (struct list *student_list) {
	char newName[128];
	printf("Which student will you remove? \n");
	scanf("%s", newName);

	struct list_elem *e;

	for (e = list_begin (student_list); e != list_end (student_list); e = list_next (e))
	{
        struct student *iterStudent = list_entry (e, struct student, elem);
		printf("\n\nTrying to remove %s\n", iterStudent->name);
		if (!strcmp(iterStudent->name, newName))
		{
			printf("removed");
			list_remove(e);
			free(iterStudent->name);
			free(iterStudent);
			return;
		}
		
    }
}

void list (struct list *student_list) {
	printf("\nStudents:");
	struct list_elem *e;
	for (e = list_begin (student_list); e != list_end (student_list); e = list_next (e))
	{
        struct student *iterStudent = list_entry (e, struct student, elem);
		char *name = iterStudent->name;
		printf("%s, ", name);
    }
}

void quit (struct list *student_list) {
	while (!list_empty (student_list))
    {
       struct list_elem *e = list_pop_front (student_list);
       struct student *iterStudent = list_entry (e, struct student, elem);
	   free(iterStudent->name);
	   free(iterStudent);
    }
	printf("List has been cleared. \n");
	exit(1);
}

int main() {
	struct list student_list;
	list_init (&student_list);
	int opt;

	do {
		printf("\nMenu:\n");
		printf("1 - Insert student\n");
		printf("2 - Delete student\n");
		printf("3 - List students\n");
		printf("4 - Exit\n\n");
		scanf("%d", &opt);
		switch (opt) {
			case 1:
				{
					insert(&student_list);
					break;
				}
			case 2:
				{
					delete(&student_list);
					break;
				}
			case 3:
				{
					list(&student_list);
					break;
				}

			case 4:
				{
					quit(&student_list);
					break;
				}
			default:
				{
					printf("Quit? (1/0):\n");
					scanf("%d", &opt);
					if (opt)
						quit(&student_list);
					break;
				}
		}
	} while(1);
		
	return 0;
}
