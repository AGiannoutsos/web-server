CC = gcc
OBJDIR = build
SRCDIR = src
BINDIR = bin
INCDIR = /
FLAGS = -g 


SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC))

OBJ_SERV = build/whoServer.o build/circular_buffer.o build/whoServer_thread_functions.o build/util.o build/message_handlers.o build/master_functions.o build/socket_functions.o  build/queries_master.o  
 
OBJ_CLNT = build/whoClient.o build/circular_buffer.o build/whoClient_thread_functions.o build/util.o build/message_handlers.o build/master_functions.o build/socket_functions.o build/whoClient_thread_functions.o build/queries_master.o build/queries_list.o

OBJ_MSTR = build/master.o build/message_handlers.o build/util.o build/date.o build/hashtable.o build/patient_list.o build/ht_avlt.o build/avl_tree.o build/queries_master.o build/master_functions.o

OBJ_WORK = build/worker.o build/message_handlers.o build/util.o build/date.o build/hashtable.o build/patient_list.o build/ht_avlt.o build/avl_tree.o build/queries_worker.o build/worker_functions.o build/socket_functions.o

.PHONY: all clean mainexe

all: whoServer_ whoClient_ master_ worker_

whoServer_: whoServer

whoClient_: whoClient

master_ : master

worker_ : worker


whoServer: $(OBJ_SERV)
	echo compiling $?
	$(CC) $(FLAGS)  -o $@ $^ -lm -lpthread

whoClient: $(OBJ_CLNT)
	echo compiling $?
	$(CC) $(FLAGS)  -o $@ $^ -lm -lpthread

master: $(OBJ_MSTR)
	echo compiling $?
	$(CC) $(FLAGS) -o $@ $^ -lm -lpthread

worker: $(OBJ_WORK)
	echo compiling $?
	$(CC) $(FLAGS) -o $@ $^ -lm -lpthread



$(OBJDIR)/whoServer.o: $(SRCDIR)/whoServer.c
	echo compiling $? $@
	$(CC) $(FLAGS)  -c  $^ -o $@ -lpthread

$(OBJDIR)/whoClient.o: $(SRCDIR)/whoClient.c
	echo compiling $? $@
	$(CC) $(FLAGS) -c  $^ -o $@ -lpthread

$(OBJDIR)/master.o: $(SRCDIR)/master.c
	echo compiling $? $@
	$(CC) $(FLAGS) -I$(INCDIR) -c  $^ -o $@ -lpthread

$(OBJDIR)/worker.o: $(SRCDIR)/worker.c
	echo compiling $? $@
	$(CC) $(FLAGS) -I$(INCDIR) -c  $^ -o $@ -lpthread

$(OBJDIR)/socket_functions.o: $(SRCDIR)/socket_functions.c
	echo compiling $? $@
	$(CC) $(FLAGS) -I$(INCDIR) -c  $^ -o $@ -lpthread

$(OBJDIR)/circular_buffer.o: $(SRCDIR)/circular_buffer.c
	echo compiling $? $@
	$(CC) $(FLAGS) -I$(INCDIR) -c  $^ -o $@ -lpthread

$(OBJDIR)/whoServer_thread_functions.o: $(SRCDIR)/whoServer_thread_functions.c
	echo compiling $? $@
	$(CC) $(FLAGS) -I$(INCDIR) -c  $^ -o $@ -lpthread


$(OBJDIR)/whoClient_thread_functions.o: $(SRCDIR)/whoClient_thread_functions.c
	echo compiling $? $@
	$(CC) $(FLAGS) -I$(INCDIR) -c  $^ -o $@ -lpthread

$(OBJDIR)/queries_list.o: $(SRCDIR)/queries_list.c
	echo compiling $? $@
	$(CC) $(FLAGS) -I$(INCDIR) -c  $^ -o $@ -lpthread





$(OBJDIR)/message_handlers.o: $(SRCDIR)/message_handlers.c
	echo compiling $? $@
	$(CC) $(FLAGS) -I$(INCDIR) -c  $^ -o $@ -lpthread

$(OBJDIR)/util.o: $(SRCDIR)/util.c
	echo compiling $? $@
	$(CC) $(FLAGS) -I$(INCDIR) -c  $^ -o $@ -lpthread

$(OBJDIR)/hashtable.o: $(SRCDIR)/hashtable.c
	echo compiling $? $@
	$(CC) $(FLAGS) -I$(INCDIR) -c  $^ -o $@ -lpthread

$(OBJDIR)/date.o: $(SRCDIR)/date.c
	echo compiling $? $@
	$(CC) $(FLAGS) -I$(INCDIR) -c  $^ -o $@ -lpthread

$(OBJDIR)/ht_avlt.o: $(SRCDIR)/ht_avlt.c
	echo compiling $? $@
	$(CC) $(FLAGS) -I$(INCDIR) -c  $^ -o $@ -lpthread

$(OBJDIR)/avl_tree.o: $(SRCDIR)/avl_tree.c
	echo compiling $? $@
	$(CC) $(FLAGS) -I$(INCDIR) -c  $^ -o $@ -lpthread

$(OBJDIR)/patient_list.o: $(SRCDIR)/patient_list.c
	echo compiling $? $@
	$(CC) $(FLAGS) -I$(INCDIR) -c  $^ -o $@ -lpthread

$(OBJDIR)/queries_master.o: $(SRCDIR)/queries_master.c
	echo compiling $? $@
	$(CC) $(FLAGS) -I$(INCDIR) -c  $^ -o $@ -lpthread

$(OBJDIR)/queries_worker.o: $(SRCDIR)/queries_worker.c
	echo compiling $? $@
	$(CC) $(FLAGS) -I$(INCDIR) -c  $^ -o $@ -lpthread

$(OBJDIR)/master_functions.o: $(SRCDIR)/master_functions.c
	echo compiling $? $@
	$(CC) $(FLAGS) -I$(INCDIR) -c  $^ -o $@ -lpthread


$(OBJDIR)/worker_functions.o: $(SRCDIR)/worker_functions.c
	echo compiling $? $@
	$(CC) $(FLAGS) -I$(INCDIR) -c  $^ -o $@ -lpthread

clean:
	rm $(OBJDIR)/*.o
	rm worker
	rm master
	rm whoServer
	rm whoClient


