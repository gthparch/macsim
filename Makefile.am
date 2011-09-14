AUTOMAKE_OPTIONS = foreign
SUBDIRS = scripts .obj tools 

all:
	-test -z .obj/macsim_opt || (test -z ./bin/macsim || rm -f ./bin/macsim) && cd bin && ln -s ../.obj/macsim_opt macsim

opt:
	@echo Build optimized executable
	make opt -C .obj
	-test -z .obj/macsim_opt || (test -z ./bin/macsim || rm -f ./bin/macsim) && cd bin && ln -s ../.obj/macsim_opt macsim

dbg:
	@echo Build debugging executable
	make -C scripts
	make dbg -C .obj
	-test -z .obj/macsim_dbg || (test -z ./bin/macsim || rm -f ./bin/macsim) && cd bin && ln -s ../.obj/macsim_dbg macsim

gpf:
	@echo Build gprof-capable executable
	make -C scripts
	make gpf -C .obj
	-test -z .obj/macsim_gpf || (test -z ./bin/macsim || rm -f ./bin/macsim) && cd bin && ln -s ../.obj/macsim_gpf macsim

cleanopt:
	make cleanopt -C .obj
	-test -z bin/macsim || rm -f bin/macsim

cleandbg:
	make cleandbg -C .obj
	-test -z bin/macsim || rm -f bin/macsim

cleangpf:
	make cleangpf -C .obj
	-test -z bin/macsim || rm -f bin/macsim

cleandep:
	make cleandep -C .obj
