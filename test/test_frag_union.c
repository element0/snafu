#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum fragrole { FRAGISCONTENT, FRAGISCONTAINER };

struct frag_content_role {
	int role;
	void *str;
};


struct frag_container_role {
	int role;
	union frag_union *contains;
	union frag_union *prev;
	union frag_union *next;
};

struct frag_controller {
	int role;
};

union frag_union {
	struct frag_controller set;
	struct frag_container_role flist;
	struct frag_content_role fbuf;
};

struct fragment {
	int role;		/* content | container */
	/* role: content */
	/* very important, content role never links lists*/
	void *source;		/* grid_addr */
	/* role: container */
	void *first_content;
	void *last_content;
	struct fragment *prev;	/* linked list */
	struct fragment *next;
};

struct hiena_dcel {
	void *dcelno;
	void *fragment;
};

int main ( int argc, char *argv[] ) {

	union frag_union f;

	/* this uses a special union member just to set the role var.
	   the 'role' var belongs to all members. */
	f.set.role = FRAGISCONTAINER;

	if(f.set.role == FRAGISCONTAINER) {
		f.flist.contains = malloc(sizeof(union frag_union));

		f.flist.contains->set.role = FRAGISCONTENT;
		f.flist.contains->fbuf.str
			= (void *)"crackerjacks";

		printf("f.flist.contains->fbuf.str: %s\n",
				f.flist.contains->fbuf.str);

		f.flist.contains->set.role = FRAGISCONTAINER;
		f.flist.contains->flist.next = malloc(sizeof(union frag_union));
		f.flist.contains->flist.contains = malloc(sizeof(union frag_union));
		f.flist.contains->flist.contains->set.role = FRAGISCONTENT;
		f.flist.contains->flist.contains->fbuf.str = (void *)"hard days night";

		f.flist.contains->flist.next->set.role = FRAGISCONTAINER;
		f.flist.contains->flist.next->flist.contains = malloc(sizeof(union frag_union));

		f.flist.contains->flist.next->flist.contains->set.role = FRAGISCONTENT;
		f.flist.contains->flist.next->flist.contains->fbuf.str = (void *)"it's a hard knock life";

		printf("f.flist.contains->flist.contains->fbuf.str: %s\n",
			f.flist.contains->flist.contains->fbuf.str);
		printf("f.flist.contains->flist.next->flist.contains->fbuf.str: %s\n", f.flist.contains->flist.next->flist.contains->fbuf.str);


		free(f.flist.contains);
	}

	return 0;
}
