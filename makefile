unamestr = $(shell uname)
ifeq ($(unamestr),Linux)
	buildout = out
else ifeq ($(unamestr),Darwin)
	buildout = out-macos
endif


default:
	# do nothing

clean:
	-rm src/lib/*.o
	-rm src/*.o
	-rm $(buildout)/*.o


#### OBJECT CODE ONLY ####
aframe: src/lib/aframe.c src/lib/aframe.h
	gcc -c -fPIC src/lib/aframe.c -o $(buildout)/aframe.o


#### SHARED LIBS ####
addressdb: src/lib/addressdb.c src/lib/addressdb.h
	gcc -shared -fPIC src/lib/addressdb.c -o $(buildout)/libaddressdb.a

liblookupmod: src/lib/lookup_module.h src/lib/server_module.c
	gcc -shared -fPIC src/lib/lookup_module.c -o $(buildout)/liblookupmod.a

libservermod: src/lib/server_module.h src/lib/server_module.c
	gcc -shared -fPIC src/lib/server_module.c -o $(buildout)/libservermod.a

gridsock: src/lib/gridsock.c src/lib/gridsock.h
	gcc -g -shared -fPIC src/lib/gridsock.c -o $(buildout)/libgridsock.a

gridhost: src/lib/gridhost.c gridsock addressdb liblookupmod
	gcc -g -shared -fPIC\
		src/lib/gridhost.c\
		-L$(buildout)\
		-lgridsock\
		-laddressdb\
		-o $(buildout)/libhost.a

gridclient: src/lib/gridclient.c src/lib/gridmsg.c gridsock
	gcc -g -shared -fPIC\
		src/lib/gridclient.c src/lib/gridmsg.c\
		src/lib/buflink.c\
		src/lib/aframe.c\
		-L$(buildout)\
		-lgridsock\
		-o $(buildout)/libgridclient.a

snf_suite: src/lib/snf_suite_ino.c gridclient
	gcc -g -shared -fPIC -Wall\
		src/lib/snf_suite_ino.c\
		-L$(buildout)\
		-lgridclient\
		-o $(buildout)/libsnf_suite.a


#### MODULES ####
servermod_file: src/module/server/file/file.c addressdb libservermod
	gcc -shared -fPIC\
		src/module/server/file/file.c\
		-L$(buildout)\
		-laddressdb\
		-lservermod\
		-o $(buildout)/libservermod_file.a

lookupmod_fudge: src/module/lookup/fudge/fudge.c liblookupmod
	gcc -shared -fPIC\
		src/module/lookup/fudge/fudge.c\
		-L$(buildout)\
		-llookupmod\
		-o $(buildout)/liblookupmod_fudge.a

#### BIN PRODUCTS ####


snafu: src/snafu.c gridclient aframe
	gcc -g -Wall\
		src/snafu.c\
		$(buildout)/aframe.o\
		-L$(buildout)\
		-lgridsock\
		-lgridclient\
		`pkg-config fuse --cflags --libs`\
		-o $(buildout)/snafu

griddb: src/griddb.c
	gcc -g -Wall\
		src/griddb.c src/lib/griddb.c src/lib/buflink.c\
		-lkyotocabinet\
		-o $(buildout)/griddb

griduser: src/griduser.c gridsock gridclient
	gcc -g -Wall\
		src/griduser.c src/lib/gridmsg_engine.c\
		-L$(buildout)\
		-lgridsock\
		-lgridclient\
		-o $(buildout)/griduser

hnfc: src/hnfc.c
	gcc -g -Wall src/hnfc.c -o $(buildout)/hnfc

### TESTS ###
test_make_all: aframe gridsock gridhost gridclient snafu gridhostd

test_gridclient: test/test_gridclient.c gridclient
	gcc -g -Wall\
		test/test_gridclient.c\
		-L$(buildout)\
		-lgridclient\
		-lgridsock\
		-o $(buildout)/test_gridclient `pkg-config fuse --cflags --libs`

test_dcel: test/test_dcel.c src/lib/dcel.h
	gcc test/test_frag_union.c -o $(buildout)/test_frag_union

test_gridmsg: test/test_gridmsg.c gridclient
	gcc -g -Wall\
		test/test_gridmsg.c\
		-L$(buildout)\
		-lgridclient\
		-lgridsock\
		`pkg-config fuse --cflags --libs`\
		-o $(buildout)/test_gridmsg

test_griddb: test/test_griddb.c griddb
	gcc -g -Wall\
		test/test_griddb.c\
		src/lib/buflink.c\
		src/lib/griddb.c\
		-o $(buildout)/test_griddb

test_gridmsg_engine: test/test_gridmsg_engine.c griddb gridclient
	gcc -g -Wall\
		test/test_gridmsg_engine.c\
		src/lib/gridmsg_engine.c\
		src/lib/buflink.c\
		src/lib/griddb.c\
		-lkyotocabinet\
		-L$(buildout)\
		-lgridclient\
		-lgridsock\
		-o $(buildout)/test_gridmsg_engine

### DOCUMENTATION ###
diagram_codelayo: design/codelayo.dot
	dot -Tsvg design/codelayo.dot > design/codelayo.svg

diagram_devorder: design/devorder.dot
	fdp -Tsvg design/devorder.dot > design/devorder.svg

diagram_addressdbflow:
	dot -Tsvg design/addressdb_flow.dot > design/addressdb_flow.svg

diagrams: diagram_codelayo diagram_devorder diagram_addressdbflow


documentation:  Documentation/Doxyfile diagrams
	doxygen Documentation/Doxyfile

### BONES ###
#initdoxy:
	# doxygen -g Documentation/Doxyfile
