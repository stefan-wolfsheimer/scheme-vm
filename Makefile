MODULES= util core programs test_core test_util 
MODULES_COV= util core
SRC=
SRC_TEST=
SRC_MAIN=
CC=gcc

OBJDIR=obj
BINDIR=bin
DEPFLAGS_TEST = -MT $@ -MMD -MP -MF ${OBJDIR}/$*.debug.td
DEPFLAGS_COV  = -MT $@ -MMD -MP -MF ${OBJDIR}/$*.cov.td
DEPFLAGS      = -MT $@ -MMD -MP -MF ${OBJDIR}/$*.td
CFLAGS_TEST   = -g -Wall -std=c99 -I. -DDEBUG -lm
CFLAGS_COV    = -g -Wall -std=c99 -I. -DDEBUG --coverage -lm
CFLAGS = -O2 -Wall -I. -lm

include $(patsubst %, %/module.mk, $(MODULES) )

OBJ =      $(patsubst %.c, $(OBJDIR)/%.o, $(SRC) ) 
OBJ_TEST = $(patsubst %.c, $(OBJDIR)/%.debug.o, $(SRC_TEST) $(SRC) ) 
OBJ_COV =  $(patsubst %.c, $(OBJDIR)/%.cov.o,   $(SRC_TEST) $(SRC) ) 
PWD     = ( pwd )
all: ${BINDIR}/lisp_test 

# ${BINDIR}/optimize_hash_table

${BINDIR}/lisp_test: ${OBJ_TEST} $(OBJDIR)/programs/lisp_test.debug.o
	mkdir -p ${BINDIR}
	${CC} ${CFLAGS_TEST} ${OBJ_TEST} $(OBJDIR)/programs/lisp_test.debug.o -o $@

${OBJDIR}/lisp_test_cov: ${OBJ_COV} $(OBJDIR)/programs/lisp_test.cov.o
	${CC} ${CFLAGS_COV} ${OBJ_COV} $(OBJDIR)/programs/lisp_test.cov.o -o $@

${BINDIR}/optimize_hash_table: ${OBJ_TEST} $(OBJDIR)/programs/optimize_hash_table.o
	${CC} ${CFLAGS_TEST} ${OBJ_TEST} $(OBJDIR)/programs/optimize_hash_table.o -o $@

${OBJDIR}/%.o : %.c
	mkdir -p $(@D)
	$(CC) ${CFLAGS} ${DEPFLAGS} -c $< -o $@
	mv ${OBJDIR}/$*.td ${OBJDIR}/$*.d 

${OBJDIR}/%.debug.o : %.c
	mkdir -p $(@D)
	$(CC) ${CFLAGS_TEST} ${DEPFLAGS_TEST} -c $< -o $@
	mv ${OBJDIR}/$*.debug.td ${OBJDIR}/$*.debug.d 

${OBJDIR}/%.cov.o : %.c
	mkdir -p $(@D)
	$(CC) ${CFLAGS_COV} ${DEPFLAGS_COV} -c $< -o $@
	mv ${OBJDIR}/$*.cov.td ${OBJDIR}/$*.cov.d 


%.debug.d: ;
.PRECIOUS: %.debug.d

%.cov.d: ;
.PRECIOUS: %.div.d

%.d: ;
.PRECIOUS: %.d

-include $(patsubst %.c, ${OBJDIR}/%.debug.d, ${SRC_TEST})
-include $(patsubst %.c, ${OBJDIR}/%.debug.d, ${SRC})
-include $(patsubst %.c, ${OBJDIR}/%.debug.d, ${SRC_MAIN_TEST})
-include $(patsubst %.c, ${OBJDIR}/%.cov.d,   ${SRC_TEST})
-include $(patsubst %.c, ${OBJDIR}/%.cov.d,   ${SRC})
-include $(patsubst %.c, ${OBJDIR}/%.civ.d,   ${SRC_MAIN_TEST})
-include $(patsubst %.c, ${OBJDIR}/%.d,       ${SRC_MAIN})
-include $(patsubst %.c, ${OBJDIR}/%.d,       ${SRC})

clean:
	rm -f obj/*.o
	rm -f obj/*.d
	rm -f obj/*/*.o
	rm -f obj/*/*.d
	rm -f bin/test_lisp
	rm -f obj/test_lisp_cov
	rm -f obj/*/*.gcda
	rm -f obj/*/*.gcno
	rm -rf doc/html
	rm -rf doc/latex
	rm -rf doc/dov

.PHONY: clean

${OBJDIR}/coverage.info: ${OBJDIR}/lisp_test_cov
	${OBJDIR}/lisp_test_cov eval
	lcov --capture --no-external --directory .  --output-file ${OBJDIR}/coverage_unfiltered.info
	lcov --remove ${OBJDIR}/coverage_unfiltered.info $(shell pwd )/'test_*/*' $(shell pwd )/'programs/*' -o ${OBJDIR}/coverage.info


cov: ${OBJDIR}/coverage.info
	genhtml ${OBJDIR}/coverage.info --output-directory doc/coverage/

doc:
	doxygen

.PHONY: doc




