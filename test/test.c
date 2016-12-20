

#include <stdio.h>
#include <stdlib.h>


/* included from /Users/paulmartin/src/modularC/test/list.module.c */
typedef struct {
  unsigned length;
  size_t allocated_size;
  void ** items; 
} list_List;
int list_push(list_List *l, void* item);
void * list_get(list_List * l, unsigned index);



void printList(list_List l){
        printf("LIST: {\
                \n\tlength         : %d,\
                \n\tallocated_size : %lu,\
                \n\titems          : [\n",l.length, l.allocated_size);
        int i;
        for (i = 0; i < l.length; i++){
            printf("\n\t\t%d,", *((int*)list_get(&l,i)));
        }
        printf("\n\t],\n}\n");
}
int main(){
    printf("Adding 10 items\n");
    int i;
    list_List l = {0};

    for (i = 10; i--;){
        int * item = malloc(sizeof(int));
        *item = i;
        list_push(&l,item);
        printList(l);
    }
}

